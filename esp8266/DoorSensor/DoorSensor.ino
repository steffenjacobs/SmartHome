#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const int sensorPin = 4; //D2 -> GPIO4

const char* ssid = "network";
const char* password = "password";
const char* mqtt_server = "192.168.0.20";

const char* mqttChannel = "/device/windowSensor/sensor1";
const char* metaChannel = "/meta/devices";

const char* stationName = "WindowSensor";

const char* joinMsg = "{\"type\":\"JOIN\", \"name\":\"WindowSensor\", \"channel\": \"/device/windowSensor/sensor1\"}";
const char* activeMsg = "{\"type\":\"ACTIVE\", \"name\":\"WindowSensor\", \"channel\": \"/device/windowSensor/sensor1\"}";
const char* quitMsg = "{\"type\":\"QUIT\", \"name\":\"WindowSensor\"}";
const char* badJson = "{\"type\": \"error\", \"message\": \"Bad JSON\"}";

const char* queryString = "whois";

WiFiClient espClient;
PubSubClient client(espClient);

int lastValue = -1;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  setupWifi();
  setupMqtt();
}

void setupWifi() {
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setupMqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loopWifi() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void loop() {
  loopWifi();
  int state = digitalRead(sensorPin);
  if (state != lastValue) {
    lastValue = state;
    sendValue(state);
    Serial.println(state);
  }
}


void sendValue(int value) {
  String msg = String("{\"value\": ");
  msg += value;
  msg += "}";

  char msgArr[20];
  msg.toCharArray(msgArr, 20);

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
