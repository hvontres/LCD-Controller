#include "common.h"
#include "spi_xfer.h"
#include "globals.h"

//*****************************************************************************
//
// The interrupt handler for SSI2.  This interrupt will occur when a DMA
// transfer is complete using the SSI2 uDMA channel.  It will also be
// triggered if the peripheral signals an error.  This interrupt handler will
// switch between Transmit ping-pong buffers A and B and will cycle buffers  
// A and B all the way through the Frame Buffer. It will also restart a RX
// uDMA transfer if the prior transfer is complete.  This will keep the SSI2
// running continuously.
//
//*****************************************************************************
void
SSI2IntHandler(void)
{
    unsigned long ulStatus;
    unsigned long ulMode;
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_PIN_3); // Turn on pin E3 for debug and timing
    
    //
    // Read the interrupt status of the UART.
    //
    ulStatus = ROM_SSIIntStatus(SSI2_BASE, 1);

    //
    // Clear any pending status, even though there should be none since no UART
    // interrupts were enabled.  If UART error interrupts were enabled, then
    // those interrupts could occur here and should be handled.  Since uDMA is
    // used for both the RX and TX, then neither of those interrupts should be
    // enabled.
    //
    ROM_SSIIntClear(SSI2_BASE, ulStatus);

    //
    // Check the DMA control table to see if the ping-pong "A" transfer is
    // complete.  The "A" transfer uses Transmit buffer "A", and the primary
    // control structure.
    //
    ulMode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_SSI2TX | UDMA_PRI_SELECT);

    //
    // If the primary control structure indicates stop, that means the "A"
    // transmit buffer is done.  The uDMA controller should still be transmitting
    // data from the "B" buffer.
    //
#if 1
    if(ulMode == UDMA_MODE_STOP)

    {
       
        //
        // Increment a counter to indicate data was received into buffer A.  In
        // a real application this would be used to signal the main thread that
        // data was received so the main thread can process the data.
        //
        if (++g_ulBufferSegCount>=3) {
	  g_ulBufferSegCount=0;
	}
	
	//UARTprintf("%i",g_ulBufferSegCount);
        //
        // Set up the next transfer for the "A" buffer, using the primary
        // control structure.  When the ongoing receive into the "B" buffer is
        // done, the uDMA controller will switch back to this one.  This
        // example re-uses buffer A, but a more sophisticated application could
        // use a rotating set of buffers to increase the amount of time that
        // the main thread has to process the data in the buffer before it is
        // reused.
        //
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SSI2TX | UDMA_PRI_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)(g_uiSsiTxBufBase+g_ulBufferSegCount*2*SSI_TXBUF_SIZE),
                                   (void *)(SSI2_BASE + SSI_O_DR),
			           (unsigned long)SSI_TXBUF_SIZE);
	//GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0);
    }

    //
    // Check the DMA control table to see if the ping-pong "B" transfer is
    // complete.  The "B" transfer uses receive buffer "B", and the alternate
    // control structure.
    //
    ulMode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_SSI2TX | UDMA_ALT_SELECT);

    //
    // If the alternate control structure indicates stop, that means the "B"
    // receive buffer is done.  The uDMA controller should still be receiving
    // data into the "A" buffer.
    //

    if(ulMode == UDMA_MODE_STOP)
    {
       //GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_PIN_3);
        //
        

        //
        // Set up the next transfer for the "B" buffer, using the alternate
        // control structure.  When the ongoing receive into the "A" buffer is
        // done, the uDMA controller will switch back to this one.  This
        // example re-uses buffer B, but a more sophisticated application could
        // use a rotating set of buffers to increase the amount of time that
        // the main thread has to process the data in the buffer before it is
        // reused.
        //
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SSI2TX | UDMA_ALT_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)(g_uiSsiTxBufBase+(g_ulBufferSegCount*2+1)*SSI_TXBUF_SIZE),
                                   (void *)(SSI2_BASE + SSI_O_DR),
				   (unsigned long)SSI_TXBUF_SIZE);
	//GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0);
    }
