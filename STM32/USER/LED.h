#ifndef __LED_H
#define __LED_H

#include "sys.h"

#define LED1 PCout(13)// Perpheral C pin 13 is connected to buzzer
#define JDQ PBout(9)// Perpheral B pin 9 is connceted to relay(¼ÌµçÆ÷)

extern void Init_LEDpin(void);


#endif

