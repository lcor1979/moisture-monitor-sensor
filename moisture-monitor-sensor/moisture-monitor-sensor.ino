#include <String.h>
#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/*=========================================================================
    APPLICATION SETTINGS

    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
#define MINIMUM_FIRMWARE_VERSION    "0.7.7"
#define MODE_LED_BEHAVIOUR          "SPI"
/*=========================================================================*/

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* GATT ATTR IDS */
const String NEW_MEASURE_CHAR = "1";
const String TEMPERATURE_CHAR = "2";
const String MOISTURE_CHAR = "3";
const String BATTERY_CHAR = "4";

/* Red led PIN */
#define LED_PIN 13

/* Init DHT sensor */
#include "DHT.h"

#define DHTPIN A0    // DHT22 PIN
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

/* Enable to go in low power mode */
#define WATCHDOG_ENABLE 1

const unsigned long SLEEP_TIME_WAIT_FOR_CONNECTION = 300000;
const unsigned long SLEEP_TIME_STAY_UP = 180000;
const unsigned long SLEEP_TIME_LOW_POWER = 600000;

#define VBATPIN A9   // Battery PIN
#define BAT_MIN 3.2  // When Battery reach this voltage, it is low
#define BAT_MAX 3.7  // When Battery reach this voltage, it is full (it can be up to 4.2 but should stay around 3.7)

// A small helper
void error(String err) {
  Serial.print(F("Error: "));
  Serial.println(err);
}

boolean initBluetooth() {
  /* Initialise the module */
  Serial.println(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    return false;
  }
  else {
    Serial.println( F("OK!") );
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  ble.verbose(false);  // debug info is a little annoying after this point!

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  Serial.println(F("******************************"));

  return true;
}

void putDeviceInFailMode() {
  while (1) {
    blink(2000, 500);
  }
}

unsigned long blink(unsigned long highDelay, unsigned long lowDelay) {
  digitalWrite(LED_PIN, HIGH);
  delay(highDelay);
  digitalWrite(LED_PIN, LOW);
  delay(lowDelay);

  return highDelay + lowDelay;
}

void setup() {
  delay(2000);

  Serial.begin(115200);

  Serial.println(F("Initialising DHT module: "));
  dht.begin();

  pinMode(LED_PIN, OUTPUT);

  if (!initBluetooth()) {
    putDeviceInFailMode();
  }

}


float readBatteryVoltage() {
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  return measuredvbat;
}

float getBatteryInPercent() {
  float batteryVoltage = readBatteryVoltage();

  return min(((batteryVoltage - BAT_MIN) / (BAT_MAX - BAT_MIN)) * 100.0, 100.0);
}

String floatToString(float f) {
  if (isnan(f)) {
    return F("NaN");
  }
  else {
    return String(f, 2);
  }
}

void setGattAttrValue(String attrId, String value) {
  String gattCommand = "AT+GATTCHAR=" + attrId + "," + value;
  ble.println( gattCommand );
  if ( !ble.waitForOK() )
  {
    error("Gatt attribute update failed for attrId " + attrId + " and value " + value);
  }
  else {
    Serial.println("Gatt attribute " + attrId + " updated to value " + value);
  }
}

void loop() {
  unsigned long timeSpent = 0;

  digitalWrite(LED_PIN, HIGH);

  String battery = floatToString(getBatteryInPercent());
  Serial.println("Battery: " + battery);

  String humidity = floatToString(dht.readHumidity());
  Serial.println("Humidity: " + humidity);

  String temperature = floatToString(dht.readTemperature());
  Serial.println("Temperature: " + temperature);

  setGattAttrValue(NEW_MEASURE_CHAR, "1");
  setGattAttrValue(BATTERY_CHAR, battery);
  setGattAttrValue(MOISTURE_CHAR, humidity);
  setGattAttrValue(TEMPERATURE_CHAR, temperature);

  // Long blink for SLEEP_TIME_STAY_UP to indicate a new measure is ready
  timeSpent = 0;
  while (timeSpent < SLEEP_TIME_STAY_UP) {
    timeSpent += blink(2000, 2000);
  }

  digitalWrite(LED_PIN, LOW);
  delay(500);

  if ( WATCHDOG_ENABLE ) {
    // If enabled, go in sleep mode
    timeSpent = 0;
    while (timeSpent < SLEEP_TIME_LOW_POWER) {
      timeSpent += Watchdog.sleep();
    }
  }
  else {
    delay(SLEEP_TIME_LOW_POWER);
  }
}
