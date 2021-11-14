#ifndef NixieDisplay_h
#define NixieDisplay_h

#include "Arduino.h"
#include "Globals.h"
#include "Settings.h"

// Location of each segment in each tube the 64 bit shift register
const byte dotOne        = 63;
const byte dotTwo        = 62;
const byte dotFive       = 31;
const byte dotSix        = 30;
const byte hourTens[]    = {41,32,33,34,35,36,37,38,39,40};
const byte hourUnits[]   = {51,42,43,44,45,46,47,48,49,50};
const byte minuteTens[]  = {61,52,53,54,55,56,57,58,59,60};
const byte minuteUnits[] = {9,0,1,2,3,4,5,6,7,8};
const byte secondTens[]  = {19,10,11,12,13,14,15,16,17,18};
const byte secondUnits[] = {29,20,21,22,23,24,25,26,27,28};

class NixieDisplay {
  public:
    NixieDisplay(Settings* settings);
    void begin();
    void enableSegment(byte segment);
    void enableSegments(byte tube, byte segment);
    void disableSegment(byte segment);
    void disableSegments(const byte segments[], int count);
    void enableAllDots();
    void disableAllSegments();
    void updateDisplay();
    void runSlotMachine(byte c);
    void displayFW();
  private:
    Settings* _settings;
    // Frame of data to be shifted into 64 bit HV shift register
    byte _frame[8]; // 8 bytes = 64 bits = 6 digits @ 10 bits + 2 dots @ 1 bit + 2 unused bits
};

#endif
