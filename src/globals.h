


//*****************************************************************************
//
// The transmit and receive buffers used for the SSI transfers.  There is one
// transmit buffer and a pair of recieve ping-pong buffers.
//
//*****************************************************************************


extern unsigned short *g_uiSsiRxBuf;
extern unsigned short *g_uiSsiTxBufBase;
extern unsigned short *g_uiSsiTxBufBaseA;
extern unsigned short *g_uiSsiTxBufBaseB;
extern unsigned short *g_uiSsiTxBufB;
extern char g_ucScreenText[30][54];
extern unsigned long g_ulRxBufBCount;
extern unsigned char g_ucExit;
//extern unsigned char *g_pucSplash;

// Display buffers
extern tDisplay g_sOffscreenDisplayA;
extern tDisplay g_sOffscreenDisplayB;
// Acitive Display Context
extern tContext sDisplayContext;