
#include <RCSwitch.h>

#define led PB1
#define tx 3
#define sensorPin PB0
RCSwitch mySwitch = RCSwitch();

int lastValue = -1;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(sensorPin, INPUT);
  mySwitch.enableTransmit(tx);
  mySwitch.setProtocol(1);
}

void loop() {
  int state = digitalRead(sensorPin);
  if (state != lastValue) {
    lastValue = state;
    if (state) {
      mySwitch.switchOn("11110", "00010");
      digitalWrite(led, HIGH);
    } else {
      mySwitch.switchOff("11110", "00010");
      digitalWrite(led, LOW);
    }
  }
}

