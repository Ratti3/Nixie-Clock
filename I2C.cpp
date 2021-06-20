#include "I2C.h"

I2C::I2C(NixieDisplay* nixie, RTC_DS3231* rtc, Adafruit_BME280* bme, Adafruit_VEML7700* veml, Settings* settings) {
  _nixie = nixie;
  _rtc = rtc;
  _bme = bme;
  _veml = veml;
  _settings = settings;
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
  Serial.print("Temp");
  Serial.print(t[0]/10);
  Serial.print(t[0]%10);
  Serial.print(t[1]/10);
  Serial.println(" ");
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
  Serial.print("H");
  Serial.print(h/10);
  Serial.print(h%10);
  Serial.println(" ");
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
  Serial.print("P");
  Serial.print(p1[0]);
  Serial.print(p1[1]);
  Serial.print(p1[2]);
  Serial.print(p1[3]);
  Serial.println(" ");
  */

}

// Light Sensor Routine
void I2C::readLight() {
  if (_settings->I2C_CODE[2]) {
    return;
  }
  Serial.println(_veml->readLux());
  Serial.println(_veml->readLux());
  if (_veml->readLux() < 5) {
    digitalWrite(PIN_HV_EN, LOW);
    Serial.println("OFF");
    delay(5000);
  } else {
    digitalWrite(PIN_HV_EN, HIGH);
    Serial.println("ON");
  }
}

// PIR Sensor Routine
void I2C::PIR() {
  val = digitalRead(PIN_PIR);        // Read the PIR Pin Output
  if (val == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn built in LED ON
    if (pirState == LOW) {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW); // Turn built in LED OFF
    if (pirState == HIGH) {
      Serial.println("Motion ended!");
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
void I2C::adjustTime(unsigned long epoch) {
  _rtc->adjust(DateTime(epoch));
  Serial.print("Setting DS3231 Date and Time");
}

// Adjust DS3231 date/time using user input values
void I2C::adjustDateTime(bool dt) {
  if (dt) { // If dt is true, then only set date with the data from the WebUI
    readTime();
  } else {  // If dt is false, then only set time with the data from the WebUI
    readDate();
  }
  _rtc->adjust(DateTime(_settings->year, _settings->month, _settings->day, _settings->hour, _settings->minute, _settings->second));
  Serial.println("Setting date/time manually via WebUI");
}
