#ifndef HV_h
#define HV_h

#include "Arduino.h"
#include "Globals.h"

class HV {
  public:
    HV();
    void begin(bool USBpower);
    bool isOn();
    void switchOn();
    void switchOff();
    void switch12VOn();
    void switch12VOff();
    bool _hvon = 0;
  private:
};

#endif
