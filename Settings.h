#ifndef Settings_h
#define Settings_h

#include "Arduino.h"

#include "Secrets.h"                   // This file holds the AP WiFi name and password

// All the extra downloaded libraries go here
#include <WiFiNINA.h>                  // v1.8.11 WiFi Library - https://github.com/arduino-libraries/WiFiNINA
#include <WiFiUdp.h>                   // v1.8.11 WiFi UDP library - https://github.com/arduino-libraries/WiFiNINA
#include <NTPClient.h>                 // v3.2.0  NTP Client Library - https://github.com/arduino-libraries/NTPClient
#include <FlashStorage.h>              // v1.0.0  Use Flash Memory as EEPROM - Nano 33 has no EEPROM :( - https://github.com/cmaglie/FlashStorage
#include <RTClib.h>                    // v1.13.0 DS3231 RTC Library - https://github.com/adafruit/RTClib
#include <Adafruit_Sensor.h>           // v1.1.4  Required by BME280 - https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h>           // v2.1.4  BME280 Environmental Sensor Library - https://github.com/adafruit/Adafruit_BME280_Library
#include <Adafruit_VEML7700.h>         // v1.1.1  VEML7700 Light Sensor Library - https://github.com/adafruit/Adafruit_VEML7700

// Create a structures to store the WiFi credentials and all other settings into flash
// NOTE: This is lost when a new skecth is uploaded
typedef struct {bool valid; char flash_SSID[50]; char flash_PASS[50];} savedWiFi;
typedef struct {bool validBrightness; byte flashBrightness;} savedBrightness;
typedef struct {bool validNTP; bool flashNTP;} savedNTP;
typedef struct {bool validPIR; bool flashPIR;} savedPIR;
typedef struct {bool validLight; bool flashLight;} savedLight;
typedef struct {bool validUSB; bool flashUSB;} savedUSB;
typedef struct {bool validFont; bool flashFont;} savedFont;
typedef struct {bool validBackground; bool flashBackground;} savedBackground;
typedef struct {bool validUTCOffset; bool flashUTCOffset;} savedUTCOffset;

class Settings {
  public:
    Settings();

    // Changeable global variables

    const char* ntpServerName = "uk.pool.ntp.org"; // The NTP pool to query
    
    const unsigned long eventTime_Time = 100;      // Event time for time functions
    const unsigned long eventTime_Light = 1000;    // Event time for light functions
    const unsigned long eventTime_PIR = 2000;      // Event time for PIR functions
    const unsigned long eventTime_Server = 2000;   // Event time for Server functions

    unsigned long previousTime_Time = 0;           // Event start time for time functions
    unsigned long previousTime_Light = 10;         // Event start time for light functions
    unsigned long previousTime_PIR = 20;           // Event start time for PIR functions
    unsigned long previousTime_Server = 30;        // Event start time for Server functions

    const char* webTitle = "Arduino Nano 33 IoT Nixie Clock";
    const char* webName = "Living Room";
    const char* webFont = "Audiowide";

    // Global variables

    unsigned long currentTime;                     // Used by millis

    bool I2C_CODE[3];                              // Hold success status of I2C devices
    byte hour, minute, second;                     // Store RTC data
    byte day, month, year;                         // Store RTC data

    String flash_SSID;
    String flash_PASS;

    // Flash EEPROM initial save values
    byte flashBrightness = 100;                    // PWM HV5530 brightness level 1 - 255 (Note: 0 would turn off the Nixies)
    bool flashNTP = 1;                             // Enable/Disable NTP
    bool flashPIR = 1;                             // Enable/Disable PIR
    bool flashLight = 1;                           // Enable/Disable Light Sensor
    bool flashUSB = 1;                             // Set power supply mode, setting to 1 enables the 5v to 12v booster for the HV5530
    byte flashFont = 1;                            // The WebUI font
    byte flashBackground = 4;                      // The WebUI background
    int flashUTCOffset = 1;                        // UTC offset in hours

    const char* ssid;
    const char* pass;

    bool noSSID = 0;

    void rwSettings(byte setting, bool save);
    void debug(byte n);

    const char* fwVersion = "1.0";

  private:
};

#endif
