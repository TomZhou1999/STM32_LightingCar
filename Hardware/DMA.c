#include "stm32f10x.h"                  // Device header


uint16_t MyDMA_Size = 128;					//定义全局变量，用于记住Init函数的Size，供Transfer函数使用

//////////////////////////////////////////////////////////////
/////////////////////// physical Layer ///////////////////////
//////////////////////////////////////////////////////////////

/**
  * 函    数：DMA初始化
  * 参    数：AddrA 原数组的首地址
  * 参    数：AddrB 目的数组的首地址
  * 参    数：Size 转运的数据大小（转运次数）
  * 返 回 值：无
  */
void MyDMA_Init(uint32_t AddrA, uint32_t AddrB, uint16_t Size)
{
	MyDMA_Size = Size;					//将Size写入到全局变量，记住参数Size
	
	/*开启时钟*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						//开启DMA的时钟
	DMA_DeInit(DMA1_Channel4); 												// SPI2_RX 对应 DMA1 通道 4
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;										//定义结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = AddrA;						//外设基地址，给定形参AddrA
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据宽度，选择字节
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址自增，选择使能
	DMA_InitStructure.DMA_MemoryBaseAddr = AddrB;							//存储器基地址，给定形参AddrB
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据宽度，选择字节
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器地址自增，选择使能
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						//数据传输方向，选择由外设到存储器
	DMA_InitStructure.DMA_BufferSize = Size;								//转运的数据大小（转运次数）
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//模式，选择正常模式
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//存储器到存储器，选择使能
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					//优先级，选择中等
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);							//将结构体变量交给DMA_Init，配置DMA1的通道1
	
	//DMA1_Channel4->CMAR = AddrB;
	
	//传输完成后启动中断
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 配置抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         // 配置子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            // 启用中断
    NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	
	
	/*DMA使能*/
	DMA_Cmd(DMA1_Channel4, DISABLE);	//这里先不给使能，初始化后不会立刻工作，等后续调用Transfer后，再开始
}

/**
  * 函    数：DMA转运目标变化
  * 参    数：转运目标地址
  * 返 回 值：无
  */

void DMA_Switch_Buffer(uint32_t Buffer)
{
	DMA1_Channel4->CMAR = Buffer;
}


//////////////////////////////////////////////////////////////
/////////////////////// Data_Link Layer //////////////////////
//////////////////////////////////////////////////////////////

/**
  * 函    数：启动DMA数据转运
  * 参    数：无
  * 返 回 值：无
  */
void MyDMA_Transfer(void)
{
	DMA_Cmd(DMA1_Channel4, DISABLE);					//DMA失能，在写入传输计数器之前，需要DMA暂停工作
	DMA_SetCurrDataCounter(DMA1_Channel4, MyDMA_Size);	//写入传输计数器，指定将要转运的次数
	DMA_Cmd(DMA1_Channel4, ENABLE);						//DMA使能，开始工作
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE); 	// 启用 SPI 的 DMA 请求
	while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);	//等待DMA工作完成
	DMA_ClearFlag(DMA1_FLAG_TC4);						//清除工作完成标志位
	DMA_Cmd(DMA1_Channel4, DISABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, DISABLE);
}
