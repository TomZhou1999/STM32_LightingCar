#include "stm32f10x.h"                  // Device header
#include "SPI.h"
#include "DMA.h"
#include "I2C.h"
#include "OLED.h"

uint8_t e_buffer[6826] = {0x00};				//存储偶数行 256B每个		//分辨率降到128*48 6726B
uint8_t o_buffer[6826] = {0x00};				//存储奇数行

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
uint16_t DataSize = 0x00;		//行数据大小
uint8_t V_DataID = 0x00;		//数据包格式

/**
  * 函    数：初始化标志位
  * 参    数：无
  * 返 回 值：无
  */
void ImageData_ClearFlag(void)
{
	//重置标志位
	V_flag=0;						//帧头标志位为0 1为找到帧头	
	H_flag = 0;							
	Image_flag = 0;
	sync_flag = 0;					//code
	ptr =0;							//循环计数
}



/**
  * 函    数：双buffer的转换
  * 参    数：无
  * 返 回 值：无
  */
void Buffer_Swap(void)
{
	if(V_current%2==0)		//偶数行为even
	{
		DMA1_Channel4->CMAR = (uint32_t)e_buffer;
	}else					//奇数行为odd
	{
		DMA1_Channel4->CMAR = (uint32_t)o_buffer;
	}
	
}

/**
  * 函    数：数据大小
  * 参    数：无
  * 返 回 值：DataSize
  */
uint16_t ImageData_Size(void)
{
	return DataSize;
}

//////////////////////////////////////////////////////////////
/////////////////////// Data_Link Layer //////////////////////
//////////////////////////////////////////////////////////////

/**
  * 函    数：预览画面实现（图形数据从cmos到stm32(DMA1 通道4),再从stm32到显示器的数据流转）负责开头的帧头识别，后续交给中断函数
  * 参    数：无
  * 返 回 值：无
  */
void ImageData_Transfer(void)
{
	CmoSPi_Init();														//初始化SPI2
	Cmos_SPI_W_SS(0);													//激活SPI2 片选
	ImageData_ClearFlag();												//重置标志位
	MyDMA_Init((uint32_t)&SPI2->DR,(uint32_t)e_buffer,256);				//初始化DMA			开始的时候放在偶数行			
	//识别帧头、分发DMA任务、break退出	
	while(1)
	{
		//识别第一个code
		//读到帧头后，读取长宽 DMA转运(W+6+6)*H+6>20KB
		sync_code = Cmos_SPI_Rx();
		if(sync_code==0xff)				//第一个FF
		{
			sync_code = Cmos_SPI_Rx();
			if(sync_code==0xff)			//第二个FF
			{
				sync_code = Cmos_SPI_Rx();
				if(sync_code==0xff)		//第三个FF
				{
					sync_code = Cmos_SPI_Rx();
					if(sync_code==0x01)	//识别到帧头code后的处理code:01
					{
							//Frame Start
							//识别帧数据量
							//OLED_ShowString(4,1,"Frame Start");
							V_flag = 1;
							V_DataID = Cmos_SPI_Rx();
							Image_Width = Cmos_SPI_Rx();
							Image_Width |= (uint32_t)Cmos_SPI_Rx()<<8;
							Image_Height = Cmos_SPI_Rx();
							Image_Height |= (uint32_t)Cmos_SPI_Rx()<<8;
							DataSize = (Image_Width+12)*Image_Height+6;
							
							//DMA转运当前帧
							DMA_SetCurrDataCounter(DMA1_Channel4, DataSize);		//数据转运次数为datasize
							DMA_Cmd(DMA1_Channel4, ENABLE);															
							SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
							//OLED_ShowString(4,1,"DMA Start");
							V_current++;											//帧计数+1
							break;													//退出循环
					}
					
				}
			}	
		}
		
	}
	





	//数据处理
}


/**
  * 函    数：读缓冲器
  * 参    数：无
  * 返 回 值：无
  */
uint8_t ImageData_ReadBuffer(void)
{
	if(V_current%2==0)		//偶数行为even
	{
		OLED_ShowHexNum(4,1,o_buffer[3],2);
		OLED_ShowHexNum(4,3,o_buffer[4],2);
		OLED_ShowHexNum(4,5,o_buffer[5],2);
		OLED_ShowHexNum(4,7,o_buffer[6],2);
		OLED_ShowHexNum(4,9,o_buffer[7],2);
//		OLED_ShowHexNum(4,11,o_buffer[5],2);
		
		//OLED_ShowHexNum(4,11,o_buffer[0x1aa5],2);
		//OLED_ShowHexNum(4,13,o_buffer[0x1aa6],2);
		return o_buffer[0x1a05];
	}else					//奇数行为odd
	{
		OLED_ShowHexNum(3,1,e_buffer[3],2);
		OLED_ShowHexNum(3,3,e_buffer[4],2);
		OLED_ShowHexNum(3,5,e_buffer[5],2);
		OLED_ShowHexNum(3,7,e_buffer[6],2);
		OLED_ShowHexNum(3,9,e_buffer[7],2);
//		OLED_ShowHexNum(3,11,e_buffer[5],2);
		//OLED_ShowHexNum(3,11,e_buffer[0x1aa5],2);
		//OLED_ShowHexNum(3,13,e_buffer[0x1aa6],2);
		return e_buffer[0x1a05];
	}
}


/**
  * 函    数：中断，识别下一帧和分发DMA任务
  * 参    数：无
  * 返 回 值：无
  */
