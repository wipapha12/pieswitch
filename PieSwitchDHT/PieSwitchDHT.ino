/*  NETPIE ESP8266                                         */
/*  More information visit : https://netpie.io             */

#include "config.h"
#include "PieRelay.h"
#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include "DHT.h"

#define ENABLE_DHT
#define DHTPIN      14
#define DHTTYPE     DHT11

#define RELAYPIN1  13
#define RELAYPIN2  12
#define BUTTONPIN1 2
#define BUTTONPIN2 0

#ifdef ENABLE_DHT
DHT dht(DHTPIN, DHTTYPE);
int lastDHTRead   = 0;
int lastFeedWrite = 0;
float humid = 0;
float temp = 0;
#endif

const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WiFiClient client;
MicroGear microgear(client);

PieRelay *sw1, *sw2;
String data;
bool toPublishSW1State, toPublishSW2State;
int lastDHTPublish = 0;

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
    dht.begin();
  #endif
     
  initWiFi();

  microgear.on(MESSAGE,onMsghandler);
  microgear.on(CONNECTED,onConnected);
  microgear.init(KEY,SECRET);
  microgear.connect(APPID);

  lastFeedWrite = millis();
}

void loop() {
    #ifdef ENABLE_DHT
      if (millis() - lastDHTRead > 2000) {
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (!isnan(h)) humid = h;
        if (!isnan(t)) temp = t;
        lastDHTRead = millis();
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

            if (millis() - lastFeedWrite > 15000) {
              data = "temp:"+String(temp)+",humid:"+String(humid);
              Serial.print("Writing data to feed --> ");
              Serial.println(data);
              microgear.writeFeed(FEEDID,data,FEEDAPIKEY);
              lastFeedWrite = millis();
            }

            if (millis() - lastDHTPublish > 2000) {
              data = String(temp)+String(",")+String(humid);
              Serial.print("Publish dht data --> ");
              Serial.println(data);
              microgear.publish("/dht",data);
              lastDHTPublish = millis();
             }
            
            microgear.loop();
        }
        else {
          microgear.connect(APPID);
          delay(500);
        }
    }
}

