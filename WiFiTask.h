#ifndef WiFiTask_h
#define WiFiTask_h

#include "Arduino.h"
#include "Globals.h"
#include "I2C.h"
#include "Settings.h"
#include "NixieDisplay.h"
#include "Secrets.h"

class WiFiTask {
  public:
    WiFiTask(NixieDisplay* nixie, I2C* i2c, NTPClient* timeclient, Settings* settings);
    void begin();
    void getNTP();
    void getNTPTime();
    void connectWiFi();
    void printWifiStatus();
    void displayIP();
  private: 
    NixieDisplay* _nixie;
    I2C* _i2c;
    NTPClient* _timeclient;
    Settings* _settings;

    int cb;                                        // Holds parsed NTP packet
    byte count;                                    // Counter for retrying packets
    byte retry_max = 1;                            // Number of total attempts (retry_count * try_count), this value is obtained from the Arduino
    bool WiFiFail = 0;                             // Holds status for WiFi connection failure
    const char* ssid = WIFI_SSID;                  // Your network SSID
    const char* pass = WIFI_PASS;                  // Your network password
    byte ipAddress[4];
};

#endif
