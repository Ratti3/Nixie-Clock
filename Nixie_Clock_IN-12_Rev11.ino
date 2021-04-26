#include "HV.h"
#include "NixieDisplay.h"
#include "TimeTask.h"
#include "I2C.h"
#include "WiFiTask.h"
//#include <Wire.h>

// Real time clock
RTC_DS3231 rtc;

// BME280 object (pins 4 and 5)
Adafruit_BME280 bme;

// VEML7700 object (pins 4 and 5)
Adafruit_VEML7700 veml = Adafruit_VEML7700();

// HV shift registers
NixieDisplay nixie;

// DC booster enable pin control
HV hv;

// Variables and settings
Settings settings;

I2C i2c(&nixie, &rtc, &bme, &veml, &settings);

// Tasks that run periodically
TimeTask timeTask(&nixie, &i2c, &settings);

WiFiUDP ntpUDP;
NTPClient timeclient(ntpUDP, settings.ntpServerName, 3600, 60000);

WiFiTask wifitask(&nixie, &i2c, &timeclient, &settings);

void setup() {

  // Start Serial
  Serial.begin(9600);
  delay(1000);

  // Initiate the HV5532 outputs
  Debug("[DEBUG] Begin HV5530", 1);
  nixie.begin();

  // Initiate the DC/DC enable pin
  Debug("[DEBUG] Begin HV enable pins", 1);
  hv.begin();

  nixie.runSlotMachine();

  // Initialise DS3231, BME280 & VEML7700
  Debug("[DEBUG] Begin I2C", 1);
  i2c.begin();
  if (settings.I2C_CODE[0]) {
    Debug("[DEBUG] Could not find a valid DS3231 IC, check wiring!", 1);
  } else {
    Debug("[DEBUG] Found a valid DS3231 IC", 1);
  }
  if (settings.I2C_CODE[1]) {
    Debug("[DEBUG] Could not find a valid BME280 sensor, check wiring!", 1);
  } else {
    Debug("[DEBUG] Found a valid BME280 sensor", 1);
  }
  if (settings.I2C_CODE[2]) {
    Debug("[DEBUG] Could not find a valid VEML7700 sensor", 1);
  } else {
    Debug("[DEBUG] Foubd a valid VEML7700 sensor", 1);
  }

wifitask.begin();

  // Initial NTP time sync
  Debug("[DEBUG] Start NTP Time Sync", 1);
  wifitask.getNTP();

  // Display the IP address of the Nano
  Debug("[DEBUG] Display IP Address", 1);
  wifitask.displayIP();

  // Start the Nano web server
  Debug("[DEBUG] Start Web Server", 1);
  ServerBegin();



}

void loop() {
  digitalWrite(PIN_HV_EN, HIGH); // There is now 170V on the board!
  settings.currentTime = millis();

  if (settings.currentTime - settings.previousTime_Time >= settings.eventTime_Time) {
    timeTask.task();
    settings.previousTime_Time = settings.currentTime;
  }

  if (settings.currentTime - settings.previousTime_Light >= settings.eventTime_Light) {
    i2c.readLight();
    settings.previousTime_Light = settings.currentTime;
  }

  if (settings.currentTime - settings.previousTime_PIR >= settings.eventTime_PIR) {
    
    i2c.PIR();
    settings.previousTime_PIR = settings.currentTime;
  }

  if (settings.currentTime - settings.previousTime_Server >= settings.eventTime_Server) {
    // Start listening for web clients
    clientServer();
    settings.previousTime_Server = settings.currentTime;
  }

}

void Debug(const char* text, bool line) {
  if (line) {
    Serial.println(text);
  } else {
    Serial.print(text);
  }
  
}
