/*
  Project: Wifi controlled LED matrix display
  NodeMCU pins    -> EasyMatrix pins
  MOSI-D7-GPIO13  -> DIN
  CLK-D5-GPIO14   -> Clk
  GPIO0-D3        -> LOAD

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#define SSID "network"
#define PASS "password"

String form =
  "<p>"
  "<center>"
  "<h1>Display Control Server</h1>"
  "<form action='data'><p>Message: <input type='text' name='msg' size=100 autofocus>"
  "<p>Intensity (0-15): <input type='text' name='int' size=2 autofocus>"
  "<p><input type='submit' value='Submit'></form></center>";

ESP8266WebServer server(80);                             // HTTP server will listen at port 80
long period;
int offset = 1, refresh = 0, renderText = 1;
int pinCS = 0; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4;
const int numberOfVerticalDisplays = 1;
String currentMsg;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 50; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels
boolean imageBuffer[numberOfHorizontalDisplays * 8][numberOfVerticalDisplays * 8];

/*
  handles the messages coming from the webbrowser, restores a few special characters and
  constructs the strings that can be sent to the oled display
*/
void handle_msg() {
  renderText = 1;
  matrix.fillScreen(LOW);
  server.send(200, "text/html", "");
  // Display msg on Oled
  String msg = server.arg("msg");
  Serial.println(msg);
  currentMsg = msg;
  // Restore special characters that are misformed to %char by the client browser
  currentMsg.replace("+", " ");
  currentMsg.replace("%21", "!");
  currentMsg.replace("%22", "");
  currentMsg.replace("%23", "#");
  currentMsg.replace("%24", "$");
  currentMsg.replace("%25", "%");
  currentMsg.replace("%26", "&");
  currentMsg.replace("%27", "'");
  currentMsg.replace("%28", "(");
  currentMsg.replace("%29", ")");
  currentMsg.replace("%2A", "*");
  currentMsg.replace("%2B", "+");
  currentMsg.replace("%2C", ",");
  currentMsg.replace("%2F", "/");
  currentMsg.replace("%3A", ":");
  currentMsg.replace("%3B", ";");
  currentMsg.replace("%3C", "<");
  currentMsg.replace("%3D", "=");
  currentMsg.replace("%3E", ">");
  currentMsg.replace("%3F", "?");
  currentMsg.replace("%40", "@");
}

void handle_intensity() {
  String intensity = server.arg("int");

  Serial.println(intensity);

  matrix.setIntensity(intensity.toFloat());
  server.send(200, "text/html", "");
}

void handle_data() {
  handle_intensity();
  handle_msg();
}

void handle_updateCol() {
  renderText = 0;
  //column of the panel
  int panelX = server.arg("pX").toFloat();
  //row of the panel
  int panelY = server.arg("pY").toFloat();
  //affected column on panel(x,y)
  int panelCol = server.arg("pCol").toFloat();
  //8 bit for each pixel in the affected column
  byte data = server.arg("data").toFloat();

  Serial.print("x: ");
  Serial.print(panelX);
  Serial.print(" y: ");
  Serial.print(panelY);
  Serial.print(" column: ");
  Serial.print(panelCol);
  Serial.print(" data: ");
  Serial.println(data);

  for (int i  = 0; i < 8; i++) {
    imageBuffer[panelX * 8][panelY * 8 + i] = data & pow2(i);
  }

  Serial.println("drawn successful.");
  server.send(200, "text/html", "");
}

int pow2(int p) {
  if (p == 0) {
    return 1;
  }
  else {
    int result = 1;
    for (int i = 0; i < p; i++) {
      result = result * 2;
    }
    return result;
  }
}

void setup(void) {
  matrix.setIntensity(5); // Use a value between 0 and 15 for brightness

  //rotate matrices
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);

  // Adjust to your own needs
  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>

  //ESP.wdtDisable();                               // used to debug, disable wachdog timer,
  Serial.begin(115200);                           // full speed to monitor

  WiFi.begin(SSID, PASS);                         // Connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {         // Wait for connection
    delay(500);
    Serial.print(".");
  }
  // Set up the endpoints for HTTP server,  Endpoints can be written as inline functions:
  server.on("/", []() {
    server.send(200, "text/html", form);
  });

  server.on("/msg", handle_msg);                  // And as regular external functions:
  server.on("/int", handle_intensity);
  server.on("/data", handle_data);
  server.on("/col", handle_updateCol);
  server.begin();                                 // Start the server

  Serial.print("SSID : ");                        // prints SSID in monitor
  Serial.println(SSID);                           // to monitor

  char result[16];
  sprintf(result, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  Serial.println();
  Serial.println(result);
  currentMsg = result;
  Serial.println("WebServer ready!   ");

  Serial.println(WiFi.localIP());                 // Serial monitor prints localIP
  Serial.print(analogRead(A0));
}

void loop(void) {
  Serial.println("_");

  if (renderText) {
    for ( int i = 0 ; i < width * currentMsg.length() + matrix.width() - 1 - spacer; i++ ) {
      server.handleClient();                        // checks for incoming messages
      if (refresh) i = 0;
      refresh = 0;
      matrix.fillScreen(LOW);

      int letter = i / width;
      int x = (matrix.width() - 1) - i % width;
      int y = (matrix.height() - 8) / 2; // center the text vertically

      while ( x + width - spacer >= 0 && letter >= 0 ) {
        if ( letter < currentMsg.length() ) {
          matrix.drawChar(x, y, currentMsg[letter], HIGH, LOW, 1);
        }

        letter--;
        x -= width;
      }

      matrix.write(); // Send bitmap to display

      delay(wait);
    }
  }
  else {
    for (int x = 0; x < sizeof(imageBuffer); x++) {
      for (int y = 0; y < sizeof(imageBuffer[x]); y++) {
        matrix.drawPixel(x, y, imageBuffer[x][y]);
        Serial.print(imageBuffer[x][y] != 0);
        Serial.print(" ");
      }
      Serial.println("");
    }
    Serial.println("___________________________");
    matrix.write();
  }
}

