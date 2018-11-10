Reads the CO2 concentration in the air from a MH Z19 Sensor and publishes it on an MQTT channel as a JSON Strings.  
  
Measurement unit: `ppm`  
Whois channel: `/meta/sensors`  
Message channel: `/sensor/co2Inside`  
Example message: `{"co2": 522}` 

Pinout:  
RX --> D7  
TX --> D8
