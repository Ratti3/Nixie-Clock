#ifndef HV_h
#define HV_h

#include "Arduino.h"
#include "Globals.h"

class HV {
  public:
    HV();
    void begin();
    bool isOn();
    void switchOn();
    void switchOff();
  private:
    boolean _hvon = false;
};

#endif
