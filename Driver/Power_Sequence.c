#include "stm32f10x.h"                  // Device header
#include "Power_Sequence.h"


/**
  *@brief 置1使用PWM输出时钟 置0用MCO
  */
#define PWM 1	

/**
  *@brief 修改XCLK时钟频率,单位MHZ 36的因子生效
  *@ref Sensor_Pin_MCLK
  *@return 无
  */
void XCLKSetFrquence(uint8_t frq)
{
	TIM_PrescalerConfig(TIM1, (int)(36/frq-1), TIM_PSCReloadMode_Immediate);
}



/**
  *@brief XCLK时钟初始化,默认时钟是6Mhz
  *@ref Sensor_Pin_MCLK
  *@return 无
  */
void XCLKInit(void)
{	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
		
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = Sensor_Pin_MCLK;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;				//驱动能力
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//将PA0引脚初始化为复用推挽输出	
																	//受外设控制的引脚，均需要配置为复用模式		
	if (PWM==1){
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 2 - 1;					//计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 6 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元
	
	/*输出比较初始化*/
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);							//结构体初始化，若结构体没有完整赋值
																	//则最好执行此函数，给结构体所有成员都赋一个默认值
																	//避免结构体初值不确定的问题
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//输出极性，选择为高，若选择极性为低，则输出高低电平取反
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;								//初始的CCR值
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);						//将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM2，定时器开始运行
	TIM_SetCompare1(TIM1, 1);		//设置CCR1的值
	TIM_CtrlPWMOutputs(TIM1, ENABLE); // 启用高级定时器 PWM 输出
	}
	else
	{
		RCC_MCOConfig(RCC_MCO_HSI);	//只能输出8M
	}
	
}











/**
  *@brief PWDN初始化
  *@ref Sensor_Pin_PWD
  *@return 无
  */
void PWDInit(void)
{
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = Sensor_Pin_PWD;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;				//驱动能力
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//将PA0引脚初始化为复用推挽输出																		//受外设控制的引脚，均需要配置为复用模式		
	GPIO_ResetBits(GPIOA, Sensor_Pin_PWD);								//设置PWDN引脚为高电平
}


/**
  *@brief PWDN控制
  *@ref Sensor_Pin_PWD
  *@return 无
  */
void PWDSet(uint8_t state)
{
	if(state == 0)
	{
		GPIO_ResetBits(GPIOA, Sensor_Pin_PWD);
	}else
	{
		GPIO_SetBits(GPIOA, Sensor_Pin_PWD);
	}

}


/**
  *@brief I2C初始化
  *@ref 无
  *@return 无
  */

/**
  *@brief 上电时序
  *@ref 无
  *@return 无
  */
void SensorPowerON(void)
{
	XCLKInit();
	PWDInit();
	PWDSet(0);
}
