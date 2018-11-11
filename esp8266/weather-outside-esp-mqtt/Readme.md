Reads the temperature, humidity and light level in the air with a DHT22 sensor (temperature, humidity) and a BH1750 sensor (light level). The program publishes the results on an MQTT channel as a JSON String.  
  
Measurement unit: light: `lx`, temperature: `degree celsius`, humidity: `percent`  
Whois channel: `/meta/sensors`  
Message channel: `/sensor/weather/outside`  
Example message: `{"light": 1232, "temperature": 15.60, "humidity": 62.50}` 
  
Pinout:  
DHT --> D5 (GPIO 14)  
BH 1750 SCL --> D1
BH 1750 SDA --> D2
