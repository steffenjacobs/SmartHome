Emits a signal on D1/D2 to control a shutter and make it move up or down. Integrates with MQTT and receives command and publishes update over a specific MQTT channel.

Whois channel: `/meta/devices` 
Message channel: `/devices/shutter`  
  
Example message for command 'move down': `{"type": "update", "stat": 2}`  
Example message for command 'move up': `{"type": "update", "stat": 1}`  
Example message for confirm 'move down': `{"type": "response","result":"moving down"}`  
Example message for confirm 'move up': `{"type": "response","result":"moving up"}`  
  
Pinout:  
shutter down pin --> D1 (GPIO 5)  
shutter up pin --> D2 (GPIO 4)  
