#include "stm32f10x.h"                  // Device header
#include "OLED.h"

uint16_t MyDMA_Size = 128;					//定义全局变量，用于记住Init函数的Size，供Transfer函数使用
uint8_t e_buffer[256] = {0x00};				//存储偶数行 256B每个
uint8_t o_buffer[256] = {0x00};				//存储奇数行
uint8_t o_flag = 0;							//奇数行传输标志位
uint8_t e_flag = 0;							//偶数行传输标志位

uint8_t sync_code = 0x00;
uint8_t sync_flag = 0;

uint32_t error_num = 0;
uint8_t code_order[9] = {0x00};
#define CODE_ORDER_SIZE 9

uint16_t ptr = 0;
uint8_t Image_flag = 0;			//传输图像标志
uint8_t V_flag = 0;				//帧标志
uint8_t H_flag = 0;				//行标志

uint32_t H_current = 0x00;		//当前行
uint32_t V_current = 0x00;		//当前帧
uint16_t Image_Width = 0x00;	//帧数据大小
uint16_t Image_Height = 0x00;	
uint16_t H_DataSize = 0x00;		//行数据大小
uint8_t V_DataID = 0x00;		//数据包格式
/**
  * 函    数：cmos spi通信信号线电平转换 NSS
  * 参    数：bitvalue
  * 返 回 值：无
  */
void Cmos_SPI_W_SS(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,(BitAction)BitValue);
}



/**
  * 函    数：信号线电平转换 SS SCK MOSI/读MISO
  * 参    数：bitvalue
  * 返 回 值：无
  */
void MySPI_W_SS(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_4,(BitAction)BitValue);
}



/**
  * 函    数：MySPI初始化，用于Flash和STM32之间
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
* 函    数：Cmos的SPI初始化，用于cmos和STM32之间,STM32为从机，单工，mode0
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




/**
  * 函    数：轮询RNXE位，把SPI_DR里面的数据取出
  * 参    数：无
  * 返 回 值：无
  */
uint8_t Cmos_SPI_Rx(void)
{
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)!=SET);
	return SPI_I2S_ReceiveData(SPI2);
}


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
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择使能
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					//优先级，选择中等
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);							//将结构体变量交给DMA_Init，配置DMA1的通道1
	/*DMA使能*/
	DMA_Cmd(DMA1_Channel4, DISABLE);	//这里先不给使能，初始化后不会立刻工作，等后续调用Transfer后，再开始
}



/**
  * 函    数：启动DMA数据转运
  * 参    数：无
  * 返 回 值：无
  */
void MyDMA_Transfer(void)
{
	Cmos_SPI_W_SS(1);
	DMA_Cmd(DMA1_Channel4, DISABLE);					//DMA失能，在写入传输计数器之前，需要DMA暂停工作
	DMA_SetCurrDataCounter(DMA1_Channel4, MyDMA_Size);	//写入传输计数器，指定将要转运的次数
	DMA_Cmd(DMA1_Channel4, ENABLE);						//DMA使能，开始工作
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE); 	// 启用 SPI 的 DMA 请求
	while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);	//等待DMA工作完成
	DMA_ClearFlag(DMA1_FLAG_TC4);						//清除工作完成标志位
	DMA_Cmd(DMA1_Channel4, DISABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, DISABLE);
	Cmos_SPI_W_SS(0);
}





/**
  * 函    数：轮询、DMA转运数据
  * 参    数：无
  * 返 回 值：无
  */
void ImageData_Transfer(void)
{
	Cmos_SPI_W_SS(0);				//片选信号
	V_flag=0;						//帧头标志位为0 1为找到帧头	
	H_flag = 0;							
	Image_flag = 0;
	sync_flag = 0;					//code
	ptr =0;							//循环计数
	
	while(Image_flag==0)
	{
		sync_code = Cmos_SPI_Rx();
		if(sync_code==0xff)				//第一个FF
		{
			sync_code = Cmos_SPI_Rx();
			if(sync_code==0xff)			//第二个ff
			{
				sync_code = Cmos_SPI_Rx();
				if(sync_code==0xff)		//第三个FF
				{
					sync_code = Cmos_SPI_Rx();
					switch(sync_code)
					{
						case 0x01://Frame Start
							V_flag = 1;
							V_DataID = Cmos_SPI_Rx();
							Image_Width = Cmos_SPI_Rx();
							Image_Width |= (uint32_t)Cmos_SPI_Rx()<<8;
							Image_Height = Cmos_SPI_Rx();
							Image_Height |= (uint32_t)Cmos_SPI_Rx()<<8;
							break;
						case 0x40://H Start
							H_flag = 1;
							H_DataSize = Cmos_SPI_Rx();
							H_DataSize |= (uint32_t)Cmos_SPI_Rx()<<8;
							//DMA转运当前行
							for(ptr=0;ptr<H_DataSize;ptr++)
									{
										e_buffer[ptr]=Cmos_SPI_Rx();
									}
									e_flag =1;
							break;
						case 0x02://当前行数							
							H_current = Cmos_SPI_Rx();
							H_current |= (uint32_t)Cmos_SPI_Rx()<<8;						
							break;
						case 0x00://结束
							
							//OLED_ShowHexNum(3,7,H_current,4);
							break;
						default:
							break;
					}
					
				}
			}
				
		}
		if(V_flag==1)
		{
			V_current++;
			OLED_ShowNum(4,3,V_current,4);
			OLED_ShowHexNum(3,1,V_DataID,2);
			V_flag=0;
			//OLED_ShowNum(4,8,Image_Height,4);
			//OLED_ShowNum(3,8,Image_Width,4);
		}
		
		if(H_flag ==1)
		{
//			//转运当前行
//			if(e_flag ==1)	//当前偶行
//			{
//			OLED_ShowHexNum(1,1,e_buffer[0],2);
//			OLED_ShowHexNum(1,4,e_buffer[1],2);
//			OLED_ShowHexNum(1,7,e_buffer[2],2);
//			OLED_ShowHexNum(1,10,e_buffer[3],2);
//			OLED_ShowHexNum(1,13,e_buffer[4],2);
//			OLED_ShowHexNum(2,1,e_buffer[5],2);
//			OLED_ShowHexNum(2,4,e_buffer[6],2);
//			OLED_ShowHexNum(2,7,e_buffer[7],2);
//			OLED_ShowHexNum(2,10,e_buffer[8],2);
//			OLED_ShowHexNum(2,13,e_buffer[9],2);
//				//转运奇行
//				e_flag =0;
//			}else if(o_flag ==1)
//			{								//当前奇行
//				//转运偶行
//				o_flag =0;
//			}
			//OLED_ShowHexNum(4,7,V_flag,2);
			OLED_ShowHexNum(3,7,H_DataSize,4);
			H_flag =0;
		}
	
	}
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
