#include "WT588D.h"
#include "stm32f10x.h"
#include "delay.h"
#define BUSY  PBin(15)  //输入
u8 yu_flag;
void GPIOF_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP ; //输出推挽电路
	GPIO_Init(GPIOB,&GPIO_InitStructure);	
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
 }
			 
void BUSY_IN(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
		
	
}
			 

void Voice_out(int16_t addr)				// addr is the base address
{
	 unsigned char i;
	 delay_ms(20);// 等待20ms
	 GPIO_ResetBits(GPIOB,GPIO_Pin_13);// CS=0; 
	 delay_ms(5); // 片选信号保持低电平5ms
	 for(i=0;i<8;i++)
	 {
	 GPIO_ResetBits(GPIOB,GPIO_Pin_12);//CLK=0; 
	 if((addr&0x01)==0x01)GPIO_SetBits(GPIOB,GPIO_Pin_14); //DATA=1; 
	 else GPIO_ResetBits(GPIOB,GPIO_Pin_14);//DATA=0; 
		delay_us(150);//延时150us 
	 GPIO_SetBits(GPIOB,GPIO_Pin_12);//CLK=1; 
	 addr>>=1;	 
	 delay_us(150);//延时150us
	 }
	 delay_ms(100);
	 GPIO_SetBits(GPIOB,GPIO_Pin_13);//CS=1;
	 delay_ms(100);
	 while(BUSY==0);//等待放完声音 
   yu_flag=1;	
}

