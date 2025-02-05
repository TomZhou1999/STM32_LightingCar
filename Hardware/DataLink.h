#ifndef __DATALINK_H
#define __DATALINK_H

void ImageData_ClearFlag(void);
void Buffer_Swap(void);
uint16_t ImageData_Size(void);
//////////////////////////////////////////////////////////////
/////////////////////// Data_Link Layer //////////////////////
//////////////////////////////////////////////////////////////

void ImageData_ClearFlag(void);
void ImageData_Transfer(void);
uint8_t ImageData_ReadBuffer(void);
#endif
