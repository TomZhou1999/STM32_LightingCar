# STM32LightingCar

 ## 1.层级的任务

**物理层（Physical Layer）：**

- 主要关注硬件信号和引脚配置。
- 面向的是硬件资源（如 GPIO 和时钟信号）。
- **关键特征：**只定义信号的传输方式，不关注信号传输的内容。

* 外设 + 协议配置：cmos sensor（供电、时钟、上电时序、I2C配置、SPI配置）、tft屏幕



**链路层（Data Link Layer）：**

- 主要关注通过协议传递数据（如 I2C、SPI）。
- 面向的是数据帧的组织和可靠传输。
- **关键特征：**实现数据帧的传输，包括起始信号、地址、数据和停止信号。

* 外设 + IO数据处理：cmos sensor（I2C读写、SPI接收数据）、tft屏幕、以及数据在外设之间的流转



**设备驱动 Driver**（外设）

* 外设 +功能的实现



面对的外设对象分类，主要有马达、CMOS SENSOR、屏幕

## 2.CMOS SENSOR

### **物理层（Physical Layer）：**

#### 输入时钟 XCLK 



* 使用 STM32 内部的 8 MHz RC 振荡器（HSI）作为时钟源，并将它输出到 GPIO 引脚（例如 MCO 引脚）。

* 定时器TIM1中使用OC输出比较PWM输出想要的频率。



有一个GPIO能实现两种方式切换

<img src="C:\Users\TOM\AppData\Roaming\Typora\typora-user-images\image-20250124104634919.png" alt="image-20250124104634919" style="zoom:50%;" />



1️⃣MCO方法的数信号通道

<img src="C:\Users\TOM\AppData\Roaming\Typora\typora-user-images\image-20250124105200339.png" alt="image-20250124105200339" style="zoom:33%;" />



2️⃣PWM方法的数据通道

<img src="C:\Users\TOM\AppData\Roaming\Typora\typora-user-images\image-20250124105511325.png" alt="image-20250124105511325" style="zoom: 25%;" />


​		

![image-20250124110739946](C:\Users\TOM\AppData\Roaming\Typora\typora-user-images\image-20250124110739946.png)



* 计算

$$
PWM频率：	Freq = CK_PSC / (PSC + 1) / (ARR + 1)\\n

PWM占空比：	Duty = CCR / (ARR + 1)\\n

PWM分辨率：	Reso = 1 / (ARR + 1)
$$

* 信号评价：余辉（波形是否稳定）、过冲、上升时间、相移

  GPIO_SPEED越大，过冲越明显

​				<img src="C:\Users\TOM\Documents\GitHub\STM32_LightingCar\信号分析\输入时钟\PWM_50Mhz.png" alt="PWM_50Mhz" style="zoom: 25%;" />				<img src="C:\Users\TOM\Documents\GitHub\STM32_LightingCar\信号分析\输入时钟\PWM_10Mhz.png" alt="PWM_10Mhz" style="zoom: 25%;" />



#### 上电时序

![image-20250205085707340](C:\Users\TOM\AppData\Roaming\Typora\typora-user-images\image-20250205085707340.png)

只有当XCLK、IO、PDN配置好后，I2C才能正常工作；再进一步，I2C写入配置后，才有数据SD输出。



#### I2C、SPI配置

使用STM32的硬件I2C、硬件SPI。





### **链路层（Data Link Layer）：**

#### 	寄存器的初始化



​	

#### 	SPI数据解析



## 3. 屏幕

​	





