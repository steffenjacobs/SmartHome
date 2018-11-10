int reedDownPin = 5; //GPIO 5 - D1
int reedUpPin = 4; // GPIO 4 - D2

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "network";
const char* password = "password";

int status = 5;
//1: up
//2: down
//3: moving up
//4: moving down
//5: unknown

ESP8266WebServer server(80);


void handleRoot() {
  server.send(200, "text/plain", "{\"status\":\"running\", \"name\": \"shutter\"}");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void onStatusRequest() {
  char buf[16];
  sprintf(buf, "{\"status\":\"%d\"}", status);
  server.send(200, "text/plain", buf);
}

void onMoveDown() {
  if (status == 3 || status == 4 || status == 2) {
    return;
  }
  status = 4;
  digitalWrite(reedDownPin, 1);
  delay(600);
  digitalWrite(reedDownPin, 0);
  status = 2;
}

void onMoveUp() {
  if (status == 3 || status == 4 || status == 1) {
    return;
  }
  status = 3;
  digitalWrite(reedUpPin, 1);
  delay(600);
  digitalWrite(reedUpPin, 0);
  status = 1;
}

void setup(void) {
  Serial.begin(115200);

  pinMode(reedDownPin, OUTPUT);
  pinMode(reedUpPin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/shutter", []() {

    int stat = server.arg("stat").toFloat();
    if (stat == 1) {
      onMoveUp();
      server.send(200, "text/plain", "OK");
    }
    else if (stat == 2) {
      onMoveDown();
      server.send(200, "text/plain", "OK");
    }
    else {
      onStatusRequest();
    }
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

}

void loop(void) {
  server.handleClient();
}
