#ifndef I2C_h
#define I2C_h

#include "Arduino.h"
#include "Globals.h"
#include "Settings.h"
#include "NixieDisplay.h"
#include "HV.h"

class I2C {
  public:
    I2C(NixieDisplay* nixie, RTC_DS3231* rtc, Adafruit_BME280* bme, Adafruit_VEML7700* veml, Settings* settings, HV* hv);
    void rtcBegin();
    void bmeBegin();
    void vemlBegin();
    void readTime();
    void readDate();
    void adjustTime(unsigned long epoch);
    void adjustDateTime(bool dt);
    void displayTHP();
    void readLight();
    void PIR();
  private:
    NixieDisplay* _nixie;
    RTC_DS3231* _rtc;
    Adafruit_BME280* _bme;
    Adafruit_VEML7700* _veml;
    Settings* _settings;
    HV* _hv;
    int pirState = LOW;
    int motion = 0;
};

#endif
