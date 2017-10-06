/*
 * main.cpp
 *
 *  Created on: 1 sep. 2017
 *      Author: Paul
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE     512
char readBuf[MAX_BUFFER_SIZE];

#define NUM_MESSAGES        100
#define DEVICE_NAME     "/dev/rpmsg_pru30"

int main(int argc, char* argv[])
{
    int     fd;
    int     lastErrno;
    int i;
    int result = 0;

    fd = open(DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        lastErrno = errno;
        printf("In %s (%s line %i) : %s\n", __FUNCTION__, __FILE__, __LINE__, strerror(lastErrno));
        return -1;
    }

    /* The RPMsg channel exists and the character device is opened */
    printf("Opened %s, sending %d messages\n\n", DEVICE_NAME, NUM_MESSAGES);

    for (i = 0; i < NUM_MESSAGES; i++) {
        /* Send 'hello world!' to the PRU through the RPMsg channel */
        result = write(fd, "hello world!", 13);
        if (result > 0)
            printf("Message %d: Sent to PRU\n", i);

        /* Poll until we receive a message from the PRU and then print it */
        result = read(fd, readBuf, MAX_BUFFER_SIZE);
        if (result > 0)
            printf("Message %d received from PRU:%s\n\n", i, readBuf);
    }

    /* Received all the messages the example is complete */
    printf("Received %d messages, closing %s\n", NUM_MESSAGES, DEVICE_NAME);


    close(fd);

    return 0;
}

