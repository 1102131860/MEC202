#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <SoftwareSerial.h>
#include <TinyGPS.h>
int alcohol_val = 0;
TinyGPS gps;
SoftwareSerial ss(4, 5);

#define alcohol 36
#include <WiFi.h>
/* 依赖 PubSubClient 2.4.0 */
#define MQTT_MAX_PACKET_SIZE 1024
#define MQTT_KEEPALIVE 60
#include <PubSubClient.h>
/* 依赖 ArduinoJson 5.13.4 */
//updated to comply with ardJson 6
#include <ArduinoJson.h>


/*WIFI配置*/
#define WIFISSID "Mate 40 Pro"     //WIFI名称
#define WIFIPSW "123456789"  //WIFI密码

/*MQTT服务器设置*/
#define MQTT_SERVER "bemfa.com"              //MQTT服务器
#define MQTT_SERVER_PORT 9501                //MQTT服务器端口
#define MQTT_ID "ae6ee58c78014529956e80366068c604"  //巴法云密钥

WiFiClient MQTTclient;            //WIFI客户端对象
PubSubClient client(MQTTclient);  //MQTT客户端对象

long lastMsg = 0;  //上次消息发布时间
char msg[50];      //消息

/*订阅的主题有消息发布时的回调函数*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);  // 打印主题信息
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // 打印主题内容
  }
  Serial.println();
}

/*重连MQTT函数*/
void MQTT_reconnect() {

  /*如果没连接成功一直循环*/
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // 尝试去连接
    if (client.connect(MQTT_ID)) {
      Serial.println("connected");  //连接成功
      client.subscribe("Altitude");    //订阅主题
      client.subscribe("Latitude");    //订阅主题
      client.subscribe("Longitude");    //订阅主题
      client.subscribe("Velocity");    //订阅主题
    } else {
      Serial.print("failed, rc=");  //连接失败，输出状态，五秒后重试
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}

/*WIFI重连（等待）函数，WIFI.begin后需要一段时间才能连接成功*/
void WIFI_reconnect() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    delay(500);
  }

  /*连接成功输出IP地址*/
  Serial.print("\r\nGet IP Address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_loop() {
  /*检查WIFI连接*/
  if (WiFi.status() != WL_CONNECTED) WIFI_reconnect();
  /*若WIFI已经连接，则检查MQTT连接*/
  else if (!client.connected()) MQTT_reconnect();
  client.loop();
}
void mqttIntervalPost(float a, float b, float c, float d){
  Serial.println("Posting");
    char msg[100];
    dtostrf(a, 4, 6, msg);
    client.publish("Latitude", msg);  //向指定主题发布消息
    dtostrf(b, 4, 6, msg);
    client.publish("Longitude", msg);  //向指定主题发布消息
    dtostrf(c, 4, 6, msg);
    client.publish("Altitude", msg);  //向指定主题发布消息
    dtostrf(d, 4, 6, msg);
    client.publish("Velocity", msg);  //向指定主题发布消息
}

static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
static void print_float(float val, float invalid, int len, int prec) {
  if (val == invalid) {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  } else {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);  // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3
                           : vi >= 10  ? 2
                                       : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len) {
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS& gps) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
            month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char* str, int len) {
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartdelay(0);
}
unsigned long lastpublished = 0;
void gps_audit() {
  float flat, flon;
  String location, speed;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;

  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  print_date(gps);
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  //print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  //print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  //print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
  


    speed = "SPEED " + String(gps.f_speed_kmph()) + " Km/h";
    //print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
    display.setCursor(0, 20);
    display.print("LAT ");
    display.print(flat);
    display.print("\nLON ");
    display.print(flon);
    display.print("\nALT ");
    display.print(gps.f_altitude());
    display.setCursor(0, 45);
    display.print(speed);

      if (millis() - lastpublished > 5000) {
      mqttIntervalPost(flat, flon, gps.f_altitude(), gps.f_speed_kmph());
      lastpublished = millis();
      Serial.print("published.\n");
    }
}
void alc_audit() {
  display.setTextSize(1);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  //String alc_reading = "ALCOHOL: " + String(alcohol_val);
  //display.print(alc_reading);
}

void setup() {
  Serial.begin(115200);
                   //连接WIFI
  ss.begin(9600);
  Serial.println("init");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
    client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);  //连接MQTT服务器
  client.setCallback(callback); 
    WiFi.begin(WIFISSID, WIFIPSW); 
  pinMode(alcohol, INPUT);
}

void loop() {
  alcohol_val = analogRead(alcohol);
  display.clearDisplay();
  alc_audit();
  display.setCursor(0, 30);
  gps_audit();
  display.display();
  mqtt_loop();
}