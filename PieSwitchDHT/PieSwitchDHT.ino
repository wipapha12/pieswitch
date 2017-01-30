#include "config.h"      // เรียกใช้งาน config appid, key, secret, wifi_ssid และ wifi_password  
#include "PieRelay.h"   // เรียกใช้งานไลบรารี่สำหรับควบคุม Relay
#include <ESP8266WiFi.h>  // เรียกใช้งานไลบรารี่สำหรับ esp8266
#include <MicroGear.h>  // เรียกใช้งานไลบรารี่สำหรับเชื่อมต่อ NETPIE
#include "DHT.h"      // เรียกใช้งานไลบรารี่สำหรับอ่านค่า DHT Sensor

#define ENABLE_DHT
#define DHTPIN      14    // กำหนด PIN ที่ต่อขา DATA ของ DHT Sensor
#define DHTTYPE     DHT11 // กำหนดรุ่นของ DHT Sensor

#define RELAYPIN1  13
#define RELAYPIN2  12
#define BUTTONPIN1 2
#define BUTTONPIN2 0

#ifdef ENABLE_DHT
DHT dht(DHTPIN, DHTTYPE); // สร้าง object สำหรับ DHT Sensor
int lastDHTRead   = 0;    // timestamp ล่าสุดสำหรับอ่านค่า DHT Sensor
int lastFeedWrite = 0;    // timestamp ล่าสุดสำหรับเก็บข้อมูลลง Feed
float humid = 0;      // ตัวแปรเก็บความชื้น
float temp = 0;       // ตัวแปรเก็บอุณหภูมิ
#endif

const char* ssid     = WIFI_SSID;   // กำหนด wifi ssid ในไฟล์ config.h
const char* password = WIFI_PASSWORD;   // กำหนด wifi password ในไฟล์ config.h

WiFiClient client;
MicroGear microgear(client);

PieRelay *sw1, *sw2;
String data;
bool toPublishSW1State, toPublishSW2State;
int lastDHTPublish = 0;   // timestamp ล่าสุดสำหรับส่งค่า DHT Sensor

void stateChange(int relayID, int type, int state) {
  Serial.print("relay "); Serial.print(relayID); Serial.print(" --> "); Serial.println(state?"ON":"OFF");
  if (relayID==1) toPublishSW1State = true;
  if (relayID==2) toPublishSW2State = true;
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char m = *(char *)msg;
  msg[msglen] = '\0';
  Serial.print("incoming message: "); Serial.print((char *)topic); Serial.print(" --> "); Serial.println((char*)msg);

  if (msg[0]=='1' && msg[1]=='0') sw1->setState(0);
  if (msg[0]=='1' && msg[1]=='1') sw1->setState(1);
  if (msg[0]=='2' && msg[1]=='0') sw2->setState(0);
  if (msg[0]=='2' && msg[1]=='1') sw2->setState(1);
  if (msg[0]=='1' && msg[1]=='?') toPublishSW1State = true;
  if (msg[0]=='2' && msg[1]=='?') toPublishSW2State = true;
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    microgear.setAlias(ALIAS);
    toPublishSW1State = true;
    toPublishSW2State = true;
}

/* Initial WIFI, this is just a basic method to configure WIFI on ESP8266.                       */
/* You may want to use other method that is more complicated, but provide better user experience */
void initWiFi(){
    if (WiFi.begin(ssid, password)) {
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
    }
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  sw1 = new PieRelay(1, RELAYPIN1, BUTTONPIN1);
  sw2 = new PieRelay(2, RELAYPIN2, BUTTONPIN2);

  sw1->setCallback(STATECHANGE, stateChange);
  sw2->setCallback(STATECHANGE, stateChange);

  #ifdef ENABLE_DHT
    dht.begin();      // เรียกใช้การอ่านค่า DHT Sensor
  #endif
     
  initWiFi();

  microgear.on(MESSAGE,onMsghandler);
  microgear.on(CONNECTED,onConnected);
  microgear.init(KEY,SECRET);
  microgear.connect(APPID);

  lastFeedWrite = millis();   // เริ่มต้นเก็บ timestamp สำหรับเก็บข้อมูลลง Feed
}

void loop() {
    #ifdef ENABLE_DHT
      if (millis() - lastDHTRead > 2000) {  // ตรวจสอบ timestamp ล่าสุดที่อ่านค่า DHT Sensor หากเกิน 2 วินาที ให้ทำงานคำสั่งข้างล่าง
        float h = dht.readHumidity();   // อ่านค่าความชื้นเก็บลงตัวแปร h
        float t = dht.readTemperature();    // อ่านค่าอุณหภูมิเก็บลงตัวแปร t
        if (!isnan(h)) humid = h;     // ตรวจสอบความชื้นที่ได้เป็นตัวเลขหรือไม่ ถ้าใช่เก็บลงตัวแปร humid
        if (!isnan(t)) temp = t;      // ตรวจสอบอุณหภูมิที่ได้เป็นตัวเลขหรือไม่ ถ้าใช่เก็บลงตัวแปร temp
        lastDHTRead = millis();     // อัพเดท timestamp ล่าสุดที่อ่านค่าจาก DHT Sensor
      }    
    #endif

    if(WiFi.status()!=WL_CONNECTED){
      WiFi.disconnect();
      initWiFi();
    } else {
        if (microgear.connected()) {
            if (toPublishSW1State) {
              microgear.publish("/state1",sw1->getState()?"1":"0");
              toPublishSW1State = false;
            }

            if (toPublishSW2State) {
              microgear.publish("/state2",sw2->getState()?"1":"0");
              toPublishSW2State = false;
            }

            if (millis() - lastFeedWrite > 15000) { // ตรวจสอบ timestamp ล่าสุดที่เก็บข้อมูล DHT Sensor ลง Feed หากเกิน 15 วินาที ให้ทำงานคำสั่งข้างล่าง
              data = "temp:"+String(temp)+",humid:"+String(humid);
              Serial.print("Writing data to feed --> ");
              Serial.println(data);
              microgear.writeFeed(FEEDID,data,FEEDAPIKEY);  // เก็บข้อมูล DHT Sensor ลง Feed
              lastFeedWrite = millis(); // อัพเดท timestamp ล่าสุดที่เก็บข้อมูล DHT Sensor ลง Feed
            }

            if (millis() - lastDHTPublish > 2000) { // ตรวจสอบ timestamp ล่าสุดที่ส่งค่า DHT Sensor หากเกิน 2 วินาที ให้ทำงานคำสั่งข้างล่าง
              data = String(temp)+String(",")+String(humid);
              Serial.print("Publish dht data --> ");
              Serial.println(data);
              microgear.publish("/dht",data);   // ส่งค่า DHT Sensor ไปที่ topic ชื่อ “/dht”
              lastDHTPublish = millis();    // อัพเดท timestamp ล่าสุดที่ส่งค่า DHT Sensor
             }
            microgear.loop();
        }
        else {
          microgear.connect(APPID);
          delay(500);
        }
    }
}

