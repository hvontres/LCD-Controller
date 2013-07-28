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

Video Demo here :http://www.youtube.com/watch?v=h1OtnWywmYI


This project has been built on linux using gcc for the Stellaris and naken_asm
for the MSP430.

1) get gcc toolchain:
https://github.com/esden/summon-arm-toolchain
or
https://launchpad.net/gcc-arm-embedded
2) get naken_asm:
http://downloads.mikekohn.net/naken_asm/naken_asm-2012-09-02.tar.gz
make sure that naken_asm is on your path, or make won't find it.

3) install stellarisware

4) edit the TOOL and SW_DIR varables in the makefile to point at your arm toolchain
and stellarisware.

5) run make all.

6) the easiest way to program the MSP430G2542 is by putting it in a launchpad and
running mspdebug. Alternatively, there is a debug headder on the controller board.
This should work fine, as long as the EK-LM4F120XL is NOT installed.
