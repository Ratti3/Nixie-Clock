#include "TimeTask.h"

TimeTask::TimeTask(NixieDisplay* nixie, I2C* i2c, Settings* settings, HV* hv) {
  _nixie = nixie;
  _i2c = i2c;
  _settings = settings;
  _hv = hv;
}

void TimeTask::task() {

  // Set a default value for time if RTC not available (RTC is absolutely necessary to display time)
  int h = 0;
  int m = 0;
  int s = 0;
  // Get current time from DS3231 RTC chip
  if (!_settings->I2C_CODE[0]) {
    _i2c->readTime();
    h = _settings->hour;
    m = _settings->minute;
    s = _settings->second;
  }

//SP(h);
//SP(m);
//SP(s);
//SPL(" ");

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

  // Blank the display
  Blank();
  // Enable and disable the right segments  
  if(_state == SetHour || _state == Run) {
    _nixie->enableSegment(hourTens[(h / 10) % 10]);
    _nixie->enableSegment(hourUnits[h % 10]);
  }
  if(_state == SetMinute || _state == Run) {
    _nixie->enableSegment(minuteTens[(m / 10) % 10]);
    _nixie->enableSegment(minuteUnits[m % 10]);
  }
  if(_state == SetSecond || _state == Run) {
    _nixie->enableSegment(secondTens[(s / 10) % 10]);
    _nixie->enableSegment(secondUnits[s % 10]);
  }

  // Flash the dots once per second
  if (s % 2 == 0 && _state == Run) {
    analogWrite(PIN_COLON, 0);
  } else {
    analogWrite(PIN_COLON, _settings->flashColon);
  }

  // Write to display
  _nixie->updateDisplay();

  // Once a minute, run the slot machine effect to prevent cathode poisoning
  if (s == 0  && _state == Run) {
    _nixie->runSlotMachine(_settings->flashSpin);
  }

  if (_state == SetDay || _state == SetMonth || _state == SetYear) {
    showDate();
  }
}

void TimeTask::OnOff() {
  int h;
  // Get current hour from DS3231 RTC chip
  if (!_settings->I2C_CODE[0]) {
    _i2c->readTime();
    h = _settings->hour;
  }
  _hv->isOn();
  if ((h >= _settings->flashOnHour) && (h < _settings->flashOffHour)) {
    if (!_hv->_hvon) {
      _hv->switchOn();
    }
  }
  if ((h >= _settings->flashOffHour) && (h < _settings->flashOnHour)) {
    if (_hv->_hvon) {
      _hv->switchOff();
    }
  }
}

void TimeTask::showDate() {
  int d = 30;
  int m = 7;
  int y = 21;

  // Get current date from DS3231 RTC chip
  if (!_settings->I2C_CODE[0]) {
    _i2c->readDate();
    d = _settings->day;
    m = _settings->month;
    y = _settings->year;
  }
  // Blank the display
  Blank();
  // Enable and disable the right segments
  if(_state == SetDay || _state == Run) {
    _nixie->enableSegment(hourTens[(d / 10) % 10]);
    _nixie->enableSegment(hourUnits[d % 10]);
  }
  if(_state == SetMonth || _state == Run) {
    _nixie->enableSegment(minuteTens[(m / 10) % 10]);
    _nixie->enableSegment(minuteUnits[m % 10]);
  }
  if(_state == SetYear || _state == Run) {
    _nixie->enableSegment(secondTens[(y / 10) % 10]);
    _nixie->enableSegment(secondUnits[y % 10]);
  }

  // Write to display
  _nixie->updateDisplay();
  if (_state == Run) {
    delay(3000);
  }
}

// Blanks the display
void TimeTask::Blank() {
  _nixie->disableSegments(hourTens, 10);
  _nixie->disableSegments(hourUnits, 10);
  _nixie->disableSegments(minuteTens, 10);
  _nixie->disableSegments(minuteUnits, 10);
  _nixie->disableSegments(secondTens, 10);
  _nixie->disableSegments(secondUnits, 10);
}

void TimeTask::setState(State state) {
  _state = state;
}
