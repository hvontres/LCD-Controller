//*****************************************************************************
// Variable Declarations and Prototypes for SPI Transfers
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

#define SSI_TXBUF_SIZE         800
#define SSI_RXBUF_SIZE         800

#define UDMA_CHANNEL_SSI2RX    12
#define UDMA_CHANNEL_SSI2TX    13




// Config Vars for SSI Selection - use ifdefs to setup for SSI_0,SSI_1A,SSI_1B,SSI_2 or SSI_3

// SSI_BASE
// UDMA_CHANNEL_RX
// UDMA_CHANNEL_TX
// 
// PERIPH_SSI
// PERIPH_GPIO
// GPIO_PORT
// GPIO_PINS
// 
// SSI_CLK_PIN
// SSI_RX_PIN
// SSI_TX_PIN
// 
// INT_SSI
// UDMA_RX_CH
// UDMA_TX_CH
// 

static unsigned long g_ulBufferSegCount;

void SSI2IntHandler(void);
void InitSSI2Transfer(void);
void DisableSSI2Transfer(void);



