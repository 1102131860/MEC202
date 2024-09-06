// /*
//    MQ-2烟雾传感器的使用
// */
// #include <Arduino.h>

// #define Sensor_AO A0
// #define Sensor_DO 7

// unsigned int sensorValue = 0;

// void setup()
// {
//   pinMode(Sensor_DO, INPUT);
//   Serial.begin(9600);
// }

// void loop()
// {
//   sensorValue = analogRead(Sensor_AO);
//   Serial.print("Sensor AD Value = ");
//   Serial.println(sensorValue);

//   if (digitalRead(Sensor_DO) == LOW)
//   {
//     Serial.println("Alarm!");
//   }
//   delay(1000);
// }


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128     // 设置OLED宽度,单位:像素
#define SCREEN_HEIGHT 64    // 设置OLED高度,单位:像素
// 自定义重置引脚,虽然教程未使用,但却是Adafruit_SSD1306库文件所必需的
#define OLED_RESET     4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //该OLED模块的IIC地址为0x3C
}

void loop() {
  words_display();
  display.display();
}

void words_display()
{
  // 清除屏幕
  display.clearDisplay();
 
  // 设置字体颜色,白色可见
  display.setTextColor(WHITE);
 
  //设置字体大小
  display.setTextSize(1.5);
 
  //设置光标位置
  display.setCursor(0, 0);
  display.print("TaichiMaker");
 
  display.setCursor(0, 20);
  display.print("time: ");
  //打印自开发板重置以来的秒数：
  display.print(millis() / 1000);
  display.print(" s");
 
  display.setCursor(0, 40);
  display.print("Author: ");
  display.print("Dapenson");
}