#include "sys.h"
#include "usart.h"	  
  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}
void USART1_Send_Byte(unsigned char byte)//串口发送一个字节
{
	USART_SendData(USART1, byte);//通过库函数发送数据
	while( USART_GetFlagStatus(USART1,USART_FLAG_TC)!= SET);
	//等待发送完成。检测 USART_FLAG_TC 是否置1；//见库函数 P359 介绍

}

//发送字符串 函数应用指针 方法
void UART1_Send_Str(unsigned char *s)//发送字符串 函数应用指针 方法
{
//	unsigned char i=0;//定义一个局部变量用来 发送字符串 ++运算

//	while(s[i]!='\0')// 每个字符串结尾 都是以结尾的
//	{
//	USART_SendData(USART1,s[i]);//通过库函数发送数据
//	while( USART_GetFlagStatus(USART1,USART_FLAG_TC)!= SET);
//	//等待发送完成。检测 USART_FLAG_TC 是否置1；//见库函数 P359 介绍

//	i++;//i++一次
	  while(*s!='\0')
		USART1_Send_Byte(*s++);			

}
	

u8 bz,ms,Rxnum,Rxbuff[10],zhi[2],i;	 //暂存数据切换变量
u8 Res;
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res=USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
	 		  		 
  } 
     //溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
     if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
    {
         USART_ReceiveData(USART1);
         USART_ClearFlag(USART1,USART_FLAG_ORE);
     }
      USART_ClearFlag(USART1,USART_IT_RXNE); //一定要清除接收中断
}	

//初始化IO 串口2
//bound:波特率
void uart2_init(u32 bound)
{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);	//使能USART2，GPIOA时钟
	
 	USART_DeInit(USART2);  //复位串口1
	 //USART1_TX   PA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART1_RX	  PA.3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3

   //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART2, ENABLE);                    //使能串口 

}


void USART2_Send_Byte(unsigned char byte)//串口发送一个字节
{
	USART_SendData(USART2, byte);//通过库函数发送数据
	while( USART_GetFlagStatus(USART2,USART_FLAG_TC)!= SET);
	//等待发送完成。检测 USART_FLAG_TC 是否置1；//见库函数 P359 介绍

}

//发送字符串 函数应用指针 方法
void UART2_Send_Str(unsigned char *s)//发送字符串 函数应用指针 方法
{

	  while(*s!='\0')
		USART2_Send_Byte(*s++);	
			

}
//unsigned int co2;	 //co2
//u8 date[9];
//void USART2_IRQHandler(void)                	//串口1中断服务程序
//{
//	 static u8 j;
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
//	{

//   date[j]=USART_ReceiveData(USART2); 
//  }
//     //溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
//     if(USART_GetFlagStatus(USART2,USART_FLAG_ORE) == SET)
//    {
//         USART_ReceiveData(USART2);
//         USART_ClearFlag(USART2,USART_FLAG_ORE);
//     }
//      USART_ClearFlag(USART2,USART_IT_RXNE); //一定要清除接收中断
//}
	
