/**
 * main.c
 */

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>

#ifndef PRU0
#ifndef PRU1
#define PRU0
#endif
#endif

#include "resource_table.h"

#define RAILCOM_BITPOS 0
#define DCC_BITPOS 1

volatile register uint32_t __R30;
volatile register uint32_t __R31;

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK   4

uint8_t payload[RPMSG_BUF_SIZE];

inline
void SendBit(const uint8_t bit)
{
    if(0 == bit)
    {
        __R30 |= (1 << DCC_BITPOS);
        __delay_cycles(20000);
        __R30 &= !(1 << DCC_BITPOS);
        __delay_cycles(20000);
    }
    else
    {
        __R30 |= (1 << DCC_BITPOS);
        __delay_cycles(11600);
        __R30 &= !(1 << DCC_BITPOS);
        __delay_cycles(11600);
    }
}

inline
void SendByte(const uint8_t byte)
{
    SendBit(0); //  Startbit

    uint8_t mask = 0x80;
    while(0 != mask)
    {
        SendBit(byte & mask);
        mask /= 2;
    }
}

inline
void RailComCutout(void)
{
    __R30 |= (1 << DCC_BITPOS);
    __delay_cycles(5800);   //  Tcs
    __R30 |= (1 << RAILCOM_BITPOS);
    __delay_cycles(47100);
    __R30 &= !(1 << DCC_BITPOS);
    __delay_cycles(47100);
    __R30 &= !(1 << RAILCOM_BITPOS);
    __delay_cycles(10);
}

void SendCommand(const uint8_t* pCmd)
{
    uint8_t i;

    for(i = 0; i < 16; i++)
    {
        SendBit(1);
    }

    for(i = 1; i < pCmd[0]; i++)
    {
        SendByte(pCmd[i]);
    }

    SendBit(1);

//    RailComCutout();
}


int main(void)
{
    static const uint8_t ResetCmd[] = { 0x04, 0x00, 0x00, 0x00 };
    static const uint8_t IdleCmd[] = { 0x04, 0xFF, 0x00, 0xFF };

    __R30 &= 0xFFFFFF00;

    struct pru_rpmsg_transport transport;
    uint16_t src, dst, len;
    volatile uint8_t *status;

    /* Allow OCP master port access by the PRU so the PRU can read external memories */
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

    /* Make sure the Linux drivers are ready for RPMsg communication */
    status = &resourceTable.rpmsg_vdev.status;
    while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

    /* Initialize the RPMsg transport structure */
    pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

    /* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
    while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);

    SendCommand(ResetCmd);

    while(1)
    {
        /* Check bit 30 of register R31 to see if the ARM has kicked us */
        if (__R31 & HOST_INT)
        {
            __R30 |= 0x08;
            /* Clear the event status */
            CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
            /* Receive all available messages, multiple messages can be sent per kick */
            while (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS)
            {
                /* Echo the message back to the same address from which we just received */
                pru_rpmsg_send(&transport, dst, src, payload, len);
                SendCommand(ResetCmd);
            }
            __R30 &= ~(0x08);
        }
        else
        {
            SendCommand(IdleCmd);
        }
    }
}
