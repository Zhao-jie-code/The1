#include "sys.h"
#include "adc.h"
#include "delay.h"
#include "lcd1602.h"
#include "ds18b20.h"
#include "usart1.h"
#include "timer.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STM32_RX1_BUF       Usart1RecBuf 
#define STM32_Rx1Counter    RxCounter
#define STM32_RX1BUFF_SIZE  USART1_RXBUFF_SIZE

unsigned int  light=0;
unsigned char temperature=0;
unsigned char setTempValue=35;        //温度设置值
unsigned int  setSoilMoisture=10;
unsigned char setLightValue=20;       //光照设置值
unsigned int  soilMoisture;           //土壤湿度
unsigned int  CO2=0;
unsigned int  CO2_Max=2000;//二氧化碳上限

bool usart_send_flag = 0;
bool mode = 0;               //0是自动模式，1是手动模式
bool shuaxin  = 0;
bool shanshuo = 0;
bool sendFlag = 1;

unsigned char setn=0;//记录设置键按下的次数

void Get_CO2(void)//获取二氧化碳浓度
{
	  unsigned char len,i,j=0,str; 
	  char *presult=0;
	  char buf[5];
	
		if(strstr(STM32_RX1_BUF,"ppm\r\n")!=NULL)
		{
					presult = STM32_RX1_BUF + 2;
			    len = strlen(presult);	
          if(len <= 10)	//5000 ppm\r\n	一个数据最多10个长度
					{						
							for(i=0; i < len; i++)
							{
									if(presult[i]!=' ')j++;else break;
							}
							memset(buf, 0, 5);//清空发送缓存区
							strncpy(buf, presult, j);
							str = strlen(buf);//计算总数据长度
							buf[str]=' ';
							buf[str+1]='\0';//加上结尾符 
							CO2 = atoi(buf);	//将字符串转换为整数				
							if(CO2>=5000)CO2=5000;
					}
			    memset(STM32_RX1_BUF, 0, STM32_RX1BUFF_SIZE);//清除缓存
					STM32_Rx1Counter = 0;//清除串口数据，准备下一次数据接收
		}
		if(CO2>=CO2_Max && shanshuo)
		{
			  LCD_Write_Char(9,1,' '); 
				LCD_Write_Char(10,1,' ');
        LCD_Write_Char(11,1,' '); 
				LCD_Write_Char(12,1,' '); 			
		}
		else
		{
				LCD_Write_Char(9,1,CO2/1000+'0'); 
				LCD_Write_Char(10,1,CO2%1000/100+'0'); 
			  LCD_Write_Char(11,1,CO2%100/10+'0'); 
			  LCD_Write_Char(12,1,CO2%10+'0'); 
		}
}

void displayLight(void)//显示光照
{
		u16 test_adc=0;
	
	  /////////////获取光线值
	  test_adc = Get_Adc_Average(ADC_Channel_8,10);//读取通道9的5次AD平均值
		light = test_adc*99/4096;//转换成0-99百分比
		light = light >= 99? 99: light;//最大只能到百分之99
	  if(light<=setLightValue && shanshuo)
		{
			  LCD_Write_Char(3,0,' '); 
				LCD_Write_Char(4,0,' '); 
		}
		else
		{
				LCD_Write_Char(3,0,light/10+'0'); 
				LCD_Write_Char(4,0,light%10+'0'); 
		}
}

void displaySoilMoisture(void)//显示土壤湿度
{
	  float voltage = 0.0;

	   voltage = Get_Adc_Average(ADC_Channel_9,10)*3.3/4096;
	   if(voltage > 3.3)voltage = 3.3;
	   if(voltage < 1.0) soilMoisture=99;
		 else
		 {
				 soilMoisture = (3.3 - voltage) / 0.023;  
				 if(soilMoisture > 99)soilMoisture = 99;        //最大取百分之99
		 }
		 if(soilMoisture<=setSoilMoisture && shanshuo)
		{
			 LCD_Write_Char(9,0,' '); 
			 LCD_Write_Char(10,0,' ');
		}
		else
		{
			 LCD_Write_Char(9,0,soilMoisture/10+'0'); 
			 LCD_Write_Char(10,0,soilMoisture%10+'0');
		}
}

