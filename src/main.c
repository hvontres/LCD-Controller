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
#include "commands.h"







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
    static unsigned long ulLoop=0;
    static unsigned char On =0;
    static long lCommandStatus=0;
    const char *test="01234567890123456789A1234567890123456789B1234567890123456789C1234567890123456789D";
//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
    static char g_cInput[APP_INPUT_BUF_SIZE];
        
    
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
 // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPULazyStackingEnable();
    
    //Initialize Buffer Pointers
    g_uiSsiTxBufBaseA=(unsigned short *)(g_pucOffscreenBufA+6);
    g_uiSsiTxBufBaseB=(unsigned short *)(g_pucOffscreenBufB+6);
    g_uiSsiTxBufBase=g_uiSsiTxBufBaseA;

    //
    // Set the clocking to run from the PLL at 80 MHz.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |SYSCTL_XTAL_16MHZ);
    //ROM_SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

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
    //Initialize the Display Buffers - Make sure the Display data starts at an even Address -- need to figure out better way to do this
    GrOffScreen1BPPInit(&g_sOffscreenDisplayA, g_pucOffscreenBufA+1, 320, 240);
    UARTprintf("\n Display A Buffer: %x \n",(g_sOffscreenDisplayA.pvDisplayData+5));
    GrOffScreen1BPPInit(&g_sOffscreenDisplayB, g_pucOffscreenBufB+1, 320, 240);
    UARTprintf("\n Display B Buffer: %x \n",(g_sOffscreenDisplayB.pvDisplayData+5));
    
    // set up Buffer A as current Context
    GrContextInit(&sDisplayContext, &g_sOffscreenDisplayA);
    // Set colors for display - White = on, Black = off
    GrContextForegroundSet(&sDisplayContext,ClrWhite);
    GrContextBackgroundSet(&sDisplayContext,ClrBlack);
    GrContextFontSet(&sDisplayContext,&g_sFontCm12b);
    

    
    
    //Draw Splash Image
    //const unsigned char *pucSplashLogo = g_pucSplash;
    //GrImageDraw(&sDisplayContext, g_pucSplash,0,0);
    for (int line=0;line<24;line++) {
      
    GrStringDraw(&sDisplayContext,test,62,0,line*10,1);
    
    }
    
    // set up Buffer B as current Context
    SwitchBuffers();
    GrContextFontSet(&sDisplayContext,&g_sFontFixed6x8);
    for (int line=0;line<30;line++) {
      
    GrStringDraw(&sDisplayContext,test,53,0,line*8,1);
    
    }
    SwitchBuffers();

    
    
    //Set BLANK HIGH and reset MSP430
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4);
    
    //
    // Initialize the uDMA SSI transfers.
    //
    InitSSI2Transfer();
    

    // Release Blank Pin
    SysCtlDelay(SysCtlClockGet() / 20 / 3);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0);
    
    On=1;
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
    
    UARTprintf("Rx Buffer Size: %i",UARTRxBytesAvail());


    while(1)
    {
        //
        // Check to see if one second has elapsed.  If so, the make some
        // updates.
        //
        
        UARTprintf("\n>");

        
        //
        // Peek to see if a full command is ready for processing
        //
        while(UARTPeek('\r') == -1)
        {
            //
            // millisecond delay.  A SysCtlSleep() here would also be OK.
            //
            SysCtlDelay(SysCtlClockGet() / (1000 / 3));
            
        }
        
        //
        // a '\r' was detected get the line of text from the user.
        //
        UARTgets(g_cInput,sizeof(g_cInput));

        //
        // Pass the line from the user to the command processor.
        // It will be parsed and valid commands executed.
        //
        lCommandStatus = CmdLineProcess(g_cInput);

        //
        // Handle the case of bad command.
        //
        if(lCommandStatus == CMDLINE_BAD_CMD)
        {
            UARTprintf("Bad command!\n");
        }

        //
        // Handle the case of too many arguments.
        //
        else if(lCommandStatus == CMDLINE_TOO_MANY_ARGS)
        {
            UARTprintf("Too many arguments for command processor!\n");
        }
        
        
        
//         if(g_ulSeconds != ulPrevSeconds)
//         {
//             //
//             // Turn on the LED as a heartbeat
//             //
//             GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
//             
//            
//             
//             //
//             // Turn off the LED.
//             //
//             GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
//         }
// 
//         //
//         // Put the processor to sleep if there is nothing to do.  This allows
//         // the CPU usage routine to measure the number of free CPU cycles.
//         // If the processor is sleeping a lot, it can be hard to connect to
//         // the target with the debugger.
//         //
//         ROM_SysCtlSleep();

        //
        // Check Exit Flag and Exit if set.
        //
        if(g_ucExit)
        {
	 break;
        }
    }

    //
    // Indicate on the display that the example is stopped.
    //
    UARTprintf("\nStopped\n");
    DisableSSI2Transfer();
    //Blank Display
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);

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
