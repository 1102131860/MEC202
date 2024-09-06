#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "LED.h"
#include "oled.h"
#include "myiic.h"
#include "adc.h"
#include "WT588D.h"

u8 AD_ZERO=5;  //传感器零点漂移
int m3_value=0, new_m3_value=0; // m3_value could be negative so it should be signed int
u8 m3_value_H = 50;// inital threshold of alcohol density
u8 adcx1;	//adc sampling quantity for light sensor
u32 adcx; //adc sampling quantity for alcohol detector
u8 key_mode = 0;	//按键模式; 如果按键模式为0（不在调试阈值范围的状态），则通过ADC采取光敏传感器和酒精传感器的值
u8 c1=0; //the sign of sending message

//scan the key
void scant()
 {
  		if(!key1){	//当key1(调节模式的按键按下)为0时；key1是上拉电阻接入，不按下时，key1就是高电平1。
					while(!key1);		//防抖，如果在按下的时刻内什么都不做
					// 状态机
					if(key_mode == 0){
							key_mode=1;	// key_mode变为1， 进入调节模式
					}else{ 					// 1 -> 0       	
							key_mode=0;	// key_mode变为0，恢复正常模式
					}    
			
			   //按键设置显示
					switch(key_mode){
							case 0: 
								OLED_ShowString(112,0,"  ",16);
						    break;
							       
							case 1: 
								OLED_ShowString(112,0,"S",16);	// 我将H改为S，表示进入“set"模式
						    JDQ=1;LED1=1;c1=0; // LED1 is buzzer
								break;
				
							default: 
								break;
					}
			}	
////////////////////酒精浓度设置////////////////////////////////////
//模式a，通道A进行设置加减阈值
		if(key_mode==1){
				if(!key2){
				  //delay_ms(100);
					if(!key2){	//再内嵌一个if判断语句防抖
					//	while(!key2);
						// 状态机
							if(m3_value_H<200) {
								m3_value_H++;
							} else {
								m3_value_H=0;
							}							
					}
				}
				
				if(!key3){
				//delay_ms(100);
					if(!key3){  //再内嵌一个if判断语句防抖
					//		while(!key3);
						// 状态机
							if(m3_value_H>0) {
									m3_value_H--;
							} else {
								  m3_value_H=200;
							}
					}
				}
				
				OLED_ShowNum(32,32,m3_value_H,3,16);
		}
}

/********GSM串口接收计数器**********/
unsigned char  GSMDATA_count; 
unsigned char *content1 = "The alcohol concentration exceeds the standard, please drive safely\r\n";  //短信内容

 /*****发英文短信********
 ******content:发送内容*/
void Send_message_gsm(unsigned char *content)	  //Global system for Mobile Communication(gsm)
{
	UART1_Send_Str("AT\r\n"); 
	delay_ms(500);
	UART1_Send_Str("AT+CMGF=1\r\n"); //英文短信
	delay_ms(500);
	UART1_Send_Str("AT+CSMP=17,167,2,25\r\n");//设置短信文本模式参数
	delay_ms(500);
	UART1_Send_Str("AT+CSCS=\"GSM\"\r\n");     //配置英文格式字节集
	delay_ms(500);
  UART1_Send_Str("AT+CMGS=\"18260179805\"\r\n");//客户手机号码
	delay_ms(500);
	UART1_Send_Str(content); // 发送短信内容！
	delay_ms(500);
	USART1_Send_Byte(0x1A);//回车发送
	delay_ms(1000);
}

int main(void)
{
	delay_init();//延时初始化5
	KEY_Init();  //按键初始化
	Init_LEDpin();//蜂鸣器LED初始化
	GPIOF_init(); //WT588D中GPIO初始化
	BUSY_IN();		//WT588D中Busy初始化
	uart_init(9600);//串口初始化
	delay_ms(500);
	Adc_Init();		  		//ADC初始化	  
  IIC_Init();   			//IIC初始化
	OLED_Init();			  //OLED初始化	显示
	
	OLED_ShowString(0,0,"People:",16);	
  OLED_ShowString(0,16,"Alcohol:",16);
	
	OLED_ShowString(0,32,"Th:",16);		// Threshold of alcohol density
	OLED_ShowNum(32,32,m3_value_H,3,16);// m3_value_H initially is 50
	
	OLED_ShowString(72,32,"Li:",16);	// Light density
	
	OLED_ShowString(0,48,"Status:",16);
	OLED_Refresh_Gram();						//更新显示到OLED
	delay_ms(500);	

	while(1){ 
		scant();//按键扫描
		
		if(key_mode==0){	// normal state
				/*************Thermal infrared Sensor***********/
				if(RHW==1){    //高电平说明有人，RHW被设置为上拉电阻，通常为高电平
				 //  OLED_ShowHz(72,0,17,1);
	        OLED_ShowString(64,0,"yes",16);	
				} else {
				   OLED_ShowString(64,0,"no ",16);			
				}
				
				/****************Light Sensor*******************/
				adcx1 = Lsens_Get_Val(5);		  //5ms之内的光强度的平均值, adc信道为ADC_Channel_1
				OLED_ShowNum(96,32,adcx1,3,16);//显示光强度
				
				/**************Alcohol Sensor******************/
				//adcx=Get_Adc(ADC_Channel_0);
				adcx = Get_Adc_Average(ADC_Channel_0,5); //5ms内酒精浓度的平均值, adc信道为ADC_Channel_0
		    m3_value = (float) adcx * 20 * ( 5.0 / 4096 );//计算浓度数值, 12-bit的采样位，5V的电压范围，m3_value的值位0到100，当系数是20
			  new_m3_value = m3_value - AD_ZERO;   // 去除传感器零点偏移，AD_ZERO
//				  if(n_m3_value<=0)	
//				  {
//					   new_m3_value=0;
//				  }	
//				  else{
//				  	 new_m3_value= n_m3_value;
//				  }
				if(new_m3_value < 0) new_m3_value = 0;
				OLED_ShowNum(72,16,new_m3_value,3,16);
				OLED_ShowString(96,16,"mg/L",16);
				
				/************酒精浓度超过阈值*******************/
			 if(new_m3_value > m3_value_H){  //酒精浓度大于上限
				 	OLED_ShowString(64,48,"abnormal",16);//异常 
				  //OLED_Refresh_Gram();//更新显示到OLED	 			  		  					 
				  JDQ=0;//切断继电器设备，则绿色LED所在的电路断路，绿灯灭；红色LED所在的电路通路，红灯亮
				  LED1=0;//蜂鸣器报警	
				  Voice_out(0); //语音播报		  
          delay_ms(500);				  
					if(c1 == 0){														
						 Send_message_gsm(content1);	  //发送短信报警
						 c1 = 1;												//当酒精浓度超过阈值的时候，通过标记c1来确保只发送一次短信
					}			
			  } else{
					  c1=0;JDQ=1;LED1=1;
				    OLED_ShowString(64,48,"normal  ",16);//正常	
			  } 
		}
	
		OLED_Refresh_Gram();//更新显示到OLED	 		
	}
}
