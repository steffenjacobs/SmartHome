#include <DHT.h>
#define DHTPIN 4 

DHT dht(DHTPIN, DHT22); 
float h=0;
float t=0;

void setup() 
{
  pinMode(1, INPUT);
  dht.begin();
  Serial.begin(9600);
  Serial.println("setup complete");
}

void loop() {
  delay(3000);
  h = dht.readHumidity(); 
  t = dht.readTemperature();
  Serial.print("temperature: ");
  Serial.print(t);
  Serial.print("C, humidity: ");
  Serial.print(h);
  Serial.println("%");
  delay(500);
}
