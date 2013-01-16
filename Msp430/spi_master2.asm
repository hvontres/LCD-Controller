/*
//*****************************************************************************
// SPI Master and LCD output controller Code for MSP430g24x2
//
// Built with the Naken assembler (http://www.mikekohn.net/micro/naken_asm.php)
// This Code needs a part with the USI subsytem in order to support 16bit
// SPI transfers.
// The data is recieved LSB first to make sure the pixels match the order on the
// Data lines.
//  MCLK = SMCLK = 16 Mhz DCO
//
//           Master
//           MSP430G2xx2
//         -----------------
//     /|\|             P2.6|-> LCD FLM
//      | |                 |
//      --|RST          P2.7|-> LCD M
// /|\    |                 |
// -+>----|RST/NMI          |
// ------>|P1.4             |
//        |             P1.0|-> LED
// -------|P1.6/SDO         |
// ------>|P1.7/SDI         |
// -------|P1.5/SCLK        |
//        |             P2.5|-> LCD LOAD
//        |             P2.4|-> LCD CL
//        |        P2.0-P2.3|-> LCD D0-D3
//         -----------------
//
//*****************************************************************************

//*****************************************************************************
// Copyright (c) 2012, Henry von Tresckow
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// Redistributions of source code must retain the above copyright notice, this list
// of conditions and the following disclaimer.

// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY HENRY VON TRESCKOW "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL {{THE COPYRIGHT HOLDER OR CONTRIBUTORS}} BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
//*****************************************************************************
*/

.include "msp430g24x2.inc"

; Use Rgister Storage for main vars to save CPU cycles
LOWNIB equ r6 	;Register for nibbles 1 and 3
HIGHNIB equ r7 	;Register for nibbles 2 and 4
OUTVAL equ r8 	;Regiser for Output Value for port 2
MASK1 equ r9 	;Bitmask Register 1
MASK2 equ r10	;Bitmask Register 2
MASK3 equ r11	;Bitmask Register 3
PIXCNT equ r12	;Pixel Counter Register
ROWCNT equ r13	;Row Counter register

PIXELS equ #20 ;20*16 pixels/loop =320 pixels
ROWS   equ #240 ; 240 rows

; P1.4 will blank display at end of current Frame if High , re-start on falling edge
;P1.4 Setup Vars:
BLANK         equ       BIT4 
BLANK_OUT     equ       P1OUT
BLANK_DIR     equ       P1DIR
BLANK_IN      equ       P1IN
BLANK_IE      equ       P1IE
BLANK_IES     equ       P1IES
BLANK_IFG     equ       P1IFG
BLANK_REN     equ       P1REN



  org 0xe800 ;start of RAM
start:
  mov.w #WDTPW|WDTHOLD, &WDTCTL
  dint ; No interupts please
  
  ;store bitmasks for mainloop in registers to save cycles
  mov.w #0x0010,MASK1 ; mask clk bit
  mov.w #0x005f, MASK2 ;set bits 0-3,4 and 6 (Data,Clock and FLM)
  mov.w #0x00f0,MASK3 ; set all high bits
  
  mov.b &CALBC1_16MHZ,&BCSCTL1 ;set clock
  mov.b &CALDCO_16MHZ,&DCOCTL
  
  mov.w #0x2ff,SP ; setup stack
  
  ;set up USI for master
  mov.b #(USIPE7 | USIPE6 | USIPE5 | USIMST | USIOE | USILSB),&USICTL0; // Port, SPI master, LSB first
  mov.b #(USIDIV_1|USISSEL_2),&USICKCTL ; set USICLK to  SMCLK/2 
  bic.b #USIIFG,&USICTL1 ; clear USI interupt
  bic.b #USISWRST,&USICTL0 ; USI released for operation
  
  ;initialize counters
  mov PIXELS,PIXCNT ;16bits * 20 = 320 pixels
  mov ROWS,ROWCNT
  
  mov.b #0x00, &P2SEL ; Turn off XTal
  mov.b #0xff, &P2DIR ;set all of P2 as output
  
  SetupP1:   
  mov.b   #0x0,&P1OUT		  ;turn off all P1 outputs
  bis.b   #0x07,&P1DIR            ; P1.0,P1.1 and P1.2 output
  
  
