#include <stdio.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define NONE         -10
#define STATECHANGE   1
#define MAXRELAY      4
#define BUTTONLOCKMS  400
#define EEPROM_OFFSET 128

class PieRelay {
  public :
    int channel;
    int state;
    int relayPin;
    int buttonPin;
    int lastPressed;
    PieRelay(int, int, int);
    void setCallback(int, void (*callback)(int, int, int));
    int getState();
    void setState(int);
    void (* cb_statechange)(int, int, int);

  private:
    
    //void onButtonPressed(void);

};
