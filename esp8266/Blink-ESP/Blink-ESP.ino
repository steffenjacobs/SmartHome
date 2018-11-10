
int builtinLED = 2; //GPIO 2

void setup() {
  pinMode(builtinLED, OUTPUT);
}

void loop() {
  digitalWrite(builtinLED, HIGH);
  delay(1000);
  digitalWrite(builtinLED, LOW);
  delay(1000);
}
