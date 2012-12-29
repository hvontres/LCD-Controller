

#define SSI_TXBUF_SIZE         800
#define SSI_RXBUF_SIZE         800

#define UDMA_CHANNEL_SSI2RX    12
#define UDMA_CHANNEL_SSI2TX    13



void SSI2IntHandler(void);
void InitSSI2Transfer(void);
void DisableSSI2Transfer(void);
void SwitchBuffers(void);


static unsigned long g_ulBufferSegCount = 0;