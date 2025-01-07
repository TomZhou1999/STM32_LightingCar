#include "stm32f10x.h"                  // Device header
#include "SPI.h"

void W25Qx_Init()
{
	MySPI_Init();
}


void W25Qx_ReadID(uint16_t * MID,uint16_t * DID)
{
	MySpi_Start();
	MySpi_SwapBytpe(0x9f);		//ID
	*MID = MySpi_SwapBytpe(0xff);
	*DID = MySpi_SwapBytpe(0xff);
	*DID = *DID << 8;
	*DID |= MySpi_SwapBytpe(0xff);
	MySpi_Stop();
}

void W25Qx_WriteEnable(void)
{
	MySpi_Start();
	MySpi_SwapBytpe(0x06);		//W25Qx_WriteEnable
	MySpi_Stop();
}

void W25Qx_WaitBusy()
{
	uint32_t Timeout;
	MySpi_Start();
	MySpi_SwapBytpe(0x05);		//W25Q64_READ_STATUS_REGISTER_1
	while((MySpi_SwapBytpe(0xff)&0x01)==0x01)
	{
		Timeout--;
		if(Timeout==0)
		{
			break;
		}
	}
	MySpi_Stop();
}

void W25Qx_PageProgram(uint32_t Address,uint8_t *DataArray,uint16_t Count)
{
	W25Qx_WriteEnable();
	uint16_t i;
	MySpi_Start();
	MySpi_SwapBytpe(0x02);	//W25Q64_PAGE_PROGRAM
	MySpi_SwapBytpe(Address>>16);
	MySpi_SwapBytpe(Address>>8);
	MySpi_SwapBytpe(Address);
	for(i=0;i<Count;i++)
	{
		MySpi_SwapBytpe(DataArray[i]);
	}
	MySpi_Stop();
	W25Qx_WaitBusy();
}

void W25Qx_SectorErase(uint32_t Address)
{
	W25Qx_WriteEnable();
	MySpi_Start();
	MySpi_SwapBytpe(0x20);	//W25Q64_SECTOR_ERASE_4KB
	MySpi_SwapBytpe(Address>>16);
	MySpi_SwapBytpe(Address>>8);
	MySpi_SwapBytpe(Address);
	MySpi_Stop();
	W25Qx_WaitBusy();
}

void W25Qx_ReadData(uint32_t Address,uint8_t *DataArray,uint16_t Count)
{
	uint16_t i;
	MySpi_Start();
	MySpi_SwapBytpe(0x03);	//W25Q64_READ_DATA
	MySpi_SwapBytpe(Address>>16);
	MySpi_SwapBytpe(Address>>8);
	MySpi_SwapBytpe(Address);
	for(i=0;i<Count;i++)
	{
		DataArray[i]=MySpi_SwapBytpe(0xff);
	}
	MySpi_Stop();
	
}