#endif
    //
    // If the UART1 DMA TX channel is disabled, that means the RX DMA transfer
    // is done.
    //
    if(!ROM_uDMAChannelIsEnabled(UDMA_CHANNEL_SSI2RX))
    {
	// Increment a counter to indicate data was received into buffer .
        //
        g_ulRxBufBCount++;
        //
        // Start another DMA transfer from SSI2RX.
        //
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SSI2RX | UDMA_PRI_SELECT,
				   UDMA_MODE_BASIC,
                                   (void *)(SSI2_BASE + SSI_O_DR),
                                   (void *)g_uiSsiRxBuf,
                                   (unsigned long)SSI_RXBUF_SIZE);

        //
        // The uDMA TX channel must be re-enabled.
        //
        ROM_uDMAChannelEnable(UDMA_CHANNEL_SSI2RX);
    }
    //GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0);
    //GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0); // turn off pin E3
}





//*****************************************************************************
//
// Initialize the SSI2 peripheral and sets up the TX and RX uDMA channels.
// The uDMA channels are configured so that the RX channel
// will copy data to a dummy buffer.  And the uDMA TX channel
// will copy data from Frame Buffer A in ping-pong mode.
//
//*****************************************************************************
void
InitSSI2Transfer(void)
{
    unsigned long SysClock=ROM_SysCtlClockGet();

    //
    // Enable the SSI peripheral, and configure it to operate even if the CPU
    // is in sleep.
    //
    
    g_ulBufferSegCount=0; 
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_SSI2);
    UARTprintf("%u\n",SysClock);
    UARTprintf("ptr_base: %X \n",g_uiSsiTxBufBase);
 
    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_PIN_3); // Turn on pin E3
    GPIOPinConfigure(GPIO_PB5_SSI2FSS);
    GPIOPinConfigure(GPIO_PB6_SSI2RX);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);
    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    //
    // Setup the SSI communication parameters.
    
    ROM_SSIConfigSetExpClk(SSI2_BASE, SysClock,
                             SSI_FRF_MOTO_MODE_1,
			     SSI_MODE_SLAVE,
			     1000000,
			     16);

 
    
    HWREG(SSI2_BASE + SSI_O_CR1) |= 0x00000020; // set Slave Bypass mode

    //
    // Enable the SSI for operation, and enable the uDMA interface for both TX
    // and RX channels.
    //
    ROM_SSIEnable(SSI2_BASE);
    ROM_SSIDMAEnable(SSI2_BASE, SSI_DMA_RX | SSI_DMA_TX);

    
    //
    // Enable the SSI peripheral interrupts.  Note that no SSI interrupts
    // were enabled, but the uDMA controller will cause an interrupt on the
    // SSI interrupt signal when a uDMA transfer is complete.
    //
    ROM_IntEnable(INT_SSI2);

    //
    // Put the attributes in a known state for the uDMA SSI2RX channel.  These
    // should already be disabled by default.
    //
    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_SSI2TX,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    ROM_uDMAChannelAssign(UDMA_CH13_SSI2TX);
    //
    // Configure the control parameters for the primary control structure for
    // the SSI RX channel.  The primary contol structure is used for the "A"
    // part of the ping-pong receive.  The transfer data size is 8 bits, the
    // source address does not increment since it will be reading from a
    // register.  The destination address increment is byte 8-bit bytes.  The
    // arbitration size is set to 4 to match the RX FIFO trigger threshold.
    // The uDMA controller will use a 4 byte burst transfer if possible.  This
    // will be somewhat more effecient that single byte transfers.
    //
    ROM_uDMAChannelControlSet(UDMA_CHANNEL_SSI2TX | UDMA_PRI_SELECT,
                              UDMA_SIZE_16 | UDMA_SRC_INC_16 | UDMA_DST_INC_NONE |
                              UDMA_ARB_4);

    //
    // Configure the control parameters for the alternate control structure for
    // the SSI RX channel.  The alternate contol structure is used for the "B"
    // part of the ping-pong receive.  The configuration is identical to the
    // primary/A control structure.
    //
    ROM_uDMAChannelControlSet(UDMA_CHANNEL_SSI2TX | UDMA_ALT_SELECT,
                              UDMA_SIZE_16 | UDMA_SRC_INC_16 | UDMA_DST_INC_NONE |
                              UDMA_ARB_4);

    //
    // Set up the transfer parameters for the SSI RX primary control
    // structure.  The mode is set to ping-pong, the transfer source is the
    // SSI data register, and the destination is the receive "A" buffer.  The
    // transfer size is set to match the size of the buffer.
    //
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SSI2TX | UDMA_PRI_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)g_uiSsiTxBufBase,
                               (void *)(SSI2_BASE + SSI_O_DR),
			       SSI_TXBUF_SIZE);

    //
    // Set up the transfer parameters for the SSI RX alternate control
    // structure.  The mode is set to ping-pong, the transfer source is the
    // SSI data register, and the destination is the receive "B" buffer.  The
    // transfer size is set to match the size of the buffer.
    //
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SSI2TX | UDMA_ALT_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)(g_uiSsiTxBufBase+SSI_TXBUF_SIZE),
                               (void *)(SSI2_BASE + SSI_O_DR),
			       SSI_TXBUF_SIZE);
    
    ROM_uDMAChannelAttributeEnable(UDMA_CHANNEL_SSI2TX, UDMA_ATTR_USEBURST);
    
    UARTprintf("A_base: %X \n",(void *)g_uiSsiTxBufBase);
    UARTprintf("B_base: %X \n",(void *)(g_uiSsiTxBufBase+SSI_TXBUF_SIZE));

    //
    // Put the attributes in a known state for the uDMA SSI2TX channel.  These
    // should already be disabled by default.
    //
    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_SSI2RX,
                                    UDMA_ATTR_ALTSELECT |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    ROM_uDMAChannelAssign(UDMA_CH12_SSI2RX);
    //
    // Set the USEBURST attribute for the uDMA SSI RX channel.  This will
    // force the controller to always use a burst when transferring data from
    // the TX buffer to the SSI.  This is somewhat more effecient bus usage
    // than the default which allows single or burst transfers.
    //
    ROM_uDMAChannelAttributeEnable(UDMA_CHANNEL_SSI2RX, UDMA_ATTR_USEBURST);

    //
    // Configure the control parameters for the SSI TX.  The uDMA SSI TX
    // channel is used to transfer a block of data from a buffer to the SSI.
    // The data size is 8 bits.  The source address increment is 8-bit bytes
    // since the data is coming from a buffer.  The destination increment is
    // none since the data is to be written to the SSI data register.  The
    // arbitration size is set to 4, which matches the SSI TX FIFO trigger
    // threshold.
    //
    ROM_uDMAChannelControlSet(UDMA_CHANNEL_SSI2RX | UDMA_PRI_SELECT,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 |
                              UDMA_ARB_4);

    //
    // Set up the transfer parameters for the uDMA SSI TX channel.  This will
    // configure the transfer source and destination and the transfer size.
    // Basic mode is used because the peripheral is making the uDMA transfer
    // request.  The source is the TX buffer and the destination is the SSI
    // data register.
    //
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SSI2RX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               (void *)(SSI2_BASE + SSI_O_DR),
			       (void *)g_uiSsiRxBuf,
                               SSI_RXBUF_SIZE);

    //
    // Now both the uDMA SSI TX and RX channels are primed to start a
    // transfer.  As soon as the channels are enabled, the peripheral will
    // issue a transfer request and the data transfers will begin.
    //
    ROM_uDMAChannelEnable(UDMA_CHANNEL_SSI2TX);
    ROM_uDMAChannelEnable(UDMA_CHANNEL_SSI2RX);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0); //Turn off pin E3
}


void
DisableSSI2Transfer(void)
{
    ROM_uDMAChannelDisable(UDMA_CHANNEL_SSI2TX);
    ROM_uDMAChannelDisable(UDMA_CHANNEL_SSI2RX);
    ROM_IntDisable(INT_SSI2);
    ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_SSI2);
    ROM_SysCtlPeripheralDisable(SYSCTL_PERIPH_SSI2);  
}

void
SwitchBuffers(void)
{
  if (sDisplayContext.pDisplay==&g_sOffscreenDisplayA){
    sDisplayContext.pDisplay=&g_sOffscreenDisplayA;
    g_uiSsiTxBufBase=g_uiSsiTxBufBaseA;
  }
  else{ //switch from B to A
    sDisplayContext.pDisplay=&g_sOffscreenDisplayA;
    g_uiSsiTxBufBase=g_uiSsiTxBufBaseA;
  }
}

