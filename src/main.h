//*****************************************************************************
// Variables and Function Prototypes for main.c
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



//*****************************************************************************
//
// The number of SysTick ticks per second used for the SysTick interrupt.
//
//*****************************************************************************
#define SYSTICKS_PER_SECOND     100




#define TEST_TIME		3
#define TEST_LOOPS		6

#define APP_INPUT_BUF_SIZE               128



//*****************************************************************************
//
// The count of uDMA errors.  This value is incremented by the uDMA error
// handler.
//
//*****************************************************************************
static unsigned long g_uluDMAErrCount = 0;


//*****************************************************************************
//
// The count of UART buffers filled, one for each ping-pong buffer.
//
//*****************************************************************************

unsigned long g_ulRxBufBCount = 0;

//*****************************************************************************
//
// The count of memory uDMA transfer blocks.  This value is incremented by the
// uDMA interrupt handler whenever a memory block transfer is completed.
//
//*****************************************************************************
static unsigned long g_ulMemXferCount = 0;

//*****************************************************************************
//
// The CPU usage in percent, in 16.16 fixed point format.
//
//*****************************************************************************
static unsigned long g_ulCPUUsage;

//*****************************************************************************
//
// The number of seconds elapsed since the start of the program.  This value is
// maintained by the SysTick interrupt handler.
//
//*****************************************************************************
static unsigned long g_ulSeconds = 0;

// Dummy buffer for Rx from SSI
unsigned short g_uiSsiRxBuf[SSI_RXBUF_SIZE];

//pointer for Base address for SSI Transfers.
unsigned short *g_uiSsiTxBufBase;
// Base A and Base B are the base address in the Framebuffers
unsigned short *g_uiSsiTxBufBaseA;
unsigned short *g_uiSsiTxBufBaseB;


// Flag to exit program
unsigned char g_ucExit =0;


//*****************************************************************************
//
// Define a 1BPP offscreen buffer and display structure.
//
//*****************************************************************************
#define OFFSCREEN_BUF_SIZE GrOffScreen1BPPSize(320, 240)
unsigned char g_pucOffscreenBufA[OFFSCREEN_BUF_SIZE+1] __attribute__ ((aligned(2))); //make sure the buffer is aligned on a 16 bit boundry
tDisplay g_sOffscreenDisplayA;
// Context for Active Display
tContext sDisplayContext;


//array for string data to display (53 cols+null*30 rows)
char g_ucScreenText[30][54]={{'0'}};


