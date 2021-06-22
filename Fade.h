#ifndef Fade_h
#define Fade_h

#include "Arduino.h"
#include "Globals.h"
#include "Settings.h"

class Fade {
  public:
    Fade(Settings* settings);
    void start();
    void fadeIn();
    void fadeOut();
    void setBrightness();
    void setSwitchLEDBrightness();
  private:
    Settings* _settings;
};

#endif
