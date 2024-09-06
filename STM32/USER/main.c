#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "LED.h"
#include "oled.h"
#include "myiic.h"
#include "adc.h"
#include "WT588D.h"

u8 AD_ZERO=5;  //���������Ư��
int m3_value=0, new_m3_value=0; // m3_value could be negative so it should be signed int
u8 m3_value_H = 50;// inital threshold of alcohol density
u8 adcx1;	//adc sampling quantity for light sensor
u32 adcx; //adc sampling quantity for alcohol detector
u8 key_mode = 0;	//����ģʽ; �������ģʽΪ0�����ڵ�����ֵ��Χ��״̬������ͨ��ADC��ȡ�����������;ƾ���������ֵ
u8 c1=0; //the sign of sending message

//scan the key
void scant()
 {
  		if(!key1){	//��key1(����ģʽ�İ�������)Ϊ0ʱ��key1������������룬������ʱ��key1���Ǹߵ�ƽ1��
					while(!key1);		//����������ڰ��µ�ʱ����ʲô������
					// ״̬��
					if(key_mode == 0){
							key_mode=1;	// key_mode��Ϊ1�� �������ģʽ
					}else{ 					// 1 -> 0       	
							key_mode=0;	// key_mode��Ϊ0���ָ�����ģʽ
					}    
			
			   //����������ʾ
					switch(key_mode){
							case 0: 
								OLED_ShowString(112,0,"  ",16);
						    break;
							       
							case 1: 
								OLED_ShowString(112,0,"S",16);	// �ҽ�H��ΪS����ʾ���롰set"ģʽ
						    JDQ=1;LED1=1;c1=0; // LED1 is buzzer
								break;
				
							default: 
								break;
					}
			}	
////////////////////�ƾ�Ũ������////////////////////////////////////
//ģʽa��ͨ��A�������üӼ���ֵ
		if(key_mode==1){
				if(!key2){
				  //delay_ms(100);
					if(!key2){	//����Ƕһ��if�ж�������
					//	while(!key2);
						// ״̬��
							if(m3_value_H<200) {
								m3_value_H++;
							} else {
								m3_value_H=0;
							}							
					}
				}
				
				if(!key3){
				//delay_ms(100);
					if(!key3){  //����Ƕһ��if�ж�������
					//		while(!key3);
						// ״̬��
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

/********GSM���ڽ��ռ�����**********/
unsigned char  GSMDATA_count; 
unsigned char *content1 = "The alcohol concentration exceeds the standard, please drive safely\r\n";  //��������

 /*****��Ӣ�Ķ���********
 ******content:��������*/
void Send_message_gsm(unsigned char *content)	  //Global system for Mobile Communication(gsm)
{
	UART1_Send_Str("AT\r\n"); 
	delay_ms(500);
	UART1_Send_Str("AT+CMGF=1\r\n"); //Ӣ�Ķ���
	delay_ms(500);
	UART1_Send_Str("AT+CSMP=17,167,2,25\r\n");//���ö����ı�ģʽ����
	delay_ms(500);
	UART1_Send_Str("AT+CSCS=\"GSM\"\r\n");     //����Ӣ�ĸ�ʽ�ֽڼ�
	delay_ms(500);
  UART1_Send_Str("AT+CMGS=\"18260179805\"\r\n");//�ͻ��ֻ�����
	delay_ms(500);
	UART1_Send_Str(content); // ���Ͷ������ݣ�
	delay_ms(500);
	USART1_Send_Byte(0x1A);//�س�����
	delay_ms(1000);
}

int main(void)
{
	delay_init();//��ʱ��ʼ��5
	KEY_Init();  //������ʼ��
	Init_LEDpin();//������LED��ʼ��
	GPIOF_init(); //WT588D��GPIO��ʼ��
	BUSY_IN();		//WT588D��Busy��ʼ��
	uart_init(9600);//���ڳ�ʼ��
	delay_ms(500);
	Adc_Init();		  		//ADC��ʼ��	  
  IIC_Init();   			//IIC��ʼ��
	OLED_Init();			  //OLED��ʼ��	��ʾ
	
	OLED_ShowString(0,0,"People:",16);	
  OLED_ShowString(0,16,"Alcohol:",16);
	
	OLED_ShowString(0,32,"Th:",16);		// Threshold of alcohol density
	OLED_ShowNum(32,32,m3_value_H,3,16);// m3_value_H initially is 50
	
	OLED_ShowString(72,32,"Li:",16);	// Light density
	
	OLED_ShowString(0,48,"Status:",16);
	OLED_Refresh_Gram();						//������ʾ��OLED
	delay_ms(500);	

	while(1){ 
		scant();//����ɨ��
		
		if(key_mode==0){	// normal state
				/*************Thermal infrared Sensor***********/
				if(RHW==1){    //�ߵ�ƽ˵�����ˣ�RHW������Ϊ�������裬ͨ��Ϊ�ߵ�ƽ
				 //  OLED_ShowHz(72,0,17,1);
	        OLED_ShowString(64,0,"yes",16);	
				} else {
				   OLED_ShowString(64,0,"no ",16);			
				}
				
				/****************Light Sensor*******************/
				adcx1 = Lsens_Get_Val(5);		  //5ms֮�ڵĹ�ǿ�ȵ�ƽ��ֵ, adc�ŵ�ΪADC_Channel_1
				OLED_ShowNum(96,32,adcx1,3,16);//��ʾ��ǿ��
				
				/**************Alcohol Sensor******************/
				//adcx=Get_Adc(ADC_Channel_0);
				adcx = Get_Adc_Average(ADC_Channel_0,5); //5ms�ھƾ�Ũ�ȵ�ƽ��ֵ, adc�ŵ�ΪADC_Channel_0
		    m3_value = (float) adcx * 20 * ( 5.0 / 4096 );//����Ũ����ֵ, 12-bit�Ĳ���λ��5V�ĵ�ѹ��Χ��m3_value��ֵλ0��100����ϵ����20
			  new_m3_value = m3_value - AD_ZERO;   // ȥ�����������ƫ�ƣ�AD_ZERO
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
				
				/************�ƾ�Ũ�ȳ�����ֵ*******************/
			 if(new_m3_value > m3_value_H){  //�ƾ�Ũ�ȴ�������
				 	OLED_ShowString(64,48,"abnormal",16);//�쳣 
				  //OLED_Refresh_Gram();//������ʾ��OLED	 			  		  					 
				  JDQ=0;//�жϼ̵����豸������ɫLED���ڵĵ�·��·���̵��𣻺�ɫLED���ڵĵ�·ͨ·�������
				  LED1=0;//����������	
				  Voice_out(0); //��������		  
          delay_ms(500);				  
					if(c1 == 0){														
						 Send_message_gsm(content1);	  //���Ͷ��ű���
						 c1 = 1;												//���ƾ�Ũ�ȳ�����ֵ��ʱ��ͨ�����c1��ȷ��ֻ����һ�ζ���
					}			
			  } else{
					  c1=0;JDQ=1;LED1=1;
				    OLED_ShowString(64,48,"normal  ",16);//����	
			  } 
		}
	
		OLED_Refresh_Gram();//������ʾ��OLED	 		
	}
}
