#include <Wire.h>
#include <BH1750.h>
#include "DHT.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 14 //GPIO 14 -> D5

DHT dht(DHTPIN, DHT22);
BH1750 lightMeter; //connected to SCL/SDA -> D1/D2

const char* ssid = "network";
const char* password = "password";
const char* mqtt_server = "192.168.0.20";

const char* mqttChannel = "/sensor/weather/outside";
const char* metaChannel = "/meta/sensors";

const char* stationName = "WeatherOutside";

const char* joinMsg = "{\"type\":\"JOIN\", \"name\":\"WeatherOutside\", \"channel\": \"/sensor/weather/outside\"}";
const char* activeMsg = "{\"type\":\"ACTIVE\", \"name\":\"WeatherOutside\", \"channel\": \"/sensor/weather/outside\"}";
const char* quitMsg = "{\"type\":\"QUIT\", \"name\":\"WeatherOutside\"}";
const char* badJson = "{\"type\": \"error\", \"message\": \"Bad JSON\"}";

const char* queryString = "whois";

const long delayMeasurementsMillis = 2000;

unsigned long lastTime = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  Serial.begin(9600);
  Wire.begin();
  lightMeter.begin();
  dht.begin();

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("setup complete.");
}

void loop(void) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  takeMeasurement();
}

void takeMeasurement() {
  unsigned long now = millis();
  if (now - lastTime < delayMeasurementsMillis) {
    return;
  }
  lastTime = now;
  uint16_t lux = lightMeter.readLightLevel();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  String msg = String("{\"light\": ");
  msg += lux;
  msg += ", \"temperature\": ";
  msg += temperature;
  msg += ", \"humidity\": ";
  msg += humidity;
  msg += "}";

  char msgArr[60];
  msg.toCharArray(msgArr, 60);

  Serial.println(msgArr);
  client.publish(mqttChannel, msgArr);
}
void reconnect() {
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(stationName, metaChannel, 1, true, quitMsg)) {
      client.publish(metaChannel, joinMsg);
      client.subscribe(metaChannel);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = String((char)payload[0]);
  for (int i = 1; i < length; i++) {
    msg += (char)payload[i];
  }

  StaticJsonBuffer<300> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(msg); //Parse message

  if (!parsed.success()) {   //Check for errors in parsing
    client.publish(mqttChannel, badJson);
    return;
  }

  if (assertEqual(parsed["type"], queryString)) {
    client.publish(metaChannel, activeMsg);
  }
}

int assertEqual(const char* first, const char* second) {

  if (!first || !*first) {
    return 0;
  }

  if (strlen(first) != strlen(second)) {
    return 0;
  }

  int len = sizeof(first);
  for (int i = 0; i < len; i++) {
    if (first[i] != second[i]) {
      return 0;
    }
  }

  return 1;
}
