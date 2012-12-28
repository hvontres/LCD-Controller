
//*****************************************************************************
//
// The number of SysTick ticks per second used for the SysTick interrupt.
//
//*****************************************************************************
#define SYSTICKS_PER_SECOND     100

//*****************************************************************************
//
// The size of the memory transfer source and destination buffers (in words).
//
//*****************************************************************************
#define MEM_BUFFER_SIZE 256        

//*****************************************************************************
//
// The size of the UART transmit and receive buffers.  They do not need to be
// the same size.
//
//*****************************************************************************
#define UART_TXBUF_SIZE         256
#define UART_RXBUF_SIZE         256



#define TEST_TIME		120


//*****************************************************************************
//
// The source and destination buffers used for memory transfers.
//
//*****************************************************************************
static unsigned long g_ulSrcBuf[MEM_BUFFER_SIZE];
static unsigned long g_ulDstBuf[MEM_BUFFER_SIZE];




//*****************************************************************************
//
// The count of uDMA errors.  This value is incremented by the uDMA error
// handler.
//
//*****************************************************************************
static unsigned long g_uluDMAErrCount = 0;

//*****************************************************************************
//
// The count of times the uDMA interrupt occurred but the uDMA transfer was not
// complete.  This should remain 0.
//
//*****************************************************************************
static unsigned long g_ulBadISR = 0;

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


unsigned short g_uiSsiRxBuf[SSI_RXBUF_SIZE];
unsigned short *g_uiSsiTxBufBase;//=g_uiPixelData[0];
unsigned short *g_uiSsiTxBufB;//=g_uiPixelData[SSI_TXBUF_SIZE];