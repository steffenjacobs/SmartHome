#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

RCSwitch mySwitch = RCSwitch();

const char* ssid = "network";
const char* password = "password";
const char* mqtt_server = "192.168.0.20";

const char* mqttChannel = "/device/433Mhz/receive";
const char* metaChannel = "/meta/devices";

const char* stationName = "433MhzReceiver";

const char* joinMsg = "{\"type\":\"JOIN\", \"name\":\"433MhzReceiver\", \"channel\": \"/device/433Mhz/receive\"}";
const char* activeMsg = "{\"type\":\"ACTIVE\", \"name\":\"433MhzReceiver\", \"channel\": \"/device/433Mhz/receive\"}";
const char* quitMsg = "{\"type\":\"QUIT\", \"name\":\"433MhzReceiver\"}";
const char* badJson = "{\"type\": \"error\", \"message\": \"Bad JSON\"}";

const char* queryString = "whois";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(2);  // Receiver on interrupt 2 -> GPIO 4 -> D2

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("setup complete.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (mySwitch.available()) {
    sendValue(mySwitch.getReceivedValue());
    mySwitch.resetAvailable();
  }
}

void sendValue(unsigned int value) {
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

