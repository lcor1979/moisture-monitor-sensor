#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#define VERBOSE_MODE                   true  // If set to 'true' enables debug output

/*=========================================================================
    APPLICATION SETTINGS

      FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
     
                                Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                                running this at least once is a good idea.
     
                                When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                                Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
         
                                Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.7.7"
#define MODE_LED_BEHAVIOUR          "SPI"
/*=========================================================================*/

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// The service and characteristic index information
int32_t gattServiceId;
int32_t gattNewMeasureCharId;
int32_t gattTemperatureCharId;
int32_t gattMoistureCharId;
int32_t gattBatteryCharId;

void setup(void)
{
  //remove these 2 lines if not debugging - nothing will start until you open the serial window
  while (!Serial); // required for Flora & Micro
  delay(500);

  boolean success;

  Serial.begin(115200);
  Serial.println(F("Adafruit Custom GATT Service Example"));
  Serial.println(F("---------------------------------------------------"));

  randomSeed(micros());

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ) {
    error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Add the Custom GATT Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the Custom GATT Service definition: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x1900"), &gattServiceId);
  if (! success) {
    error(F("Could not add Custom GATT service"));
  }
  else {
    Serial.print( F("Gatt Service ID is :") ) ;
    Serial.println( gattServiceId ) ;
  }

  Serial.println(F("Adding the NewMeasure characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=1901,PROPERTIES=0x10,MIN_LEN=1, MAX_LEN=1, DESCRIPTION=New measure available, VALUE=0x00"), &gattNewMeasureCharId);
  if (! success) {
    error(F("Could not add NewMeasure characteristic"));
  }
  else {
    Serial.print( F("NewMeasure Char ID is :") ) ;
    Serial.println( gattNewMeasureCharId ) ;
  }

  Serial.println(F("Adding the Temperature characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=1902,PROPERTIES=0x02,MIN_LEN=1, MAX_LEN=10, DESCRIPTION=Temperature value, VALUE=0.0"), &gattTemperatureCharId);
  if (! success) {
    error(F("Could not add Temperature characteristic"));
  }
  else {
    Serial.print( F("Temperature Char ID is :") ) ;
    Serial.println( gattTemperatureCharId ) ;
  }

  Serial.println(F("Adding the Moisture characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=1903,PROPERTIES=0x02,MIN_LEN=1, MAX_LEN=10, DESCRIPTION=Moisture value, VALUE=0.0"), &gattMoistureCharId);
  if (! success) {
    error(F("Could not add Moisture characteristic"));
  }
  else {
    Serial.print( F("Moisture Char ID is :") ) ;
    Serial.println( gattMoistureCharId ) ;
  }

  Serial.println(F("Adding the Battery characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=1904,PROPERTIES=0x02,MIN_LEN=1, MAX_LEN=10, DESCRIPTION=Battery value, VALUE=0.0"), &gattBatteryCharId);
  if (! success) {
    error(F("Could not add Battery characteristic"));
  }
  else {
    Serial.print( F("Battery Char ID is :") ) ;
    Serial.println( gattBatteryCharId ) ;
  }

  /* Set device name */
  Serial.println(F("Setting device name: "));
  ble.sendCommandCheckOK( F("AT+GAPDEVNAME=MoistureSensor") );

  /* Add the Custom GATT Service to the advertising data */
  //0x1312 from AT+GATTLIST - 16 bit svc id
  Serial.println(F("Adding Custom GATT Service UUID to the advertising payload: "));
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-03-02-00-19") );

  /* Reset the device for the new service setting changes to take effect */
  Serial.println(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  ble.sendCommandCheckOK( F("AT+GATTLIST") );

  Serial.println();
}

void loop(void)
{

}
