#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 

#define RELAY1 PBout(6)
#define RELAY2 PBout(7)
#define RELAY3 PBout(8)
#define BEEP   PBout(9)

#define KEY1 PBin(12)
#define KEY2 PBin(13)
#define KEY3 PBin(14)
#define KEY4 PBin(15)

void KEY_GPIO_Init(void);//≥ı ºªØ
	 				    
#endif

