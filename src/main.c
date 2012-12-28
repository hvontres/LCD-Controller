//*****************************************************************************
//
// main.c - Main LCD Controller Code.
//
// 
//
//*****************************************************************************

#include "common.h"
#include "spi_xfer.h"
#include "main.h"


#include "dp.h"






//#define ROWS 240
//#define PIXELS 20
//const unsigned int g_u16PixelData[ROWS][PIXELS];



//*****************************************************************************
//
// A spinning line that is used to indicate that the application is running.
//
//*****************************************************************************
//static const char g_pcTwirl[4] = { '\\', '|', '/', '-' };

//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
//
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
unsigned char g_ucControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(g_ucControlTable, 1024)
unsigned char g_ucControlTable[1024];
#else
static unsigned char g_ucControlTable[1024] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
// The interrupt handler for the SysTick timer.  This handler will increment a
// seconds counter whenever the appropriate number of ticks has occurred.  It
// will also call the CPU usage tick function to find the CPU usage percent.
//
//*****************************************************************************
void
SysTickHandler(void)
{
    static unsigned long ulTickCount = 0;

    //
    // Increment the tick counter.
    //
    ulTickCount++;

    //
    // If the number of ticks per second has occurred, then increment the
    // seconds counter.
    //
    if(!(ulTickCount % SYSTICKS_PER_SECOND))
    {
        g_ulSeconds++;
    }

    //
    // Call the CPU usage tick function.  This function will compute the amount
    // of cycles used by the CPU since the last call and return the result in
    // percent in fixed point 16.16 format.
    //
    g_ulCPUUsage = CPUUsageTick();
}

//*****************************************************************************
//
// The interrupt handler for uDMA errors.  This interrupt will occur if the
// uDMA encounters a bus error while trying to perform a transfer.  This
// handler just increments a counter if an error occurs.
//
//*****************************************************************************
void
uDMAErrorHandler(void)
{
    unsigned long ulStatus;

    //
    // Check for uDMA error bit
    //
    ulStatus = ROM_uDMAErrorStatusGet();

    //
    // If there is a uDMA error, then clear the error and increment
    // the error counter.
    //
    if(ulStatus)
    {
        ROM_uDMAErrorStatusClear();
        g_uluDMAErrCount++;
    }
}

//*****************************************************************************
//
// The interrupt handler for uDMA interrupts from the memory channel.  This
// interrupt will increment a counter, and then restart another memory
// transfer.
//
//*****************************************************************************
void
uDMAIntHandler(void)
{
    unsigned long ulMode;

    //
    // Check for the primary control structure to indicate complete.
    //
    ulMode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_SW);
    if(ulMode == UDMA_MODE_STOP)
    {
        //
        // Increment the count of completed transfers.
        //
        g_ulMemXferCount++;

        //
        // Configure it for another transfer.
        //
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SW, UDMA_MODE_AUTO,
                                   g_ulSrcBuf, g_ulDstBuf, MEM_BUFFER_SIZE);

        //
        // Initiate another transfer.
        //
        ROM_uDMAChannelEnable(UDMA_CHANNEL_SW);
        ROM_uDMAChannelRequest(UDMA_CHANNEL_SW);
    }

    //
    // If the channel is not stopped, then something is wrong.
    //
    else
    {
        g_ulBadISR++;
    }
}



//*****************************************************************************
//
// Initializes the uDMA software channel to perform a memory to memory uDMA
// transfer.
//
//*****************************************************************************
void
InitSWTransfer(void)
{
    unsigned int uIdx;

    //
    // Fill the source memory buffer with a simple incrementing pattern.
    //
    for(uIdx = 0; uIdx < MEM_BUFFER_SIZE; uIdx++)
    {
        g_ulSrcBuf[uIdx] = uIdx;
    }

    //
    // Enable interrupts from the uDMA software channel.
    //
    ROM_IntEnable(INT_UDMA);

    //
    // Put the attributes in a known state for the uDMA software channel.
    // These should already be disabled by default.
    //
    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_SW,
                                    UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT |
                                    (UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK));

    //
    // Configure the control parameters for the SW channel.  The SW channel
    // will be used to transfer between two memory buffers, 32 bits at a time.
    // Therefore the data size is 32 bits, and the address increment is 32 bits
    // for both source and destination.  The arbitration size will be set to 8,
    // which causes the uDMA controller to rearbitrate after 8 items are
    // transferred.  This keeps this channel from hogging the uDMA controller
    // once the transfer is started, and allows other channels cycles if they
    // are higher priority.
    //
    ROM_uDMAChannelControlSet(UDMA_CHANNEL_SW | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_32 | UDMA_DST_INC_32 |
                              UDMA_ARB_8);

    //
    // Set up the transfer parameters for the software channel.  This will
    // configure the transfer buffers and the transfer size.  Auto mode must be
    // used for software transfers.
    //
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SW | UDMA_PRI_SELECT,
                               UDMA_MODE_AUTO, g_ulSrcBuf, g_ulDstBuf,
                               MEM_BUFFER_SIZE);

    //
    // Now the software channel is primed to start a transfer.  The channel
    // must be enabled.  For software based transfers, a request must be
    // issued.  After this, the uDMA memory transfer begins.
    //
    ROM_uDMAChannelEnable(UDMA_CHANNEL_SW);
    ROM_uDMAChannelRequest(UDMA_CHANNEL_SW);
}

