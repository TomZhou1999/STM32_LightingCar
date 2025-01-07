#ifndef __SPI_H
#define __SPI_H

void MySPI_W_SS(uint8_t BitValue);
void MySPI_W_SCK(uint8_t BitValue);
void MySPI_W_MOSI(uint8_t BitValue);
uint8_t MySPI_R_MISO(void);
void MySPI_Init(void);
void MySpi_Start(void);
void MySpi_Stop(void);
uint8_t MySpi_SwapBytpe(uint8_t ByteSend);

void MyDMA_Init(uint32_t AddrA, uint32_t AddrB, uint16_t Size);
void MyDMA_Transfer(void);
void CmoSPi_Init(void);
uint8_t ReadBuffer(uint8_t i);
uint8_t Cmos_SPI_Rx(void);
void ImageData_Transfer(void);

#endif
