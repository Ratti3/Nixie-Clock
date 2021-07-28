#include "I2C.h"

I2C::I2C(NixieDisplay* nixie, RTC_DS3231* rtc, Adafruit_BME280* bme, Adafruit_VEML7700* veml, Settings* settings, HV* hv) {
  _nixie = nixie;
  _rtc = rtc;
  _bme = bme;
  _veml = veml;
  _settings = settings;
  _hv = hv;
}

void I2C::rtcBegin() {
  Wire.begin();

  // Start the DS3231 RTC
  if (!_rtc->begin()) {
    _settings->I2C_CODE[0] = 1;
  } else {
    _settings->I2C_CODE[0] = 0;
  }
}

void I2C::bmeBegin() {
  // Start the BME280 Environmental Sensor with address of 0x76
  if (!_bme->begin(0x76)) {
    _settings->I2C_CODE[1] = 1;
    // Reduce BME sampling rate to prevent overheating
    _bme->setSampling(Adafruit_BME280::MODE_FORCED,
      Adafruit_BME280::SAMPLING_X1, // temperature
      Adafruit_BME280::SAMPLING_X1, // pressure
      Adafruit_BME280::SAMPLING_X1, // humidity
      Adafruit_BME280::FILTER_OFF);
  } else {
    _settings->I2C_CODE[1] = 0;
  }
}

void I2C::vemlBegin() {
  // Start the VEML7700 light sensor
  if (!_veml->begin()) {
    _settings->I2C_CODE[2] = 1;
  } else {
    _settings->I2C_CODE[2] = 0;
  }

  // Set the BS612 PIR pin to input
  pinMode(PIN_PIR, INPUT);
}

void I2C::displayTHP() {

  int t[2];
  int h;
  int p;

  _bme->takeForcedMeasurement();

  t[0] = _bme->readTemperature();
  t[1] = _bme->readTemperature();
  h = _bme->readHumidity();
  p = _bme->readPressure() / 100.0F;

  // Display the IP Address
  // Enable and disable the right segments
  _nixie->disableSegments(hourTens, 10);
  _nixie->disableSegments(hourUnits, 10);
  _nixie->disableSegments(minuteTens, 10);
  _nixie->disableSegments(minuteUnits, 10);
  _nixie->disableSegments(secondTens, 10);
  _nixie->disableSegments(secondUnits, 10);

  _nixie->enableSegment(hourTens[t[0]/10]);
  _nixie->enableSegment(hourUnits[t[0]%10]);
  _nixie->enableSegment(minuteTens[t[1]/10]);

  // Write to display
  _nixie->updateDisplay();
  delay(2000);

  /* DEBUG *
  SP("Temp");
  SP(t[0]/10);
  SP(t[0]%10);
  SP(t[1]/10);
  SPL(" ");
  */

  // Display humidity
  // Enable and disable the right segments
  _nixie->disableSegments(hourTens, 10);
  _nixie->disableSegments(hourUnits, 10);
  _nixie->disableSegments(minuteTens, 10);
  _nixie->disableSegments(minuteUnits, 10);
  _nixie->disableSegments(secondTens, 10);
  _nixie->disableSegments(secondUnits, 10);

  _nixie->enableSegment(hourTens[h/10]);
  _nixie->enableSegment(hourUnits[h%10]);

  // Write to display
  _nixie->updateDisplay();
  delay(2000);

  /* DEBUG *
  SP("H");
  SP(h/10);
  SP(h%10);
  SPL(" ");
  */

  byte p1[4];

  if (p > 999) {
    p1[0] = p/1000;
    p1[1] = (p/100)%10;
    p1[2] = (p/10)%10;
    p1[3] = p%10;
  } else if (p > 99) {
    p1[0] = 10;
    p1[1] = (p/100)%10;
    p1[2] = (p/10)%10;
    p1[3] = p%10;
  } else if (p > 9) {
    p1[0] = 0;
    p1[1] = 0;
    p1[2] = (p/10)%10;
    p1[3] = p%10;
  } else if (p > 0) {
    p1[0] = 0;
    p1[1] = 0;
    p1[2] = 0;
    p1[3] = p%10;
  } else {
    p1[0] = 0;
    p1[1] = 0;
    p1[2] = 0;
    p1[3] = 0;
  }

  // Display pressure
  // Enable and disable the right segments
  _nixie->disableSegments(hourTens, 10);
  _nixie->disableSegments(hourUnits, 10);
  _nixie->disableSegments(minuteTens, 10);
  _nixie->disableSegments(minuteUnits, 10);
  _nixie->disableSegments(secondTens, 10);
  _nixie->disableSegments(secondUnits, 10);

  _nixie->enableSegment(hourTens[p1[0]]);
  _nixie->enableSegment(hourUnits[p1[1]]);
  _nixie->enableSegment(minuteTens[p1[2]]);
  _nixie->enableSegment(minuteUnits[p1[3]]);

  // Write to display
  _nixie->updateDisplay();
  delay(2000);

  /* DEBUG *
  SP("P");
  SP(p1[0]);
  SP(p1[1]);
  SP(p1[2]);
  SP(p1[3]);
  SPL(" ");
  */

}

// Light Sensor Routine
void I2C::readLight() {
  if (_settings->I2C_CODE[2]) {
    return;
  }
  if (_veml->readLux() < _settings->flashLux) {
    //digitalWrite(PIN_HV_EN, LOW);


  } else {
    //digitalWrite(PIN_HV_EN, HIGH);

  }
}

// PIR Sensor Routine
void I2C::PIR() {
  motion = digitalRead(PIN_PIR);     // Read the PIR Pin Output
  if (motion == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn built in LED ON
    if (pirState == LOW) {
      _hv->switchOn();
      pirState = HIGH;
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn built in LED OFF
    if (pirState == HIGH) {
      _hv->switchOff();
      pirState = LOW;
    }
  }
}

// Read time from DS3231
void I2C::readTime() {
  DateTime now = _rtc->now();
  _settings->hour = now.hour();
  _settings->minute = now.minute();
  _settings->second = now.second();
}

// Read date from DS3231
void I2C::readDate() {
  DateTime now = _rtc->now();
  _settings->day = now.day();
  _settings->month = now.month();
  _settings->year = now.year();
}

// Adjust DS3231 date and time using epoch
void I2C::adjustEpoch(unsigned long epoch) {
  _rtc->adjust(DateTime(epoch));
  SP("Setting DS3231 Date and Time using EPOCH");
}

// Adjust DS3231 date/time using user input values
void I2C::adjustDateTime(byte dt) {
  if (dt == 1) { // If dt = 1, then only set date
    readTime();
  } else if (dt == 2) {  // If dt = 2, then only set time
    readDate();
  }
  _rtc->adjust(DateTime(_settings->year, _settings->month, _settings->day, _settings->hour, _settings->minute, _settings->second));
  SPL("Setting date/time manually updated");
}
