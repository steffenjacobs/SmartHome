
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "network";
const char* password = "password";
const char* mqtt_server = "192.168.0.20";

const char* mqttChannel = "/sensor/co2Inside";
const char* metaChannel = "/meta/sensors";

const char* stationName = "Co2Inside";

const char* joinMsg = "{\"type\":\"JOIN\", \"name\":\"Co2Inside\", \"channel\": \"/sensor/co2Inside\"}";
const char* activeMsg = "{\"type\":\"ACTIVE\", \"name\":\"Co2Inside\", \"channel\": \"/sensor/co2Inside\"}";
const char* quitMsg = "{\"type\":\"QUIT\", \"name\":\"Co2Inside\"}";
const char* badJson = "{\"type\": \"error\", \"message\": \"Bad JSON\"}";

const char* queryString = "whois";

const long delayMeasurementsMillis = 2000;

unsigned long lastTime = 0;

WiFiClient espClient;
PubSubClient client(espClient);

#define MH_Z19_RX D7
#define MH_Z19_TX D8

SoftwareSerial co2Serial(MH_Z19_RX, MH_Z19_TX); // define MH-Z19

void setup() {
  unsigned long previousMillis = millis();
  co2Serial.begin(9600);

  setupCommunication();
}

void setupCommunication() {
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

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

  int ppm = readCO2();

  String msg = String("{\"co2\": ");
  msg += ppm;
  msg += "}";

  char msgArr[30];
  msg.toCharArray(msgArr, 30);

  client.publish(mqttChannel, msgArr);
}

int readCO2() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  byte response[9]; // for answer

  co2Serial.write(cmd, 9); //request PPM CO2

  while (co2Serial.available() > 0 && (unsigned char)co2Serial.peek() != 0xFF) {
    co2Serial.read();
  }

  memset(response, 0, 9);
  co2Serial.readBytes(response, 9);

  if (response[1] != 0x86)
  {
    //Serial.println("Invalid response from co2 sensor!");
    return -1;
  }

  byte crc = 0;
  for (int i = 1; i < 8; i++) {
    crc += response[i];
  }
  crc = 255 - crc + 1;

  if (response[8] == crc) {
    int responseHigh = (int) response[2];
    int responseLow = (int) response[3];
    int ppm = (256 * responseHigh) + responseLow;
    return ppm;
  } else {
    //Serial.println("CRC error!");
    return -1;
  }
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
