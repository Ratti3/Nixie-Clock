#ifndef Buttons_h
#define Buttons_h

#include "Arduino.h"
#include "Globals.h"
#include "I2C.h"
#include "TimeTask.h"

using namespace ace_button;

class Buttons {
  public:
    Buttons(I2C* i2c, TimeTask* timetask, Settings* settings, AceButton* button);
    void task();
    void begin();
  private:
    I2C* _i2c;
    TimeTask* _timetask;
    Settings* _settings;
    AceButton* _button;
    void static handleEvent(AceButton*, uint8_t, uint8_t);
    boolean sw1db;
    boolean sw2db;
    boolean sw3db;
    State _state;
    boolean debounce(boolean value, boolean* store);
};

#endif
