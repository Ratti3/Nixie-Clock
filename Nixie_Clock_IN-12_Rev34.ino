#include "WiFiTask.h"

// Real time clock
RTC_DS3231 rtc;

// BME280 object (SCL/SDA Pins 4 and 5)
Adafruit_BME280 bme;

// VEML7700 object (SCL/SDA Pins 4 and 5)
Adafruit_VEML7700 veml = Adafruit_VEML7700();

// DC booster enable pin control
HV hv;

// Variables and settings
Settings settings(&hv);

// HV shift registers
NixieDisplay nixie(&settings);

// PWM fade functions
Fade fade(&settings);

// I2C devices
I2C i2c(&nixie, &rtc, &bme, &veml, &settings, &hv);

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

// 
AceButton button1(PIN_SW1);
AceButton button2(PIN_SW2);
AceButton button3(PIN_SW3);

Buttons buttons(&i2c, &fade, &timetask, &settings);
// Forward reference to prevent Arduino compiler becoming confused.
void handleEvent(AceButton*, uint8_t, uint8_t);

void setup() {

  // Start Serial
  Serial.begin(57600);
  delay(2000);

  // Generate random seed for random number generation
  randomSeed(analogRead(RandomSeed));

  // Load all default/saved settings from Flash
  settings.debug(0);
  settings.begin();

  // Initiate the button functions
  buttons.begin();

  // Set and configure AceButton features
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  buttonConfig->setLongPressDelay(settings.longpressduration);

  // Initialise the Colon and Switch LEDs
  //settings.debug(0);
  fade.start();
  
  // Initiate the HV5532 outputs
  settings.debug(1);
  nixie.begin();

  // Fire up the 170V and 5 to 12V booster if needed
  settings.debug(2);
  hv.begin(settings.flashUSB);

  // Take the Nixie tubes for a quick spin
  nixie.runSlotMachine(1);

  // Display the firmware version
  nixie.displayFW();

  // Initialise DS3231 RTC
  settings.debug(3);
  i2c.rtcBegin();
  if (settings.I2C_CODE[0]) {
  settings.debug(4);
  } else {
    settings.debug(5);
  }

  // Initialise BME280 environmental sensor
  i2c.bmeBegin();
  if (settings.I2C_CODE[1]) {
    settings.debug(6);
  } else {
    settings.debug(7);
  }

  // Initialise VEML7700 light sensor
  i2c.vemlBegin();
  if (settings.I2C_CODE[2]) {
    settings.debug(8);
  } else {
    settings.debug(9);
  }

  // Connect to wifi SSID
  settings.debug(10);
  wifitask.connectWiFi();

  // Start the builtin AP if Wi-Fi connection has failed
  if (wifitask.WiFiFail) {
    wifitask.startAP();
  }

  // Start the NTP client task
  settings.debug(11);
  wifitask.ntpBegin();

  // Start the Web Server, update the RTC via NTP and display the IP address of the device
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

  // Using millis to call timed functions, you can use interrupts if more accuracy is required
  settings.currentTime = millis();

  // Main time display function
  if (settings.currentTime - settings.previousTime_Time >= settings.eventTime_Time) {
    timetask.task();
    settings.previousTime_Time = settings.currentTime;
  }
  // Light function
  if (settings.currentTime - settings.previousTime_Light >= settings.eventTime_Light) {
    i2c.readLight();
    settings.previousTime_Light = settings.currentTime;
  }
  // PIR function
  if (settings.currentTime - settings.previousTime_PIR >= settings.eventTime_PIR) {
    i2c.PIR();
    settings.previousTime_PIR = settings.currentTime;
  }
  // WiFi Server function
  if (settings.currentTime - settings.previousTime_Server >= settings.eventTime_Server) {
    wifitask.clientServer();
    settings.previousTime_Server = settings.currentTime;
  }
  // 
  if (settings.currentTime - settings.previousTime_OnOffHour >= settings.eventTime_OnOffHour) {
    settings.OnOff();
    settings.previousTime_OnOffHour = settings.currentTime;
  }
  // Check for button presses, no need to use millis or interrupts for accuracy, works fine <4ms and less
  button1.check();
  button2.check();
  button3.check();

}

// The event handler for the button library, note due to it being a static function it's not much use adding to the buttons.cpp file
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventClicked:        // Short press
    case AceButton::kEventReleased:       // Medium press
      if (button->getPin() == PIN_SW1) {
        SPL("Button 1 clicked!");
        buttons.task(1);
      } else if (button->getPin() == PIN_SW2) {
        SPL("Button 2 clicked!");
        buttons.task(2);
      } else if (button->getPin() == PIN_SW3) {
        SPL("Button 3 clicked!");
        buttons.task(3);
      }
      break;
    case AceButton::kEventLongReleased:   // Long press, duration defined by settings.longpressduration
      switch (button->getPin()) {
        case PIN_SW1:
          break;
        case PIN_SW2:
          break;
        case PIN_SW3:
          break;
      }
      break;
    case AceButton::kEventDoubleClicked:  // Double press
      switch (button->getPin()) {
        case PIN_SW1:
          if (!wifitask.WiFiFail) {
            wifitask.displayIP(); 
          }
          break;
        case PIN_SW2:
          if (!wifitask.WiFiFail) {
            wifitask.getNTP();
          }
          break;
        case PIN_SW3:
          nixie.runSlotMachine(1);
          break;
      }
      break;
  }
}