//*****************************************************************************
//
// This example demonstrates how to use the uDMA controller to transfer data
// between memory buffers and to and from a peripheral, in this case a UART.
// The uDMA controller is configured to repeatedly transfer a block of data
// from one memory buffer to another.  It is also set up to repeatedly copy a
// block of data from a buffer to the UART output.  The UART data is looped
// back so the same data is received, and the uDMA controlled is configured to
// continuously receive the UART data using ping-pong buffers.
//
// The processor is put to sleep when it is not doing anything, and this allows
// collection of CPU usage data to see how much CPU is being used while the
// data transfers are ongoing.
//
//*****************************************************************************
int
main(void)
{
    static unsigned long ulPrevSeconds;
    static unsigned long ulPrevXferCount;
    static unsigned long ulPrevUARTCount = 0;
    unsigned long ulXfersCompleted;
    unsigned long ulBytesTransferred;
    volatile unsigned long ulLoop;

    //
    g_uiSsiTxBufBase=(unsigned short *)g_u16PixelData;
    
    g_uiSsiTxBufB=g_uiSsiTxBufBase+SSI_TXBUF_SIZE;
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
 // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPULazyStackingEnable();

    //
    // Set the clocking to run from the PLL at 50 MHz.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    //
    // Enable peripherals to operate when CPU is in sleep.
    //
    ROM_SysCtlPeripheralClockGating(true);

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Enable the GPIO pins for the LED (PF2).  
    //
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    
    // Reset Pin
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
    
    //Blank Pin
    
    //ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
   
    //
    // Initialize the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);
    UARTprintf("\033[2JuDMA Example\n");

    //
    // Show the clock frequency on the display.
    //
    UARTprintf("Stellaris @ %u MHz\n\n", ROM_SysCtlClockGet() / 1000000);
    UARTprintf("ptr_base:%x \n",g_uiSsiTxBufBase);
    //
    // Show statistics headings.
    //
    UARTprintf("CPU    Memory     UART       Remaining\n");
    UARTprintf("Usage  Transfers  Transfers  Time\n");

    //
    // Configure SysTick to occur 100 times per second, to use as a time
    // reference.  Enable SysTick to generate interrupts.
    //
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();

    //
    // Initialize the CPU usage measurement routine.
    //
    CPUUsageInit(ROM_SysCtlClockGet(), SYSTICKS_PER_SECOND, 2);

    //
    // Enable the uDMA controller at the system level.  Enable it to continue
    // to run while the processor is in sleep.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);

    //
    // Enable the uDMA controller error interrupt.  This interrupt will occur
    // if there is a bus error during a transfer.
    //
    ROM_IntEnable(INT_UDMAERR);

    //
    // Enable the uDMA controller.
    //
    ROM_uDMAEnable();

    //
    // Point at the control table to use for channel control structures.
     //
    ROM_uDMAControlBaseSet(g_ucControlTable);

    //
    // Initialize the uDMA memory to memory transfers.
    //
    //InitSWTransfer();

    
    //Reset Set BLAK HIGH and reset MSP430
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4);
    
    //
    // Initialize the uDMA UART transfers.
    //
    InitSSI2Transfer();
    //InitUART1Transfer();

    // Release Blank Pin
    SysCtlDelay(SysCtlClockGet() / 20 / 3);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0);
    //
    // Remember the current SysTick seconds count.
    //
    ulPrevSeconds = g_ulSeconds;

    //
    // Remember the current count of memory buffer transfers.
    //
    ulPrevXferCount = g_ulMemXferCount;

    //
    // Loop until the button is pressed.  The processor is put to sleep
    // in this loop so that CPU utilization can be measured.
    //
    while(1)
    {
        //
        // Check to see if one second has elapsed.  If so, the make some
        // updates.
        //
        if(g_ulSeconds != ulPrevSeconds)
        {
            //
            // Turn on the LED as a heartbeat
            //
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
            
            //
            // Print a message to the display showing the CPU usage percent.
            // The fractional part of the percent value is ignored.
            //
            UARTprintf("\r%3d%%   ", g_ulCPUUsage >> 16);
            
            //
            // Remember the new seconds count.
            //
            ulPrevSeconds = g_ulSeconds;

            //
            // Calculate how many memory transfers have occurred since the last
            // second.
            //
            ulXfersCompleted = g_ulMemXferCount - ulPrevXferCount;

            //
            // Remember the new transfer count.
            //
            ulPrevXferCount = g_ulMemXferCount;

            //
            // Compute how many bytes were transferred in the memory transfer
            // since the last second.
            //
            ulBytesTransferred = ulXfersCompleted * MEM_BUFFER_SIZE * 4;

            //
            // Print a message showing the memory transfer rate.
            //
            if(ulBytesTransferred >= 100000000)
            {
                UARTprintf("%3d MB/s   ", ulBytesTransferred / 1000000);
            }
            else if(ulBytesTransferred >= 10000000)
            {
                UARTprintf("%2d.%01d MB/s  ", ulBytesTransferred / 1000000,
                           (ulBytesTransferred % 1000000) / 100000);
            }
            else if(ulBytesTransferred >= 1000000)
            {
                UARTprintf("%1d.%02d MB/s  ", ulBytesTransferred / 1000000,
                           (ulBytesTransferred % 1000000) / 10000);
            }
            else if(ulBytesTransferred >= 100000)
            {
                UARTprintf("%3d KB/s   ", ulBytesTransferred / 1000);
            }
            else if(ulBytesTransferred >= 10000)
            {
                UARTprintf("%2d.%01d KB/s  ", ulBytesTransferred / 1000,
                           (ulBytesTransferred % 1000) / 100);
            }
            else if(ulBytesTransferred >= 1000)
            {
                UARTprintf("%1d.%02d KB/s  ", ulBytesTransferred / 1000,
                           (ulBytesTransferred % 1000) / 10);
            }
            else if(ulBytesTransferred >= 100)
            {
                UARTprintf("%3d B/s    ", ulBytesTransferred);
            }
            else if(ulBytesTransferred >= 10)
            {
                UARTprintf("%2d B/s     ", ulBytesTransferred);
            }
            else
            {
                UARTprintf("%1d B/s      ", ulBytesTransferred);
            }

            //
            // Calculate how many UART transfers have occurred since the last
            // second.
            //
            ulXfersCompleted = (g_ulRxBufBCount + g_ulRxBufBCount -
                                ulPrevUARTCount);

            //
            // Remember the new UART transfer count.
            //
            ulPrevUARTCount = g_ulRxBufBCount;

            //
            // Compute how many bytes were transferred by the UART.  The number
            // of bytes received is multiplied by 2 so that the TX bytes
            // transferred are also accounted for.
            //
            ulBytesTransferred = ulXfersCompleted * SSI_RXBUF_SIZE * 2;

            //
            // Print a message showing the UART transfer rate.
            //
            if(ulBytesTransferred >= 1000000)
            {
                UARTprintf("%1d.%02d MB/s  ", ulBytesTransferred / 1000000,
                           (ulBytesTransferred % 1000000) / 10000);
            }
            else if(ulBytesTransferred >= 100000)
            {
                UARTprintf("%3d KB/s   ", ulBytesTransferred / 1000);
            }
            else if(ulBytesTransferred >= 10000)
            {
                UARTprintf("%2d.%01d KB/s  ", ulBytesTransferred / 1000,
                           (ulBytesTransferred % 1000) / 100);
            }
            else if(ulBytesTransferred >= 1000)
            {
                UARTprintf("%1d.%02d KB/s  ", ulBytesTransferred / 1000,
                           (ulBytesTransferred % 1000) / 10);
            }
            else if(ulBytesTransferred >= 100)
            {
                UARTprintf("%3d B/s    ", ulBytesTransferred);
            }
            else if(ulBytesTransferred >= 10)
            {
                UARTprintf("%2d B/s     ", ulBytesTransferred);
            }
            else
            {
                UARTprintf("%1d B/s      ", ulBytesTransferred);
            }

            //
            // Print a spinning line to make it more apparent that there is
            // something happening.
            //
            UARTprintf("%2ds", TEST_TIME - ulPrevSeconds);
            
            //
            // Turn off the LED.
            //
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
        }

        //
        // Put the processor to sleep if there is nothing to do.  This allows
        // the CPU usage routine to measure the number of free CPU cycles.
        // If the processor is sleeping a lot, it can be hard to connect to
        // the target with the debugger.
        //
        ROM_SysCtlSleep();

        //
        // See if we have run long enough and exit the loop if so.
        //
        if(g_ulSeconds >= TEST_TIME)
        {
            break;
        }
    }

    //
    // Indicate on the display that the example is stopped.
    //
    UARTprintf("\nStopped\n");
    
    //Blank Display
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4);

    //
    // Loop forever with the CPU not sleeping, so the debugger can connect.
    //
    while(1)
    {
        //
        // Turn on the GREEN LED.
        //
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);

        //
        // Delay for a bit.
        //
        SysCtlDelay(SysCtlClockGet() / 20 / 3);

        //
        // Turn off the GREEN LED.
        //
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
        
        //
        // Delay for a bit.
        //
        SysCtlDelay(SysCtlClockGet() / 20 / 3);

    }
}
