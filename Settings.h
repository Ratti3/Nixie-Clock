#ifndef Settings_h
#define Settings_h

#include "Arduino.h"

#include <Wire.h>
#include <SPI.h>
#include <WiFiNINA.h>                  // v1.8.8 WiFi library - https://github.com/arduino-libraries/WiFiNINA
#include <WiFiUdp.h>                   // v1.8.8 WiFi UDP library - https://github.com/arduino-libraries/WiFiNINA
#include <NTPClient.h>                 // v3.2.0 NTP Client Library - https://github.com/arduino-libraries/NTPClient
#include <FlashStorage.h>              // v1.0.0 Save settings to Flash memory - https://github.com/cmaglie/FlashStorage
#include "Secrets.h"                   // This file holds the AP WiFi name and password
#include <RTClib.h>                    // v1.13.0 DS3231 RTC - https://github.com/adafruit/RTClib
#include <Adafruit_Sensor.h>           // v1.1.4 Required by BME280 - https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h>           // v2.1.2 BME280 Environmental Sensor - https://github.com/adafruit/Adafruit_BME280_Library
#include <Adafruit_VEML7700.h>         // v1.1.1 VEML7700 Light Sensor https://github.com/adafruit/Adafruit_VEML7700

typedef struct {
  boolean valid;
  char flash_SSID[50];
  char flash_PASS[50];
} wifiCreds;


class Settings {
  public:
    Settings();

    // Changeable global variables

    int utcOffset = 3600;                          // UTC offset in seconds

    const char* ntpServerName = "uk.pool.ntp.org"; // The NTP pool to query
    byte try_count = 15;                           // Number of packet send attempts, 1 try_count = 2 seconds between each packet
    
    const unsigned long eventTime_Time = 100;      // Event time for time functions
    const unsigned long eventTime_Light = 1000;    // Event time for light functions
    const unsigned long eventTime_PIR = 2000;      // Event time for PIR functions
    const unsigned long eventTime_Server = 2000;   // Event time for Server functions

    unsigned long previousTime_Time = 0;           // Event start time for time functions
    unsigned long previousTime_Light = 10;         // Event start time for light functions
    unsigned long previousTime_PIR = 20;           // Event start time for PIR functions
    unsigned long previousTime_Server = 30;        // Event start time for Server functions

    // Global variables

    unsigned long currentTime;                     // Used by millis

    bool I2C_CODE[3];                              // Hold success status of I2C devices
    int hour, minute, second;                      // Store RTC data
    int day, month, year;                          // Store RTC data

    String flash_SSID;
    String flash_PASS;

    const char* ssid;
    const char* pass;

    bool noSSID = 0;

    void wifiCredentials(bool save);
    void debug(byte n);

  private:
};

#endif