Blank: ;label for blanking display
  bis.b #0x02,&P1OUT ;turn off VEE - Inverted signal
  bic.b #0x01,&P1OUT ;turn off backlight and 5V for display
  
  ; set up p1.4 interupt on falling edge
  bis.b #BLANK,&BLANK_REN
  bic.b #BLANK,&BLANK_OUT
  bis.b #BLANK,&BLANK_IES
  bic.b #BLANK,&BLANK_IFG
  bis.b #BLANK,&BLANK_IE
  
  eint ;allow interupts so we can wake back up
  bis.b #(LPM0 + GIE),SR ;go to sleep, wake up on interupt Slave needs to setup first Pixel before releasing P1.4
  dint ; No interupts please
  
  
  
  mov #0x3210,&USISR; initialize USISR, usefull for debugging SPI stream
  
  mov.b #0x50,&USICNT ;get first pixel
  mov.w #32,LOWNIB ; 32 Cycle delay to make sure Slave is ready
Wait:
  dec LOWNIB
  jnz Wait
  
  bis.b #0x01,&P1OUT ;turn on backlight and +5V for Display
  
MainLoop:
  mov &USISR,LOWNIB ; get current pixels
  mov.b #0x50,&USICNT ;start next xfer
  
  swpb LOWNIB ; swap endianness
  mov LOWNIB,HIGHNIB ; make copy for high nibbles
  ; shift high nibble right by 4 bits
  rra HIGHNIB
  rra HIGHNIB
  rra HIGHNIB
  rra HIGHNIB
  
  mov LOWNIB,OUTVAL ;put first nibble in output 
  bis MASK3,OUTVAL ; set high bits to 1
  and MASK2,OUTVAL ; mask data, clock and flm bits
  mov.b OUTVAL,&P2OUT ;send output to port 2
  bic.b MASK1,&P2OUT ; toggle clock
  
  mov HIGHNIB,OUTVAL ;put second nibble in output
  bis MASK3,OUTVAL ;set high bits to 1
  and MASK2,OUTVAL ;mask data, clock and flm bits
  mov.b OUTVAL,&P2OUT ;send output to port 2
  swpb LOWNIB ; swap bits in LOWNIB
  bic.b MASK1,&P2OUT ;toggle clock
  
  mov LOWNIB,OUTVAL ;put third nibble in output 
  bis MASK3,OUTVAL ;set high bits to 1
  and MASK2,OUTVAL ;mask data, clock and flm bits
  mov.b OUTVAL,&P2OUT ;send output to port 2
  swpb HIGHNIB ; swap bits in HIGHNIB
  bic.b MASK1,&P2OUT ;toggle clock
  
  mov HIGHNIB,OUTVAL ;put fourth nibble in output 
  bis MASK3,OUTVAL ;set high bits to 1
  and MASK2,OUTVAL ;mask data, clock and flm bits
  mov.b OUTVAL,&P2OUT ;send output to port 2 
  bic.b MASK1,&P2OUT ;toggle clock
 
  dec PIXCNT ; PICCNT--
  ;end of row ?
  jnz MainLoop
  
  bis.b #0x20,&P2OUT ;set row clock high
  
  bic.b #0x40,MASK2 ;turn off FLM bit in MASK2
  xor.b #0x80,MASK2 ;toggle M in MASK2
  mov PIXELS,PIXCNT ;reload pixel counter
  bic.b #0x20,&P2OUT ; load row on falling edge

  dec ROWCNT ;ROWCNT--
  ;end of screen ?
  jnz MainLoop
 
  bis.b #0x40,MASK2; set flm bit in MASK2
  bic.b #0x02,&P1OUT ; turn on VEE - gives a 1/50 s delay between +5V and VEE to prevent powerup issues
 
  mov ROWS,ROWCNT ;reset row counter
  bit.b #BLANK,&P1IN ; check p1.4, if high, stop updating screen
  jc Blank
  jmp MainLoop ; start over 

Wakeup_int:
  mov.b #0,&BLANK_IFG ;clear interupt
  bic.b #BLANK,&BLANK_IE ;disable interrupt
  bic.w #CPUOFF,0(SP) ; wakeup cpu
  reti
  
  ;Vector Table
 org 0xffe4
vectors:
  dw Wakeup_int ;18
  dw 0 ;19
  dw 0 ;20
  dw 0 ;21
  dw 0 ;22
  dw 0 ;23      
  dw 0 ;24
  dw 0 ;25
  dw 0 ;26
  dw 0 ;27
  dw 0 ;28
  dw 0 ;29
  dw 0 ;30
  dw start                 ; Reset 31


