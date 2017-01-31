# moisture-monitor-sensor
Wireless bluetooth low energy temperature / humidity sensor based on Feather 32u4 BLE with AM2302 (DHT22) sensor

https://learn.adafruit.com/adafruit-feather-32u4-bluefruit-le/
https://learn.adafruit.com/dht

The sensor is totaly wireless (use a LIPO battery which can be charged witgh the Feather's USB port).

(pictures will be added later)

It uses GATT attributes to publish the measures.

https://learn.adafruit.com/introduction-to-bluetooth-low-energy/gatt

Sent attributes are :
- One notify attribute indicating a new measure is available
- Three read attributes for : battery (in percent), humidity (in percent), temperature (in °C)

The code contains two projects :

- One to initialize the GATT attributes and GAP informations. It should be ran only once on the device.
- One to run the logic which should be uploaded to the Feather after the initialisation has been done.

Code could be run in Arduino IDE : https://www.arduino.cc/en/Main/Software
