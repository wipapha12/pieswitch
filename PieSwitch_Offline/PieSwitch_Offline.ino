/*  Simple PieRelay Offline version         */

#include "PieRelay.h"  // เรียกใช้งานไลบรารี่สำหรับควบคุม Relay

#define RELAYPIN1  13   // ตำแหน่ง PIN ของ Relay สวิตซ์ตัวที่ 1
#define RELAYPIN2  12   // ตำแหน่ง PIN ของ Relay สวิตซ์ตัวที่ 2
#define BUTTONPIN1 2    // ตำแหน่ง PIN ของปุ่มสวิตซ์ตัวที่ 1
#define BUTTONPIN2 0    // ตำแหน่ง PIN ของปุ่มสวิตซ์ตัวที่ 2

PieRelay *sw1, *sw2;    // ประกาศ object ของสวิตซ์ตัวที่ 1 และ 2

void stateChange(int relayID, int type, int state) {
  Serial.print("relay "); Serial.print(relayID); Serial.print(" --> "); Serial.println(state?"ON":"OFF");
}

void setup() {
  Serial.begin(115200);

  sw1 = new PieRelay(1, RELAYPIN1, BUTTONPIN1);    // กำหนด object ให้สวิตซ์ตัวที่ 1
  sw2 = new PieRelay(2, RELAYPIN2, BUTTONPIN2);    // กำหนด object ให้สวิตซ์ตัวที่ 2

  sw1->setCallback(STATECHANGE, stateChange);     // กำหนด callback ให้สวิตซ์ตัวที่ 1
  sw2->setCallback(STATECHANGE, stateChange);     // กำหนด callback ให้สวิตซ์ตัวที่ 2
}

void loop() {
  delay(250); 
}

