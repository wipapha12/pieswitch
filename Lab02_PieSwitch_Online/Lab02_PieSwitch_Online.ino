/*  PieRelay Online version  */
 
#include "config.h"                       // load a NETPIE credential from file
#include "PieRelay.h"

#include <ESP8266WiFi.h>                  // include a wifi library
#include <MicroGear.h>                    // include a NETPIE library
 
#define RELAYPIN1  12
#define RELAYPIN2  13
#define BUTTONPIN1 2
#define BUTTONPIN2 0
 
const char* ssid = WIFI_SSID;             // assign wifi ssid
const char* password = WIFI_PASSWORD;     // assign wifi password
 
WiFiClient client;                        // declare a  wificlient 
MicroGear microgear(client);              // declare microgear object

PieRelay *sw1, *sw2;

bool toPublishSW1State, toPublishSW2State;
 
void stateChange(int relayID, int type, int state) {
    Serial.print("relay "); Serial.print(relayID); Serial.print(" --> ");
    Serial.println(state?"ON":"OFF");
    if (relayID==1) toPublishSW1State = true;
    if (relayID==2) toPublishSW2State = true;
}
 
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    char m = *(char *)msg;
    msg[msglen] = '\0';
    Serial.print("incoming message: "); Serial.print((char *)topic);  
    Serial.print(" --> "); Serial.println((char*)msg);
 
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
 
/* A function to init WIFI
  * This is just a basic method to configure WIFI on ESP8266.                        
  * There are other methods that provide better user experience,
  * but a little bit more complicated.
 */
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
     
    initWiFi();                             // setup and connect to WiFi
 
    // add event listeners
    microgear.on(MESSAGE,onMsghandler);     // when new message arrives
    microgear.on(CONNECTED,onConnected);    // when connected to NETPIE
    microgear.init(KEY,SECRET);             // initial a micrigear variable
    microgear.connect(APPID);               // let a device connects to NETPIE
}
 
void loop() {
    if(WiFi.status()!=WL_CONNECTED){
        WiFi.disconnect();
        initWiFi();
    } else {
        if (microgear.connected()) {

          // to check if switch 1 need to update its status
            if (toPublishSW1State) {  
                // check the status and send message to the topic “/state1”
                microgear.publish("/state1",sw1->getState()?"1":"0");
                toPublishSW1State = false;
            }
            if (toPublishSW2State) {
                microgear.publish("/state2",sw2->getState()?"1":"0");
                toPublishSW2State = false;
            }
            microgear.loop();   
        }
        else {
            microgear.connect(APPID);
            delay(500);
        }
    }
}
