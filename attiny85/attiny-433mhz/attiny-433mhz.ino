
#include <RCSwitch.h>

#define led PB1
#define tx 3
RCSwitch mySwitch = RCSwitch();

void setup() {
  pinMode(led, OUTPUT);
  mySwitch.enableTransmit(tx);
  mySwitch.setProtocol(1);
}

void loop() {

  mySwitch.switchOn("11111", "00010");
  digitalWrite(led, HIGH);
  delay(500);
  mySwitch.switchOff("11111", "00010");
  digitalWrite(led, LOW);
  delay(500);
}

