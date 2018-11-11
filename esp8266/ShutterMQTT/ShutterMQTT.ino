#include <ArduinoJson.h>
#include <espDMX.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//send to open: {\"type\": \"update\", \"stat\": 1}
//Send to close: {\"type\": \"update\", \"stat\": 2}

const char* ssid = "network";
const char* password = "password";
const char* mqtt_server = "192.168.0.20";

int reedDownPin = 5; //GPIO 5 - D1
int reedUpPin = 4; // GPIO 4 - D2

const int millisToOpen = 16500;
const int millisToClose = 12000;
//1: up
//2: down
//3: moving up
//4: moving down
//5: unknown
int status = 5;
int currentPosition = 0;

const char* mqttChannel = "/devices/shutter";
const char* metaChannel = "/meta/devices";

const char* stationName = "ShutterController";

const char* joinMsg = "{\"type\":\"JOIN\", \"name\":\"ShutterController\", \"channel\": \"/devices/shutter\"}";
const char* activeMsg = "{\"type\":\"ACTIVE\", \"name\":\"ShutterController\", \"channel\": \"/devices/shutter\"}";
const char* quitMsg = "{\"type\":\"QUIT\", \"name\":\"ShutterController\"}";
const char* badInput = "{\"type\": \"error\", \"message\": \"Bad Input\"}";
const char* badJson = "{\"type\": \"error\", \"message\": \"Bad JSON\"}";

const char* moveUpMsg = "{\"type\": \"response\",\"result\":\"moving up\"}";
const char* moveDownMsg = "{\"type\": \"response\",\"result\":\"moving down\"}";

WiFiClient espClient;
PubSubClient client(espClient);

void onMoveDown() {
  if (status == 3 || status == 4 ) {
    return;
  }
  status = 4;
  digitalWrite(reedDownPin, 1);
  delay(600);
  digitalWrite(reedDownPin, 0);
  status = 2;
}

void onMoveUp() {
  if (status == 3 || status == 4 ) {
    return;
  }
  status = 3;
  digitalWrite(reedUpPin, 1);
  delay(600);
  digitalWrite(reedUpPin, 0);
  status = 1;
}

/**move to position between 0 and 100 where 0 is fully open and 100 is fully closed*/
void onMoveToPosition(int newPosition) {
  if (status == 3 || status == 4 || currentPosition == newPosition) {
    /*shutter is moving or already in correct position*/
    return;
  }
  boolean up = newPosition < currentPosition;

  if (up) {
    client.publish(mqttChannel, moveUpMsg);
    status = 3;
    unsigned long startTime = millis();
    digitalWrite(reedUpPin, 1);
    delay(600);
    digitalWrite(reedUpPin, 0);

    while (millis() < startTime + (millisToOpen * newPosition / (double)100)) {
      delay(10);
    }

    digitalWrite(reedUpPin, 1);
    delay(600);
    digitalWrite(reedUpPin, 0);
    status = 2;
  }
  else {
    client.publish(mqttChannel, moveDownMsg);
    status = 4;
    unsigned long startTime = millis();
    digitalWrite(reedDownPin, 1);
    delay(600);
    digitalWrite(reedDownPin, 0);

    while (millis() < startTime + (millisToClose * newPosition / (double)100)) {
      delay(10);
    }

    digitalWrite(reedDownPin, 1);
    delay(600);
    digitalWrite(reedDownPin, 0);
    status = 2;
  }
  currentPosition = newPosition;
  char buf[40];
  strcpy(buf, "{\"type\": \"response\",\"position\":");
  sprintf(buf, "%02i", currentPosition);
  strcat(buf, "\"}");
  client.publish(mqttChannel, buf);
}

int assertEqual(const char* first, char* second) {
  if (sizeof(first) != sizeof(second)) {
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

void callback(char* topic, byte* payload, unsigned int length) {
  return;
  String msg = String((char)payload[0]);
  for (int i = 1; i < length; i++) {
    msg += (char)payload[i];
  }


  StaticJsonBuffer<300> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(msg); //Parse message

  if (!parsed.success()) {   //Check for errors in parsing
    client.publish(topic, badJson);
    return;
  }

  //make sure, type is equal to "update"
  if (!assertEqual(parsed["type"], "update")) {
    if (assertEqual(parsed["type"], "whois")) {
      client.publish(metaChannel, activeMsg);
    }
    return;
  }

  if (assertEqual(parsed["type"], "position")) {
    onMoveToPosition(parsed["position"]);
    return;
  }

  int stat = parsed["stat"];

  if (stat == 1) {
    onMoveUp();
    client.publish(mqttChannel, moveUpMsg);
  }
  else if (stat == 2) {
    onMoveDown();
    client.publish(mqttChannel, moveDownMsg);
  }
}

void reconnect() {
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(stationName, metaChannel, 1, true, quitMsg)) {
      client.publish(metaChannel, joinMsg);
      client.subscribe(mqttChannel);
      client.subscribe(metaChannel);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(void) {

  pinMode(reedDownPin, OUTPUT);
  pinMode(reedUpPin, OUTPUT);

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(void) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
