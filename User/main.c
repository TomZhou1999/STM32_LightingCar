#include "stm32f10x.h"                  // Device header
#include "Power_Sequence.h"
#include "I2C.h"
#include "sensor_BF30A2.h"
#include "OLED.h"
#include "SPI.h"
#include "Delay.h"
#include "PWM.h"
#include "Motor.h"
#include "W25Qx.h"
uint8_t counter=0;
uint8_t SID,REG;
uint16_t MID,DID;
uint8_t ArrayWrite[]={0x01,0x02,0x03,0x04};
uint8_t ArrayRead[4]={0x01,0x02,0x03,0x04};
uint8_t ibuffer[240]={0x00};

int main(void)
{	
	OLED_Init();
	OLED_Clear();
	
	
	
	Sensor_BF30A2Init();
	SID = Sensor_GetID();
	OLED_ShowString(1,1,"ID:");
	OLED_ShowHexNum(1,4,SID,2);
	OLED_ShowString(2,1,"Y:");
	OLED_ShowString(2,6,"Speed:");
	OLED_ShowString(4,1,"F_");
	//Motor_Init();
	//Motor_SetSpeed(+90);
		
	W25Qx_Init();
	W25Qx_ReadID(&MID,&DID);
	OLED_ShowHexNum(1,7,MID,4);
	OLED_ShowHexNum(1,12,DID,4);
	//OLED_ShowString(3,1,"W:");
	//OLED_ShowString(4,1,"R:");
	W25Qx_SectorErase(0x000000);	//前3位是同一个扇区
	
	CmoSPi_Init();
	
	ImageData_Transfer();
//	MyDMA_Init((uint32_t)&SPI2->DR,(uint32_t)ibuffer,128);
//	MyDMA_Transfer();
	
	
	
////询问流转	
//	Cmos_SPI_W_SS(0);
//	for(counter=0;counter<128;counter++)
//	{
//		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);  // 等待接收完成
//		ibuffer[counter] = SPI_I2S_ReceiveData(SPI2);                 // 读取数据
//	}
//	counter=0;
	while (1)
	{
		
		ArrayWrite[counter%4] = MyI2C_ReadReg(0x88);
		OLED_ShowHexNum(2,3,ArrayWrite[counter%4],2);
		OLED_ShowNum(2,12,ArrayWrite[counter%4]/4+36,2);

		//		//Motor_SetSpeed(REG/4+36);
//		OLED_ShowHexNum(3,3,ArrayWrite[0],2);
//		OLED_ShowHexNum(3,6,ArrayWrite[1],2);
//		OLED_ShowHexNum(3,9,ArrayWrite[2],2);
//		OLED_ShowHexNum(3,12,ArrayWrite[3],2);
		
		if((counter+1)%10==0)
		{
//			W25Qx_SectorErase(0x000000);
//			W25Qx_PageProgram(0x000000,ArrayWrite,4);
//			W25Qx_ReadData(0x000000,ArrayRead,4);
//	
//			OLED_ShowHexNum(4,3,ArrayRead[0],2);
//			OLED_ShowHexNum(4,6,ArrayRead[1],2);
//			OLED_ShowHexNum(4,9,ArrayRead[2],2);
//			OLED_ShowHexNum(4,12,ArrayRead[3],2);
			
			
			//图像buffer
			OLED_ShowHexNum(3,1,ibuffer[counter],2);
			OLED_ShowHexNum(3,4,ibuffer[counter+1],2);
			OLED_ShowHexNum(3,7,ibuffer[counter+2],2);
			OLED_ShowHexNum(3,10,ibuffer[counter+3],2);
			OLED_ShowHexNum(3,13,ibuffer[counter+4],2);
			OLED_ShowHexNum(4,1,ibuffer[counter+5],2);
			OLED_ShowHexNum(4,4,ibuffer[counter+6],2);
			OLED_ShowHexNum(4,7,ibuffer[counter+7],2);
			OLED_ShowHexNum(4,10,ibuffer[counter+8],2);
			OLED_ShowHexNum(4,13,ibuffer[counter+9],2);
		}
		if(counter==255){counter=0;}
		else {counter++;}		
		
		
		
		Delay_ms(400);
	}
}
