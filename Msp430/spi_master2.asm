
.include "msp430g24x2.inc"
LOWNIB equ r6
HIGHNIB equ r7
OUTVAL equ r8
MASK1 equ r9
MASK2 equ r10
MASK3 equ r11
PIXCNT equ r12
ROWCNT equ r13

PIXELS equ #20
ROWS   equ #240


BLANK         equ       BIT4 ; P1.4 will blank display at end of current Frame if High , re-start on falling edge
BLANK_OUT     equ       P1OUT
BLANK_DIR     equ       P1DIR
BLANK_IN      equ       P1IN
BLANK_IE      equ       P1IE
BLANK_IES     equ       P1IES
BLANK_IFG     equ       P1IFG
BLANK_REN     equ       P1REN

DATA_RDY      equ       BIT3 ; use P1.3 to signal Data Ready at end of line

  org 0xe800
start:
  ;mov.w #0x5a80, &WDTCTL
  mov.w #WDTPW|WDTHOLD, &WDTCTL
  dint ; No interupts please
  
  ;store bitmasks for mainloop in registers to save cycles
  mov.w #0x0010,MASK1
  mov.w #0x005f, MASK2 ;set bits 0-3,4 and 6 (Data,Clock and FLM)
  mov.w #0x00f0,MASK3 ; set all high bits
  
  mov.b &CALBC1_16MHZ,&BCSCTL1 ;set clock
  mov.b &CALDCO_16MHZ,&DCOCTL
  
  mov.w #0x2ff,SP ; setup stack
  
  ;set up USI for master
  mov.b #(USIPE7 | USIPE6 | USIPE5 | USIMST | USIOE | USILSB),&USICTL0; // Port, SPI master, LSB first
  ;bis.b USIIE,&USICTL1 ; Counter interrupt, flag remains set
  mov.b #(USIDIV_1|USISSEL_2),&USICKCTL ; /2 SMCLK
  ;bis.b #USICKPH,&USICTL1 ; set clock phase to 1 to match stellaris
  bic.b #USIIFG,&USICTL1
  bic.b #USISWRST,&USICTL0 ; USI released for operation
  
  ;initialize counters
  mov PIXELS,PIXCNT ;16bits * 20 = 320 pixels
  mov ROWS,ROWCNT
  
  mov.b #0x00, &P2SEL ; Turn off XTal
  mov.b #0xff, &P2DIR ;set all of P2 as output
  ;mov.b #0x01, &P1DIR ;set  P1.0 as output
  SetupP1:   
  mov.b   #0x0,&P1OUT		  ;turn off all P1 outputs
  bis.b   #0x07,&P1DIR            ; P1.0,P1.1 and P1.2 output
  bis.b	  #0x08,&P1REN		  ; Set P1.3 Pulldown
  ;bis.b   #0x0C,&P1SEL            ; P1.2 and P1.3 TA1/2 otions
  ;SetupC0:     
  ;mov.w   #160,&TACCR0              ; PWM Period/2
  ;SetupC1:    
  ;mov.w   #OUTMOD_4,&TACCTL1        ; CCR1 toggle
  ;mov.w   #80,&TACCR1               ; CCR1 PWM Duty Cycle	
  ;SetupTA:     
  ;mov.w   #TASSEL_2+MC_0,&TACTL   ; SMCLK, Stop Timer for now
  ;bis.b #0x01,&P1OUT
  mov #0x3210,&USISR
Blank:
  bic.b #0x01,&P1OUT ;turn off backlight
  bis.b #0x02,&P1OUT; turn off VEE
  ;mov.w #MC_0,&TACTL ; Turn off Timer
  ; set up p1.4 interupt on falling edge
  bis.b #BLANK,&BLANK_REN
  bic.b #BLANK,&BLANK_OUT
  bis.b #BLANK,&BLANK_IES
  bic.b #BLANK,&BLANK_IFG
  bis.b #BLANK,&BLANK_IE
  
  eint ;allow interupts so we can wake back up
  bis.b #(LPM0 + GIE),SR ;go to sleep, wake up on interupt Slave needs to setup first Pixel before releasing P1.4
  dint ; No interupts please
  
  
  
  mov #0x3210,&USISR
  
  mov.b #0x50,&USICNT ;get first pixel
  mov.w #32,LOWNIB
Wait:
  dec LOWNIB
  jnz Wait
  

  
  bis.b #0x01,&P1OUT ;turn on backlight
  
MainLoop:
  mov &USISR,LOWNIB ; get current pixel
  mov.b #0x50,&USICNT ;start next xfer
  
  swpb LOWNIB ; swap endianness
  mov LOWNIB,HIGHNIB
  rra HIGHNIB
  rra HIGHNIB
  rra HIGHNIB
  rra HIGHNIB
  mov LOWNIB,OUTVAL
  bis MASK3,OUTVAL
  and MASK2,OUTVAL
  mov.b OUTVAL,&P2OUT
  
  bic.b MASK1,&P2OUT ; toggle clock
  
  ;bic.b #USIIFG,&USICTL1
  mov HIGHNIB,OUTVAL
  bis MASK3,OUTVAL ;set clock and flm bits
  and MASK2,OUTVAL
  mov.b OUTVAL,&P2OUT
  swpb LOWNIB
  bic.b MASK1,&P2OUT ;toggle clock
  mov LOWNIB,OUTVAL ;swap LOWNIB to get second Pixel
  bis MASK3,OUTVAL ;set clock and flm bits
  and MASK2,OUTVAL
  mov.b OUTVAL,&P2OUT
  swpb HIGHNIB ; swap HIGHNIB to get 2nd Pixel
  bic.b MASK1,&P2OUT ;toggle clock
  mov HIGHNIB,OUTVAL
  bis MASK3,OUTVAL ;set clock and flm bits
  and MASK2,OUTVAL
  mov.b OUTVAL,&P2OUT
  
  bic.b MASK1,&P2OUT ;toggle clock
  ;bis.b #0x04,&P1OUT ; set FS high
  dec PIXCNT
  ;end of row ?
  jnz MainLoop
  ;toggle row clock
  bis.b #0x20,&P2OUT
  
  bic.b #0x40,MASK2 ;turn off FLM
  xor.b #0x80,MASK2 ;toggle M
  mov PIXELS,PIXCNT ;reload pixel counter
  bic.b #0x20,&P2OUT ; load row
;Data_Rdy:
 ; bit.b #DATA_RDY,&P1IN ; check p1.3, if high, stop updating screen
  ;jeq Data_Rdy
  dec ROWCNT
  ;end of screen ?
  jnz MainLoop
  
  
  bis.b #0x40,MASK2; set flm bit
  bic.b #0x02,&P1OUT ; turn on VEE
  ;bis.b #0x01,&P1OUT ;turn on LED
 
  mov ROWS,ROWCNT ;reset row counter
  bit.b #BLANK,&P1IN ; check p1.4, if high, stop updating screen
  jc Blank
  jmp MainLoop ; start over 

Wakeup_int:
  mov.b #0,&BLANK_IFG
  bic.b #BLANK,&BLANK_IE
  bic.w #CPUOFF,0(SP)
  reti
  
  ;org 0xfffe
  ;dw start             ; set reset vector to 'init' label
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