void displayTemperature(void)//显示温度
{
		temperature=ReadTemperature();
	  if(temperature>=setTempValue && shanshuo)
		{
			 LCD_Write_Char(0,1,' '); 
			 LCD_Write_Char(1,1,' ');
		}
		else
		{
			 LCD_Write_Char(0,1,temperature/10+'0'); 
			 LCD_Write_Char(1,1,temperature%10+'0');
		}
}

void displaySetValue(void)
{
		if(setn == 1)
		 {
				LCD_Write_Char(7,1,setSoilMoisture/10+'0'); 
			  LCD_Write_Char(8,1,setSoilMoisture%10+'0');
		 }
		 if(setn == 2)
		 {
				LCD_Write_Char(7,1,setTempValue/10+'0'); 
			  LCD_Write_Char(8,1,setTempValue%10+'0');
		 }
		 if(setn == 3)
		 {
				LCD_Write_Char(7,1,setLightValue/10+'0'); 
			  LCD_Write_Char(8,1,setLightValue%10+'0');
		 }
		 if(setn == 4)
		 {
				LCD_Write_Char(5,1,CO2_Max/1000+'0'); 
				LCD_Write_Char(6,1,CO2_Max%1000/100+'0'); 
			  LCD_Write_Char(7,1,CO2_Max%100/10+'0'); 
			  LCD_Write_Char(8,1,CO2_Max%10+'0'); 
		 }
}

void keyscan(void)
{
	if(KEY1 == 0)//模式切换按键
	{
	 	delay_ms(20);//消抖
		if(RELAY3==1)delay_ms(50);
		if(KEY1 == 0)
		{
		 	while(KEY1 == 0);//等待按键松开
			BEEP=0;
			setn ++;
			 if(setn == 1)
			 {
					LCD_Write_String(0,0,"set the Moisture");//显示字符串
					LCD_Write_String(0,1,"       00%      ");
			 }
			 if(setn == 2)
			 {
					LCD_Write_String(0,0,"  set the Temp  ");//显示字符串
					LCD_Write_String(0,1,"       00 C     ");
				  LCD_Write_Char(9,1,0xdf);
			 }
			 if(setn == 3)
			 {
					LCD_Write_String(0,0,"  set the Light ");//显示字符串
					LCD_Write_String(0,1,"       00%      ");
			 }
			 if(setn == 4)
			 {
					LCD_Write_String(0,0,"   set the CO2  ");//显示字符串
					LCD_Write_String(0,1,"     0000ppm    ");
			 }
			 if(setn == 5)
			 {
					LCD_Write_String(0,0,"  set the mode  ");//显示字符串
					LCD_Write_String(0,1,"       ZD       ");
				  if(mode==0)LCD_Write_String(7,1,"ZD");else LCD_Write_String(7,1,"SD");
			 }
			 displaySetValue();
			 if(setn >= 6)
			 {
					setn = 0;
				  LCD_Write_String(0,0,"Gx:  % S:  %    ");//显示字符串
					LCD_Write_String(0,1,"   C CO2:    ppm");
					LCD_Write_Char(2,1,0xdf);
				  if(mode==0)LCD_Write_String(13,0,"ZD");else LCD_Write_String(13,0,"SD");
			 }
		}
	}
	if(KEY2 == 0)//模式切换按键
	{
	 	delay_ms(20);//消抖
		if(RELAY3==1)delay_ms(50);
		if(KEY2 == 0)
		{
       while(KEY2 == 0);
			 if(setn == 0 && mode==1)//手动
			 {
					RELAY1=~RELAY1;
			 }
       if(setn == 1)
			 {
					if(setSoilMoisture<99)setSoilMoisture++;
			 }
			 if(setn == 2)
			 {
					if(setTempValue<99)setTempValue++;
			 }
			 if(setn == 3)
			 {
				 if(setLightValue<99)setLightValue++;
			 }
			 if(setn == 4)
			 {
				 if(CO2_Max<5000)CO2_Max+=10;
			 }
			 if(setn == 5)
			 {
				  mode=0;
				  LCD_Write_String(0,1,"       ZD       ");
			 }
			 displaySetValue();
		}
	}
	if(KEY3 == 0)//加键
	{
		delay_ms(20);//消抖
		if(RELAY3==1)delay_ms(50);
		if(KEY3 == 0 )
		{
			while(KEY3 == 0);
			if(setn == 0 && mode==1)//手动
			 {
					RELAY2=~RELAY2;
			 }
      if(setn == 1)
			 {
					if(setSoilMoisture>0)setSoilMoisture--;
			 }
			 if(setn == 2)
			 {
					if(setTempValue>0)setTempValue--;
			 }
			 if(setn == 3)
			 {
				 if(setLightValue>0)setLightValue--;
			 }
			 if(setn == 4)
			 {
				 if(CO2_Max>=10)CO2_Max-=10;
			 }
			 if(setn == 5)
			 {
				  mode=1;
				  LCD_Write_String(0,1,"       SD       ");
			 }
			 displaySetValue();
		}
	}
	
	if(KEY4 == 0)//减键
	{
		delay_ms(20);//消抖
		if(RELAY3==1)delay_ms(50);
		if(KEY4 == 0 )
		{
			while(KEY4 == 0);
      if(setn == 0 && mode==1)//手动
			 {
					RELAY3=~RELAY3;
			 }
		}
	}
}