void DMA1_Channel4_IRQHandler(void)
{
    
	//OLED_ShowString(4,7,"IRQHandler");
	if (DMA_GetITStatus(DMA1_IT_TC4))  				// 检查传输完成中断
    {
        DMA_ClearITPendingBit(DMA1_IT_TC4);  		// 清除中断标志
		
		// 重新分发任务（普通模式下）
        DMA_Cmd(DMA1_Channel4, DISABLE);            // 禁用 DMA
		Buffer_Swap();								//变换缓冲区
	while(1)
	{
		//识别第一个code
		//读到帧头后，读取长宽 DMA转运(W+6+6)*H+6>20KB
		sync_code = Cmos_SPI_Rx();
		if(sync_code==0xff)				//第一个FF
		{
			sync_code = Cmos_SPI_Rx();
			if(sync_code==0xff)			//第二个FF
			{
				sync_code = Cmos_SPI_Rx();
				if(sync_code==0xff)		//第三个FF
				{
					sync_code = Cmos_SPI_Rx();
					if(sync_code==0x02)
					{break;}
					
				}
			}
		}
	}
	
	if(sync_code==0x02)
	{
	//识别到帧头code后的处理code:02
	H_current =	Cmos_SPI_Rx();
	H_current |= (uint32_t)Cmos_SPI_Rx()<<8;	
	DataSize  = Image_Width+6+(Image_Width+12)*(Image_Height-H_current-1)+6;
	//DataSize = 0x1A40;
	
	//DMA转运当前帧						
	DMA_SetCurrDataCounter(DMA1_Channel4, DataSize);		//数据转运次数为datasize						
	DMA_Cmd(DMA1_Channel4, ENABLE);																					
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
	V_current++;					
	OLED_ShowHexNum(1,13,V_current,4);
	OLED_ShowHexNum(3,13,H_current,4);
	OLED_ShowHexNum(4,13,DataSize,4);
	}
	
//	e_buffer[0]	=Cmos_SPI_Rx();
//	e_buffer[1]	=Cmos_SPI_Rx();
//	e_buffer[2]	=Cmos_SPI_Rx();
//	e_buffer[3]	=Cmos_SPI_Rx();
//	e_buffer[4]	=Cmos_SPI_Rx();
//	e_buffer[5]	=Cmos_SPI_Rx();
//	e_buffer[7]	=Cmos_SPI_Rx();
//	e_buffer[8]	=Cmos_SPI_Rx();
//	e_buffer[9]	=Cmos_SPI_Rx();
//	e_buffer[10]=Cmos_SPI_Rx();
//	e_buffer[11]=Cmos_SPI_Rx();
//	e_buffer[12]=Cmos_SPI_Rx();
//	e_buffer[13]=Cmos_SPI_Rx();
//	
//		OLED_ShowHexNum(3,1,e_buffer[0],2);
//		OLED_ShowHexNum(3,3,e_buffer[1],2);
//		OLED_ShowHexNum(3,5,e_buffer[2],2);
//		OLED_ShowHexNum(3,7,e_buffer[3],2);
//		OLED_ShowHexNum(3,9,e_buffer[4],2);
//		OLED_ShowHexNum(3,11,e_buffer[5],2);		
//		OLED_ShowHexNum(4,1, e_buffer[7],2);
//		OLED_ShowHexNum(4,3, e_buffer[8],2);
//		OLED_ShowHexNum(4,5, e_buffer[9],2);
//		OLED_ShowHexNum(4,7, e_buffer[10],2);
//		OLED_ShowHexNum(4,9, e_buffer[11],2);
//		OLED_ShowHexNum(4,11,e_buffer[12],2);
//		OLED_ShowHexNum(4,13,e_buffer[13],2); 
					
	}
	
}


//while(sync_code==0xff)
//		{
//			sync_code = Cmos_SPI_Rx();
//		}
//		OLED_ShowHexNum(3,3,sync_code,2);
//					if(sync_code==0x01)
//					{
//							//Frame Start							
//							//OLED_ShowString(4,5,"Frame Start");
//							V_flag = 1;
//							V_DataID = Cmos_SPI_Rx();
//							Image_Width = Cmos_SPI_Rx();
//							Image_Width |= (uint32_t)Cmos_SPI_Rx()<<8;
//							Image_Height = Cmos_SPI_Rx();
//							Image_Height |= (uint32_t)Cmos_SPI_Rx()<<8;
//							//DataSize = (Image_Width+12)*Image_Height+6;
//							OLED_ShowHexNum(4,5,Image_Width,4);
//							OLED_ShowHexNum(4,10,Image_Height,4);
//							//DMA转运当前帧
//							DMA_SetCurrDataCounter(DMA1_Channel4, DataSize);		//数据转运次数为datasize
//							DMA_Cmd(DMA1_Channel4, ENABLE);															
//							SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
//							
//							V_current++;
//							//OLED_ShowHexNum(4,13,V_current,2);
//							//OLED_ShowHexNum(3,7,DataSize,4);
//							
//							OLED_ShowString(3,13,"!1");
//							
//					}else if(sync_code==0x02)
//					{						
//							//H Count 如果接收不到头，就看看行头							
//							//OLED_ShowString(4,5,"H_");
//							H_current = Cmos_SPI_Rx();
//							H_current |= (uint32_t)Cmos_SPI_Rx()<<8;
//							OLED_ShowString(4,13,"!02");
//							//DataSize = Image_Width+(Image_Width+12)*(Image_Height-H_current-1)+6;
//							//DMA转运当前帧
//							DMA_SetCurrDataCounter(DMA1_Channel4, DataSize);		//数据转运次数为datasize
//							DMA_Cmd(DMA1_Channel4, ENABLE);															
//							SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
//							
//					}
//					
//		




