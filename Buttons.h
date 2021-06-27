#ifndef Buttons_h
#define Buttons_h

#include "Arduino.h"
#include "Globals.h"
#include "I2C.h"
#include "TimeTask.h"

class Buttons {
  public:
    Buttons(I2C* i2c, TimeTask* timetask, Settings* settings);
    void task();
    void begin();
  private:
    I2C* _i2c;
    TimeTask* _timetask;
    Settings* _settings;
    boolean sw1db;
    boolean sw2db;
    boolean sw3db;
    State _state;
    boolean debounce(boolean value, boolean* store);
};

#endif
