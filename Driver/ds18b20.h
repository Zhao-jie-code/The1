#ifndef __DS18B20_H
#define __DS18B20_H 
#include "sys.h"   
   	
//如果想要修改引脚，只需修改下面的宏
#define RCC_DS18B20_PORT 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define DS18B20_GPIO_PIN      GPIO_Pin_5
#define DS18B20_GPIO_PORT     GPIOB

//IO方向设置（CRL寄存器对应引脚0~7,CRH寄存器对应引脚8~15）
#define DS18B20_IO_IN()  {DS18B20_GPIO_PORT->CRL&=0xFF0FFFFF;DS18B20_GPIO_PORT->CRL|=0x00800000;}
#define DS18B20_IO_OUT() {DS18B20_GPIO_PORT->CRL&=0xFF0FFFFF;DS18B20_GPIO_PORT->CRL|=0x00300000;}

#define DS18B20_OUT_0   GPIO_ResetBits(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)//IO为低电平
#define DS18B20_OUT_1   GPIO_SetBits(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)//IO为高电平
#define READ_DS18B20_IO GPIO_ReadInputDataBit(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)//读取IO电平
		
void DS18B20_GPIO_Init(void);
u8 DS18B20_Init(void);			//初始化DS18B20
float ReadTemperature(void); //获取温度值

#endif

















