#ifndef __W25QX_H
#define __W25QX_H

void W25Qx_Init(void);
void W25Qx_ReadID(uint16_t * MID,uint16_t * DID);
void W25Qx_WriteEnable(void);
void W25Qx_WaitBusy(void);
void W25Qx_PageProgram(uint32_t Address,uint8_t *DataArray,uint16_t Count);
void W25Qx_SectorErase(uint32_t Address);
void W25Qx_ReadData(uint32_t Address,uint8_t *DataArray,uint16_t Count);

#endif
