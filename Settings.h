#ifndef Settings_h
#define Settings_h

#include "Arduino.h"
#include "Globals.h"

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
#include <AceButton.h>                 // v1.9.0  Button library - https://github.com/bxparks/AceButton

using namespace ace_button;

// Create a structures to store the WiFi credentials and all other settings into flash
// NOTE: This is lost when a new skecth is uploaded
typedef struct {bool valid; char flash_SSID[50]; char flash_PASS[50];} savedWiFi;
typedef struct {bool validBrightness; byte flashBrightness;} savedBrightness;
typedef struct {bool validNTP; bool flashNTP;} savedNTP;
typedef struct {bool validNTPPool; byte flashNTPPool;} savedNTPPool;
typedef struct {bool validOnOffHour; byte flashOnHour; byte flashOffHour;} savedOnOffHour;
typedef struct {bool validPIR; bool flashPIR;} savedPIR;
typedef struct {bool validLight; bool flashLight;} savedLight;
typedef struct {bool validUSB; bool flashUSB;} savedUSB;
typedef struct {bool validFont; byte flashFont;} savedFont;
typedef struct {bool validBackground; byte flashBackground;} savedBackground;
typedef struct {bool validUTCOffset; int flashUTCOffset;} savedUTCOffset;
typedef struct {bool validColon; byte flashColon;} savedColon;
typedef struct {bool validLED; byte flashLED1; byte flashLED2; byte flashLED3;} savedLED;
typedef struct {bool validSpin; byte flashSpin;} savedSpin;
typedef struct {bool validLux; byte flashLux;} savedLux;
typedef struct {bool validName; char flashTitle[50]; char flashName[50];} savedName;

class Settings {
  public:
    Settings();

    // Changeable global variables
    
    const unsigned long eventTime_Time = 100;                 // Event time for time functions
    const unsigned long eventTime_Light = 3000;               // Event time for light functions
    const unsigned long eventTime_PIR = 2000;                 // Event time for PIR functions
    const unsigned long eventTime_Server = 2000;              // Event time for Server functions

    unsigned long previousTime_Time = 0;                      // Event start time for time functions
    unsigned long previousTime_Light = 11;                    // Event start time for light functions
    unsigned long previousTime_PIR = 22;                      // Event start time for PIR functions
    unsigned long previousTime_Server = 35;                   // Event start time for Server functions

    const char* webTitle = "Arduino Nano 33 IoT Nixie Clock"; // WebUI Header Title First Part
    const char* webName = "Living Room";                      // WebUI Header Title Second Part / Room Name
    const char* webFont = "Audiowide";                        // WebUI Initial Font

    const int longpressduration = 5000;                       // Time in ms for button long press duration

    // Global variables

    unsigned long currentTime;                                // Used by millis

    bool I2C_CODE[3];                                         // Hold success status of I2C devices
    byte hour, minute, second;                                // Store RTC data
    byte day, month, year;                                    // Store RTC data

    // Flash EEPROM initial save values
    String flash_SSID;                                        // Store the SSID retrieved from the WebUI
    String flash_PASS;                                        // Store the Password retrieved from the WebUI
    String flashTitle;                                        // Store the WebUI Title retrieved from the WebUI
    String flashName;                                         // Store the WebUI Name retrieved from the WebUI
    byte flashBrightness = 255;                               // PWM HV5530 brightness level 1 - 255 (Note: 0 would turn off the Nixies but leave the HV on)
    bool flashNTP = 1;                                        // Enable/Disable NTP
    byte flashNTPPool = 3;                                    // Stores the NTP pool address (1 = africa, 2 = asia, 3 = europe, 4 = north america, 5 = oceania, 6 = south america)
    byte flashOnHour = 6;
    byte flashOffHour = 23;
    bool flashPIR = 1;                                        // Enable/Disable PIR
    bool flashLight = 1;                                      // Enable/Disable Light Sensor
    bool flashUSB = 1;                                        // Set power supply mode, setting to 1 enables the 5v to 12v booster for the HV5530
    byte flashFont = 1;                                       // The WebUI font
    byte flashBackground = 4;                                 // The WebUI background
    int flashUTCOffset = 1;                                   // UTC offset in hours
    byte flashColon = 15;                                     // Colon LED Brightness via PWM
    byte flashLED1 = 180;                                     // Switch 1 LED Brightness via PWM
    byte flashLED2 = 30;                                      // Switch 2 LED Brightness via PWM
    byte flashLED3 = 170;                                     // Switch 3 LED Brightness via PWM
    byte flashSpin = 1;                                       // Nixie Hourly Spin Cycles
    byte flashLux = 1;                                        // Low Lux Level Threshold

    const char* ntpServerName = "europe.pool.ntp.org";        // Stores the NTP pool to query, changed via flashNTPPool
    
    const char* ssid;                                         // Hold the SSID
    const char* pass;                                         // Hold the SSID Password

    bool noSSID = 0;

    void begin();
    void rwSettings(byte setting, bool save);
    void debug(byte n);

    const char* fwVersion = "1.0";

  private:
};

#endif
