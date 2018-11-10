/*
  dmxChaser - a simple example for espDMX library
  Copyright (c) 2016, Matthew Tong
  https://github.com/mtongnz/espDMX

  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
  later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program.
  If not, see http://www.gnu.org/licenses/
*/

#include <ArduinoJson.h>
#include <espDMX.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "network";
const char* password = "password";
const char* mqtt_server = "192.168.0.20";

const char* mqttChannel = "/devices/dmx";
const char* mqttChannelO = "/devices/dmxO";
const char* mqttChannelBinary = "/devices/dmxB";
const char* metaChannel = "/meta/devices";

const char* stationName = "DMXController";

const char* joinMsg = "{\"type\":\"JOIN\", \"name\":\"DMXController\", \"channel\": \"/devices/dmx\"}";
const char* quitMsg = "{\"type\":\"QUIT\", \"name\":\"DMXController\"}";
const char* badInput = "{\"type\": \"error\", \"message\": \"Bad Input\"}";
const char* badJson = "{\"type\": \"error\", \"message\": \"Bad JSON\"}";

byte valsA[512] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                  };

byte valsB[512] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                  };



WiFiClient espClient;
PubSubClient client(espClient);

void resetDMX(int universe) {
  if (universe == 0) {
    for (int ch = 0; ch < 512; ch++) {
      valsA[ch] = 0;
    }
    dmxA.setChans(valsA, 512, 1);
  }
  else  if (universe == 1) {
    for (int ch = 0; ch < 512; ch++) {
      valsB[ch] = 0;
    }
    dmxB.setChans(valsA, 512, 1);
  }
  else {
    for (int ch = 0; ch < 512; ch++) {
      valsA[ch] = 0;
      valsB[ch] = 0;
    }
    dmxA.setChans(valsA, 512, 1);
    dmxB.setChans(valsB, 512, 1);
  }
}

void updateDMX(int channel, int value, int universe) {
  if (channel >= 0 && channel <= 512 && value >= 0 && value <= 255 ) {
    if (channel == 0) {
      resetDMX(universe);
    }
    else if (universe == 1) {
      valsB[channel - 1] = value;
      dmxB.setChans(valsB, 512, 1);
    }
    else if (universe == 0) {
      valsA[channel - 1] = value;
      dmxA.setChans(valsA, 512, 1);
    }
    else if (universe == 2) {
      //update both universes
      valsB[channel - 1] = value;
      dmxB.setChans(valsB, 512, 1);
      valsA[channel - 1] = value;
      dmxA.setChans(valsA, 512, 1);
    }
  }
  else {
    client.publish(mqttChannel, badInput);
  }
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

  if (assertEqual(mqttChannelBinary, topic) && length == 11) {

    String msg = String((char)payload[0]);
    for (int i = 1; i < length; i++) {
      msg += (char)payload[i];
    }   

    int chn = msg.substring(0, 3).toInt();
    int val = msg.substring(4, 7).toInt();
    int univ = msg.substring(8, 11).toInt();

    updateDMX(chn, val, univ);
    return;
  }

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

  //make sure, type is equal to "update"
  if (!assertEqual(parsed["type"], "update")) {
    if (assertEqual(parsed["type"], "whois")) {
      client.publish(metaChannel, joinMsg);
    }
    return;
  }

  int chn = parsed["chn"];
  int val = parsed["val"];
  int univ = parsed["univ"];

  updateDMX(chn, val, univ);

  //send response
  String msgr = String( "{\"type\": \"response\", \"chn\":");
  msgr += chn;
  msgr += ",\"val\":";
  msgr += val;
  msgr += ",\"univ\":";
  msgr += univ;
  msgr += "}";

  char arrr[50];
  msgr.toCharArray(arrr, 50);
  client.publish(mqttChannel, arrr);
}

void reconnect() {
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(stationName, metaChannel, 1, true, quitMsg)) {
      client.publish(metaChannel, joinMsg);
      client.subscribe(mqttChannel);
      client.subscribe(mqttChannelBinary);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Start dmxA, status LED on pin 12 with 1/5 intensity
  dmxA.begin(12, 51);

  // Start dmxB, status LED on pin 13 with 1/5 intensity
  dmxB.begin(13, 51);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
