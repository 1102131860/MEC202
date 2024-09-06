#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define key3  PAin(7)   	
#define key2  PAin(6)	 
#define key1  PAin(5)	 
#define RHW  PAin(8)		//Thermal infrared sensor

void KEY_Init(void);  //IO initalize
u8 KEY_Scan(u8 mode); //scan the key, but there is no definition in key.c

#endif
