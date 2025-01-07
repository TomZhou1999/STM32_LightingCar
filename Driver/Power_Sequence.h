#ifndef __POWER_SEQUENCE_H__
#define __POWER_SEQUENCE_H__

#define Sensor_Pin_MCLK GPIO_Pin_8		//PA8 MCO/PWM
#define Sensor_Pin_PWD GPIO_Pin_1		//PA1 
#define Sensor_Pin_SDA GPIO_Pin_11		//PB11 HARDWARE I2C
#define Sensor_Pin_SCL GPIO_Pin_10		//PB10 HARDWARE I2C
#define Sensor_Pin_PCLK GPIO_Pin_5		//PA8 HARDWARE SPI
#define Sensor_Pin_Data0 GPIO_Pin_6		//PA8 HARDWARE SPI

void XCLKSetFrquence(uint8_t frq);
void XCLKInit(void);
void PWDInit(void);
void PWDSet(uint8_t state);
void SensorPowerON(void);
#endif
