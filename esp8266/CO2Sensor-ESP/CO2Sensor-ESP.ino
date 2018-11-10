
#include <SoftwareSerial.h>


#define INTERVAL 5000
#define MH_Z19_RX D7
#define MH_Z19_TX D6

long previousMillis = 0;

SoftwareSerial co2Serial(MH_Z19_RX, MH_Z19_TX); // define MH-Z19

#define IDX_mhz19   26


void setup() {
  Serial.begin(115200);


  // Démarrer la communication série avec le MH-Z19 - Start UART communication with MZ-Z19 sensor
  unsigned long previousMillis = millis();
  co2Serial.begin(9600); //Init sensor MH-Z19(14)
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < INTERVAL)
    return;

  previousMillis = currentMillis;
  Serial.print("Requesting CO2 concentration...");
  int ppm = readCO2();
  Serial.println("  PPM = " + String(ppm));

}

int readCO2() {
  // D'après le code original de | From original code https://github.com/jehy/arduino-esp8266-mh-z19-serial
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  // command to ask for data
  byte response[9]; // for answer

  co2Serial.write(cmd, 9); //request PPM CO2

  // The serial stream can get out of sync. The response starts with 0xff, try to resync.
  while (co2Serial.available() > 0 && (unsigned char)co2Serial.peek() != 0xFF) {
    co2Serial.read();
  }

  memset(response, 0, 9);
  co2Serial.readBytes(response, 9);

  if (response[1] != 0x86)
  {
    Serial.println("Invalid response from co2 sensor!");
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
    Serial.println("CRC error!");
    return -1;
  }
}
