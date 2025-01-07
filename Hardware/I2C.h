#ifndef __I2C_H__
#define __I2C_H__

void MyI2C_Dev_ADDRESS(uint8_t ADDRESS);
void MyI2C_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);
void MyI2C_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MyI2C_ReadReg(uint8_t RegAddress);
void MyI2C_Init(void);
uint8_t MyI2C_GetID(void);
void Cmos_SPI_W_SS(uint8_t BitValue);

#endif
