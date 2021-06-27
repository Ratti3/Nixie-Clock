#include "Buttons.h"

Buttons::Buttons(I2C* i2c, TimeTask* timetask, Settings* settings) {
  _i2c = i2c;
  _timetask = timetask;
  _settings = settings;
  _state = Run;
}

void Buttons::begin() {
  pinMode(PIN_SW1, INPUT);
  pinMode(PIN_SW2, INPUT);
  pinMode(PIN_SW3, INPUT);
}

void Buttons::task() {
  bool sw1 = debounce(digitalRead(PIN_SW1) == LOW, &sw1db);
  bool sw2 = debounce(digitalRead(PIN_SW2) == LOW, &sw2db);
  bool sw3 = debounce(digitalRead(PIN_SW3) == LOW, &sw2db);
  
  if (sw1) {
    if(_state == Run) {
      _state = SetDay;
    } else if (_state == SetDay) {
      _state = SetMonth;
    } else if (_state == SetMonth) {
      _state = SetYear;
    } else if (_state == SetYear) {
      _state = SetHour;
    } else if (_state == SetHour) {
      _state = SetMinute;
    } else if (_state == SetMinute) {
      _state = SetSecond;
    } else if (_state == SetSecond) {
      _state = Run;
    }
    _timetask->setState(_state);
  }
  
  if (sw2 && _state != Run) {

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

    if (_state == SetHour) {
      int hour = h;
      if (hour == 23) {
        hour = 0;
      } else {
        hour++;
      }
      _i2c->adjustDateTime(3);
      _settings->year, _settings->month, _settings->day, _settings->hour, _settings->minute, _settings->second
      _rtc->adjust(DateTime(now.year(), now.month(), now.day(), hour, now.minute(), now.second()));
    } else if (_state == SetMinute) {
      int minute = m;
      if(minute == 59) {
        minute = 0;
      } else {
        minute++;
      }
      _rtc->adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), minute, now.second()));
    } else if (_state == SetSecond) {
      int second = s;
      if(second == 59) {
        second = 0;
      } else {
        second++;
      }
      _rtc->adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), second));
    }
    _timetask->task();
  }

}

boolean Buttons::debounce(boolean value, boolean* store) {
  if(value) {
    if(*store) {
      value = false;
    } else {
      *store = true;
    }
  } else {
    *store = false;
  }
  return value;
}
