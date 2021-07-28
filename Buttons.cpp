#include "Buttons.h"

Buttons::Buttons(I2C* i2c, Fade* fade, TimeTask* timetask, Settings* settings) {
  _i2c = i2c;
  _fade = fade;
  _timetask = timetask;
  _settings = settings;
  _state = Run;
}

void Buttons::begin() {
  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_SW2, INPUT_PULLUP);
  pinMode(PIN_SW3, INPUT_PULLUP);
}

void Buttons::task(byte sw) {
  if (sw == 1) {
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
  
  if (sw == 2 && _state != Run) {

    // Hold the date and time for adjusting
    int d = 0;
    int m = 0;
    int y = 0;
    int h = 0;
    int mn = 0;
    int s = 0;

    // Get current time and date from DS3231 RTC
    if (!_settings->I2C_CODE[0]) {
      _i2c->readDate();
      _i2c->readTime();
      d = _settings->day;
      m = _settings->month;
      y = _settings->year;
      h = _settings->hour;
      mn = _settings->minute;
      s = _settings->second;
    }
    if (_state == SetHour) {
      if (h == 23) {
        h = 0;
      } else {
        h++;
      }
    } else if (_state == SetMinute) {
      if(mn == 59) {
        mn = 0;
      } else {
        mn++;
      }
    } else if (_state == SetSecond) {
      if(s == 59) {
        s = 0;
      } else {
        s++;
      }
    } else if (_state == SetDay) {
      if(d == 31) {
        d = 1;
      } else {
        d++;
      }
    } else if (_state == SetMonth) {
      if(m == 12) {
        m = 1;
      } else {
        m++;
      }
    } else if (_state == SetYear) {
      if(y == 99) {
        y = 20;
      } else {
        y++;
      }
    }
    // Store the adjusted date and time
    if (!_settings->I2C_CODE[0]) {
      _settings->day = d;
      _settings->month = m;
      _settings->year = y;
      _settings->hour = h;
      _settings->minute = mn;
      _settings->second = s;
    }
    // Save date and time
    _i2c->adjustDateTime(3);
    _fade->switchLEDFlash(1);
  }

  if (sw == 3 && _state != Run) {

    // Hold the date and time for adjusting
    byte d = 0;
    byte m = 0;
    byte y = 0;
    byte h = 0;
    byte mn = 0;
    byte s = 0;

    // Get current time and date from DS3231 RTC
    if (!_settings->I2C_CODE[0]) {
      _i2c->readDate();
      _i2c->readTime();
      d = _settings->day;
      m = _settings->month;
      y = _settings->year;
      h = _settings->hour;
      mn = _settings->minute;
      s = _settings->second;
    }
    if (_state == SetHour) {
      if (h == 1) {
        h = 23;
      } else {
        h--;
      }
    } else if (_state == SetMinute) {
      if(mn == 0) {
        mn = 59;
      } else {
        mn--;
      }
    } else if (_state == SetSecond) {
      if(s == 0) {
        s = 59;
      } else {
        s--;
      }
    } else if (_state == SetDay) {
      if(d == 1) {
        d = 31;
      } else {
        d--;
      }
    } else if (_state == SetMonth) {
      if(m == 1) {
        m = 12;
      } else {
        m--;
      }
    } else if (_state == SetYear) {
      if(y == 20) {
        y = 99;
      } else {
        y--;
      }
    }
    // Store the adjusted date and time
    if (!_settings->I2C_CODE[0]) {
      _settings->day = d;
      _settings->month = m;
      _settings->year = y;
      _settings->hour = h;
      _settings->minute = mn;
      _settings->second = s;
    }
    // Save date and time
    _i2c->adjustDateTime(3);
    _fade->switchLEDFlash(1);
  }

  // Show THP
  if (sw == 2 && _state == Run) {
    _i2c->displayTHP();
  }
  // Show date
  if (sw == 3 && _state == Run) {
    _timetask->showDate();
  }

}
