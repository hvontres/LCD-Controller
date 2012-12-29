
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
// Define two 1BPP offscreen buffers and display structures.  ]
//
//*****************************************************************************
#define OFFSCREEN_BUF_SIZE GrOffScreen1BPPSize(320, 240)
unsigned char g_pucOffscreenBufA[OFFSCREEN_BUF_SIZE+1];
unsigned char g_pucOffscreenBufB[OFFSCREEN_BUF_SIZE+1];
tDisplay g_sOffscreenDisplayA;
tDisplay g_sOffscreenDisplayB;
// Context for Active Display
tContext sDisplayContext;


