#include "TimeTask.h"

TimeTask::TimeTask(NixieDisplay* nixie, I2C* i2c, Settings* settings) {
  _nixie = nixie;
  _i2c = i2c;
  _settings = settings;
}

void TimeTask::task() {

int h = 88;
int m = 88;
int s = 88;

  // Get current time from DS3231 RTC chip
  if (!_settings->I2C_CODE[0]) {
    _i2c->readTime();
    h = _settings->hour;
    m = _settings->minute;
    s = _settings->second;
  }


//Serial.print(h);
//Serial.print(m);
//Serial.print(s);
//Serial.println(" ");


/*
  // DST auto-adjust for Europe
  if(_settings->getEuropeDstEnabled()) {
    if(now.dayOfWeek() == 0 && now.month() == 10 && now.day() >= 25 && 
      hour == 3 && _settings->getEuropeSummerTime()) {

      // It's 3 am on the last Sunday of October and summertime is still in effect
      _rtc->adjust(DateTime(now.year(), now.month(), now.day(), 2, minute, second)); // 3 am -> 2 am
      _settings->setEuropeSummerTime(false); // Remember that winter time is now in effect
      
    } else if (now.dayOfWeek() == 0 && now.month() == 3 && now.day() >= 25 && 
      hour == 2 && !_settings->getEuropeSummerTime()) {

      // It's 2 am on the last Sunday of March and wintertime is still in effect
      _rtc->adjust(DateTime(now.year(), now.month(), now.day(), 3, minute, second)); // 2 am -> 3 am
      _settings->setEuropeSummerTime(true); // Remember that summer time is now in effect
    
    }
  }
 */

  // Enable and disable the right segments
  _nixie->disableSegments(hourTens, 10);
  _nixie->disableSegments(hourUnits, 10);
  _nixie->disableSegments(minuteTens, 10);
  _nixie->disableSegments(minuteUnits, 10);
  _nixie->disableSegments(secondTens, 10);
  _nixie->disableSegments(secondUnits, 10);
  
  if(!(_state == SetHour && s % 2 == 0)) {
    _nixie->enableSegment(hourTens[(h / 10) % 10]);
    _nixie->enableSegment(hourUnits[h % 10]);
  }
  if(!(_state == SetMinute && s % 2 == 0)) {
    _nixie->enableSegment(minuteTens[(m / 10) % 10]);
    _nixie->enableSegment(minuteUnits[m % 10]);
  }
  if(!(_state == SetSecond && s % 2 == 0)) {
    _nixie->enableSegment(secondTens[(s / 10) % 10]);
    _nixie->enableSegment(secondUnits[s % 10]);
  }

  // Flash the dots once per second
  if (s % 2 == 0) {
    analogWrite(PIN_COLON, 0);
  } else {
    analogWrite(PIN_COLON, 100);
  }

  // Write to display
  _nixie->updateDisplay();

  // Once a minute, run the slot machine effect to prevent cathode poisoning
  if (s == 0) {
    _nixie->runSlotMachine();
  }
  
}

void TimeTask::showDate() {

  // Get current date from DS3231 RTC chip
  int d = 88;
  int m = 88;
  int y = 88;

  // Get current time from DS3231 RTC chip
  if (!_settings->I2C_CODE[0]) {
    _i2c->readDate();
    d = _settings->day;
    m = _settings->month;
    y = _settings->year;
  }


  // Enable and disable the right segments
  _nixie->disableSegments(hourTens, 10);
  _nixie->disableSegments(hourUnits, 10);
  _nixie->disableSegments(minuteTens, 10);
  _nixie->disableSegments(minuteUnits, 10);
  _nixie->disableSegments(secondTens, 10);
  _nixie->disableSegments(secondUnits, 10);

  _nixie->enableSegment(hourTens[(d / 10) % 10]);
  _nixie->enableSegment(hourUnits[d % 10]);
  _nixie->enableSegment(minuteTens[(m / 10) % 10]);
  _nixie->enableSegment(minuteUnits[m % 10]);
  _nixie->enableSegment(secondTens[(y / 10) % 10]);
  _nixie->enableSegment(secondUnits[y % 10]);

  // Write to display
  _nixie->updateDisplay();
  delay(3000);

}

void TimeTask::setState(State state) {
  _state = state;
}
