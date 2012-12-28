


//*****************************************************************************
//
// The transmit and receive buffers used for the UART transfers.  There is one
// transmit buffer and a pair of recieve ping-pong buffers.
//
//*****************************************************************************


extern unsigned short g_uiSsiRxBuf[SSI_RXBUF_SIZE];
extern unsigned short *g_uiSsiTxBufBase;//=g_uiPixelData[0];
extern unsigned short *g_uiSsiTxBufB;//=g_uiPixelData[SSI_TXBUF_SIZE];
extern unsigned long g_ulRxBufBCount;