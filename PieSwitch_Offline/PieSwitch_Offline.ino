/*  Simple PieRelay Offline version         */

#include "PieRelay.h"

#define RELAYPIN1  13
#define RELAYPIN2  12
#define BUTTONPIN1 2
#define BUTTONPIN2 0

PieRelay *sw1, *sw2;

void stateChange(int relayID, int type, int state) {
  Serial.print("relay "); Serial.print(relayID); Serial.print(" --> "); Serial.println(state?"ON":"OFF");
}

void setup() {
  Serial.begin(115200);

  sw1 = new PieRelay(1, RELAYPIN1, BUTTONPIN1);
  sw2 = new PieRelay(2, RELAYPIN2, BUTTONPIN2);

  sw1->setCallback(STATECHANGE, stateChange);
  sw2->setCallback(STATECHANGE, stateChange);  
}

void loop() {
  delay(250); 
}
