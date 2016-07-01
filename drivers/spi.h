#ifndef _SPI_H
#define _SPI_H

void spiInit(void);
void spiIoShutdown(void);
u_char SPI_SendByteData(u_char ubData);
void SpiWriteDataBurst(uint8_t biDataInLength, uint8_t *pabiDataIn);
void SpiReadDataBurst(uint8_t biDataOutLength, uint8_t *paboDataOut);


#endif
