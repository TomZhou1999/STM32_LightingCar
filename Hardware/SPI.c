#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "W25Qx.h"



//////////////////////////////////////////////////////////////
/////////////////////// physical Layer ///////////////////////
//////////////////////////////////////////////////////////////

/**
  * 函    数：激活SPI2、PB12写1 [NSS]
  * 参    数：bitvalue
  * 返 回 值：无
  */
void Cmos_SPI_W_SS(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,(BitAction)BitValue);
}



/**
  * 函    数：激活SPI1、PA4写0 [NSS]
  * 参    数：bitvalue
  * 返 回 值：无
  */
void MySPI_W_SS(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_4,(BitAction)BitValue);
}


/**
  * 函    数：SPI1初始化,[STM32主机][FLASH从机][全双工][mode0]
  * 参    数：无
  * 返 回 值：无
  */
void MySPI_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);			//开启SPI1时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//上拉
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽，交给片上外设
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
	
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						//主机
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;	//全双工
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//8bit
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;					//高位在前
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_128;	//72M/128
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//空闲低电平
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//第一边缘采样(移入)
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//多主机控制
	SPI_InitStructure.SPI_CRCPolynomial=7;								//校验？
	SPI_Init(SPI1,&SPI_InitStructure);
	SPI_Cmd(SPI1,ENABLE);
	
	MySPI_W_SS(1);	
}









/**
* 函    数：SPI2初始化，[STM32主机][CMOS主机][单工][mode0]
  * 参    数：无
  * 返 回 值：无
  */
void CmoSPi_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);			//开启SPI2时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				//NSS PB12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//时钟浮空输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;				//MOSI 主机输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			//时钟浮空输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;						//SCL
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
	
	//SPI配置
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;						//从机
	SPI_InitStructure.SPI_Direction=SPI_Direction_1Line_Rx;				//单工接收
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//8bit
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_LSB;					//低位在前
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_256;	//72M/128
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//空闲低电平
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//第一边缘采样(移入)
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//多主机控制
	SPI_InitStructure.SPI_CRCPolynomial=7;								//校验？
	SPI_Init(SPI2,&SPI_InitStructure);
	SPI_Cmd(SPI2,ENABLE);
	//MyDMA_Init((uint32_t)buffer, (uint32_t)&SPI1->DR,MyDMA_Size);
		
}











//////////////////////////////////////////////////////////////
/////////////////////// Data_Link Layer //////////////////////
//////////////////////////////////////////////////////////////

/**
  * 函    数：等待RNXE位，把SPI_DR里面的数据取出
  * 参    数：无
  * 返 回 值：uint8_t SPI_DR
  */
uint8_t Cmos_SPI_Rx(void)
{
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)!=SET);
	return SPI_I2S_ReceiveData(SPI2);
}





/**
* 函    数：读数据缓冲器
  * 参    数：无
  * 返 回 值：无
  */

void MySpi_Start(void)
{
	MySPI_W_SS(0);
}


void MySpi_Stop(void)
{
	MySPI_W_SS(1);
}

uint8_t MySpi_SwapBytpe(uint8_t ByteSend)
{	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)!=SET);
	SPI_I2S_SendData(SPI1,ByteSend);
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)!=SET);
	return SPI_I2S_ReceiveData(SPI1);
}
