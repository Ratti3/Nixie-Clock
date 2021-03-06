#ifndef Buttons_h
#define Buttons_h

#include "Arduino.h"
#include "Globals.h"
#include "I2C.h"
#include "Fade.h"
#include "TimeTask.h"

class Buttons {
  public:
    Buttons(I2C* i2c, Fade* fade, TimeTask* timetask, Settings* settings);
    void task(byte sw);
    void begin();
  private:
    I2C* _i2c;
    Fade* _fade;
    TimeTask* _timetask;
    Settings* _settings;
    State _state;
};

#endif