int main(void)
{	
		delay_init();	    //延时函数初始化	  
	  NVIC_Configuration();
		delay_ms(500);       //上电瞬间加入一定延时在初始化
	  DS18B20_GPIO_Init();
	  Adc_Init();          //ADC初始化
	  KEY_GPIO_Init();    //按键初始化
		LCD_Init();         //屏幕初始化
	  DS18B20_Init();
	  uart1_Init(9600);
	  LCD_Write_String(0,0,"Gx:00% S:00% ZD ");//显示字符串
	  LCD_Write_String(0,1,"00 C CO2:0000ppm");
	  LCD_Write_Char(2,1,0xdf);
		TIM3_Init(99,719);   //定时器初始化，定时1ms
		//Tout = ((arr+1)*(psc+1))/Tclk ; 
		//Tclk:定时器输入频率(单位MHZ)
		//Tout:定时器溢出时间(单位us)
		while(1)
		{  
			 keyscan();
			 if(setn == 0)
			 {
				   if(shuaxin == 1)
					 {
						   shuaxin = 0;
							 displayLight();   //显示光照
							 displaySoilMoisture(); //显示土壤湿度
							 displayTemperature();  //显示温度
						   Get_CO2();
							 if(mode==0)   
							 {
									 if(light<=setLightValue)RELAY1=1;else RELAY1=0;              //光线暗开灯
									 if(temperature>=setTempValue||CO2>=CO2_Max)RELAY2=1;else RELAY2=0;         //温度高开风扇
									 if(soilMoisture<=setSoilMoisture)RELAY3=1;else RELAY3=0;     //湿度低开水泵
								 
									 if(light<=setLightValue||temperature>=setTempValue||soilMoisture<=setSoilMoisture||CO2>=CO2_Max)BEEP=1;else BEEP=0;  //蜂鸣器提醒
							 }
							 else
							 {
									 BEEP=0;
							 }
					 }
			 }
			 delay_ms(20);
		}	
}


void TIM3_IRQHandler(void)   //TIM3中断,50毫秒一次中断
{
		static u16 timeCount1 = 0;
    static u16 timeCount2 = 0;
	
		if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
        timeCount1++;
			  timeCount2++;
        if(timeCount1 >= 300)  //300ms
				{
						timeCount1 = 0;
					  shanshuo = !shanshuo;
					  shuaxin = 1;
				}
				if(timeCount2 >= 800)  //800ms
				{
						timeCount2 = 0;
					  sendFlag = 1;
				}
		}
}

