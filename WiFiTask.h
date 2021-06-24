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
#include "HV.h"

class WiFiTask {
  public:
    WiFiTask(NixieDisplay* nixie, TimeTask* timetask, I2C* i2c, NTPClient* timeclient, WiFiServer* server, Settings* settings, Fade* fade, HV* hv);
    void connectWiFi();
    void ntpBegin();
    void serverBegin();
    void getNTP();
    void displayIP();
    void clientServer();
    void apServer();
    void startAP();
    bool WiFiFail = 0;                             // Holds state of WiFi connection
  private: 
    void printWifiStatus();
    
    NixieDisplay* _nixie;
    TimeTask* _timetask;
    I2C* _i2c;
    NTPClient* _timeclient;
    WiFiServer* _server;
    Settings* _settings;
    Fade* _fade;
    HV* _hv;

    String urlDecode(const String& text);

    const char* ssid_ap = WIFI_AP_SSID;            // Stores the AP SSID from Secrets.h
    const char* pass_ap = WIFI_AP_PASS;            // Stores the AP SSID Password from Secrets.h

    byte ipAddress[4];
};

#endif
