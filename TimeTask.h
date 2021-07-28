#ifndef TimeTask_h
#define TimeTask_h

#include "Arduino.h"
#include "Globals.h"
#include "I2C.h"
#include "Settings.h"
#include "NixieDisplay.h"

class TimeTask {
  public:
    TimeTask(NixieDisplay* nixie, I2C* i2c, Settings* settings);
    void task();
    void showDate();
    void setState(State state);
  private: 
    void Blank();
    NixieDisplay* _nixie;
    I2C* _i2c;
    Settings* _settings;
    State _state;
};

#endif
