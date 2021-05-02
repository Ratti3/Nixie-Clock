#ifndef WiFiTask_h
#define WiFiTask_h

#include "Arduino.h"
#include "Globals.h"
#include "I2C.h"
#include "Settings.h"
#include "NixieDisplay.h"
#include "TimeTask.h"
#include "Secrets.h"
#include "Fade.h"

class WiFiTask {
  public:
    WiFiTask(NixieDisplay* nixie, TimeTask* timetask, I2C* i2c, NTPClient* timeclient, WiFiServer* server, Settings* settings, Fade* fade);
    void connectWiFi();
    void ntpBegin();
    void serverBegin();
    void getNTP();
    void displayIP();
    void clientServer();
    void apServer();
    void startAP();

    bool WiFiFail = 0;                             // Holds status for WiFi connection failure
    bool apStatus;
  private: 
    void getNTPTime();
    void printWifiStatus();
    
    NixieDisplay* _nixie;
    TimeTask* _timetask;
    I2C* _i2c;
    NTPClient* _timeclient;
    WiFiServer* _server;
    Settings* _settings;
    Fade* _fade;

    String urlDecode(const String& text);

    int cb;                                        // Holds parsed NTP packet
    byte count;                                    // Counter for retrying packets
    byte retry_max = 1;                            // Number of total attempts (retry_count * try_count)

//    const char* ssid = WIFI_SSID;                  // Your network SSID
//    const char* pass = WIFI_PASS;                  // Your network password

    const char* ssid_ap = WIFI_AP_SSID;                  // Your network AP SSID
    const char* pass_ap = WIFI_AP_PASS;                  // Your network password

    byte ipAddress[4];
};

#endif
