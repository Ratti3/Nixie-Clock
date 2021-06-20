#include "WiFiTask.h"

// Real time clock
RTC_DS3231 rtc;

// BME280 object (SCL/SDA Pins 4 and 5)
Adafruit_BME280 bme;

// VEML7700 object (SCL/SDA Pins 4 and 5)
Adafruit_VEML7700 veml = Adafruit_VEML7700();

// HV shift registers
NixieDisplay nixie;

// DC booster enable pin control
HV hv;

// Variables and settings
Settings settings;

// PWM fade functions
Fade fade(&settings);

// I2C devices
I2C i2c(&nixie, &rtc, &bme, &veml, &settings);

// Tasks that run periodically
TimeTask timetask(&nixie, &i2c, &settings);

// HTTP Server Port
WiFiServer server(80);

// Initialise UDP
WiFiUDP ntpUDP;

// Initialise NTP Client
NTPClient timeclient(ntpUDP);

// WiFi functions
WiFiTask wifitask(&nixie, &timetask, &i2c, &timeclient, &server, &settings, &fade, &hv);

void setup() {

  // Start Serial
  Serial.begin(57600);
  delay(2000);

  // Generate random seed
  randomSeed(analogRead(RandomSeed));

  // Load all saved settings from Flash
  settings.debug(0);
  settings.begin();
  
  // Initiate the HV5532 outputs
  settings.debug(1);
  nixie.begin();

  // Fire up the 170V and 5 to 12V booster if needed
  settings.debug(2);
  hv.begin(settings.flashUSB);

  // Take the Nixie tubes for a quick spin
  nixie.runSlotMachine(1);

  // Initialise DS3231
  settings.debug(3);
  i2c.rtcBegin();
  if (settings.I2C_CODE[0]) {
  settings.debug(4);
  } else {
    settings.debug(5);
  }

  // Initialise BME280
  i2c.bmeBegin();
  if (settings.I2C_CODE[1]) {
    settings.debug(6);
  } else {
    settings.debug(7);
  }

  // Initialise VEML7700
  i2c.vemlBegin();
  if (settings.I2C_CODE[2]) {
    settings.debug(8);
  } else {
    settings.debug(9);
  }

  // Connect to wifi SSID
  settings.debug(10);
  wifitask.connectWiFi();

  if (wifitask.WiFiFail) {
    wifitask.startAP();
  }

  settings.debug(11);
  wifitask.ntpBegin();

  if (!wifitask.WiFiFail) {
    settings.debug(12);
    wifitask.serverBegin();

    settings.debug(13);
    wifitask.getNTP();
  
    settings.debug(14);
    wifitask.displayIP(); 
  }

}

void loop() {

  settings.currentTime = millis();

  if (settings.currentTime - settings.previousTime_Time >= settings.eventTime_Time) {
    timetask.task();
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
    wifitask.clientServer();
    settings.previousTime_Server = settings.currentTime;
  }

}
