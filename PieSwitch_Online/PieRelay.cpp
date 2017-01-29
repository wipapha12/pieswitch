#include "PieRelay.h"

int swcount = 0;
PieRelay *sw[MAXRELAY];
 
void onButtonPressed(void) {
  int i;
  for (i=0;i<swcount;i++) {
    if (millis() - sw[i]->lastPressed > BUTTONLOCKMS) {
      if (digitalRead(sw[i]->buttonPin) == HIGH) {
        sw[i]->lastPressed = millis();
        sw[i]->state = sw[i]->state?0:1;
        EEPROM.write(EEPROM_OFFSET+sw[i]->channel, sw[i]->state);
        EEPROM.commit();
        digitalWrite(sw[i]->relayPin, sw[i]->state);
        if (sw[i]->cb_statechange) {
          sw[i]->cb_statechange(sw[i]->channel, STATECHANGE, sw[i]->state);
        }
        return;
      }
    }
  }
}


int PieRelay::getState() {
  return this->state;
}

void PieRelay::setState(int state) {
  if (this->state != state) {
    this->state = state;
    EEPROM.write(EEPROM_OFFSET+this->channel, this->state);
    EEPROM.commit();  
    digitalWrite(this->relayPin, this->state);
    this->cb_statechange(this->channel, STATECHANGE, this->state);
  }
}

PieRelay::PieRelay(int channel, int relayPin, int buttonPin) {
  if (swcount >= MAXRELAY) return;  

  sw[swcount++] = this;  
  this->channel = channel;
  this->relayPin = relayPin;
  this->buttonPin = buttonPin;
  this->lastPressed = 0;
  this->cb_statechange = NULL;

  EEPROM.begin(512);

  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  this->state = EEPROM.read(EEPROM_OFFSET+this->channel);
  digitalWrite(this->relayPin, this->state);
  attachInterrupt(this->buttonPin, onButtonPressed, FALLING);
}

void PieRelay::setCallback(int cbtype, void (* callback)(int, int, int)  ) {
  switch (cbtype) {
    case STATECHANGE :  this->cb_statechange = callback;
              break;
  }
}



