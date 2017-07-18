/*  Simple PieRelay Offline version  */

#include "PieRelay.h"
 
#define RELAYPIN1  12		  // PIN of the relay 1
#define RELAYPIN2  13		  // PIN of the relay 2
#define BUTTONPIN1 2		  // PIN of the button 1
#define BUTTONPIN2 0		  // PIN of the button 2
PieRelay *sw1, *sw2;		  // switch variables

void stateChange(int relayID, int type, int state) {
    Serial.print("relay "); Serial.print(relayID); Serial.print(" --> ");    
    Serial.println(state?"ON":"OFF");
}
 
void setup() {
    Serial.begin(115200);

    // create a switch 1 which binds replay 1 with button 1
    sw1 = new PieRelay(1, RELAYPIN1, BUTTONPIN1);

    // create a switch 1 which binds replay 1 with button 1
    sw2 = new PieRelay(2, RELAYPIN2, BUTTONPIN2);

     // set callback function for switch 1
    sw1->setCallback(STATECHANGE, stateChange);


     // set callback function for switch 2
    sw2->setCallback(STATECHANGE, stateChange);
}
 
void loop() {
    delay(1000); 
}
