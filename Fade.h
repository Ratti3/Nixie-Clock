#ifndef Fade_h
#define Fade_h

#include "Arduino.h"
#include "Globals.h"

class Fade {
  public:
    Fade();
    void fadeIn();
    void fadeOut();
    void setBrightness();
  private:
};

#endif
