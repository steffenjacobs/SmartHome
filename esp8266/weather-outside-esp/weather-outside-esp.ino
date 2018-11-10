#include <Wire.h>
#include <BH1750.h>
#include "DHT.h"

#define DHTPIN 14 //GPIO 14 -> D5

DHT dht(DHTPIN, DHT22); 
BH1750 lightMeter; //connected to SCL/SDA -> D1/D2

void setup(){

  Serial.begin(9600);
  Wire.begin();
  lightMeter.begin();
  dht.begin();
  
  Serial.println("setup complete.");
}

void loop() {

  delay(2000);
  uint16_t lux = lightMeter.readLightLevel();
  float humidity = dht.readHumidity(); 
  float temperature = dht.readTemperature();
  
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.print("lx, temperature: ");
  Serial.print(temperature);
  Serial.print(" C, humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  delay(1000);
}
