# DCC_PRU
The repository for PRU sourcecode for the DCC project.

The DCC project is an attempt to implement a DCC (Digital Command Control) Command Station based on the BeagleBone Black platform.
The project consists of four parts:
- DCC_BBB : the Linux code for the BeagleBone.
- DCC_PRU (this repo) : the PRU code for generating the actuel DCC signal (including RailCom cutout).
- DCC_AVR : the AVR code for additional interfaces (for example Expressnet).
- DCC_Hardware : the schematics and PCB-layout files for the hardware (KiCad)
