#include <Manchester.h>
#define RX_PIN 4 //= pin D2
#define LED_PIN 5

void setup() {
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceive();
  Serial.begin(9600);
  Serial.print("started.");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial.println("entering loop...");
  if (man.receiveComplete()) {
    digitalWrite(LED_PIN, HIGH);
    uint16_t m = man.getMessage();
    Serial.print("Received: ");
    Serial.println(m);
    digitalWrite(LED_PIN, LOW);
    man.beginReceive(); 
   }
}
