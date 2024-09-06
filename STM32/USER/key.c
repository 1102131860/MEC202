#include "key.h"
#include "delay.h"
	  	    
//key initilization function
//PA15和PC5 设置成输入
void KEY_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//enable PORTA clock
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //initially push up
	
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//initialize GPIOA0
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;//PA1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // initially push up
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//initialize GPIOA1
 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;//PA2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //initially push up
	GPIO_Init(GPIOA, &GPIO_InitStructure);//initialize GPIOA2
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;//PA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //initially push up
	GPIO_Init(GPIOA, &GPIO_InitStructure);//initialize GPIOA3
	
} 

