LCD-Controller
==============

Stellaris+Msp430 Lcd Controller


This software is designed to run a 320x240 monochrome LCD with minimal CPU 
overhead.

A seprate controller board supplies the VEE for the display, controls the 
Backlight and has a MSP430G2542 running as a SPI Master. The MSP also controls
the clocks for moving the data to the display.

The Stellaris side sets up a 1BPP offscreen display for Grlib and also sets up a
UDMA job that transfers the display buffer to the SPI port for the MSP to collect.

The SPI connection is set up to transfer 16 bits at a time, allowing for 4 pixel
updates for each transfer. With the SPI clock running at 8Mhz, the display can 
update at ~51 Hz.