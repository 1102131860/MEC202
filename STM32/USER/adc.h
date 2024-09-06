#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

void  Adc_Init(void);
u8  Get_Val(u8 times);
u16 get_Adc_Value(u8 ch);//通道一采样值
u16 Get_Adc_Average(u8 ch,u8 times);
u16  Get_Adc(u8 ch);
void Lsens_Init(void); 				//初始化光敏传感器
u8   Lsens_Get_Val(u8 times);

#endif 
