#include "NixieDisplay.h"

NixieDisplay::NixieDisplay() {}

void NixieDisplay::begin() {
  // HV5530 Pins
  pinMode(PIN_HV_LE, OUTPUT);
  pinMode(PIN_HV_BL, OUTPUT);
  pinMode(PIN_HV_DATA, OUTPUT);
  pinMode(PIN_HV_CLK, OUTPUT);
  analogWrite(PIN_HV_BL, 255);

  // Transistor controlled IN-12B Decimal Points (tubes 3 and 4)
  pinMode(PIN_DOT_THREE, OUTPUT);
  pinMode(PIN_DOT_FOUR, OUTPUT);
  
  disableAllSegments();
  updateDisplay(); // Write 64 zeros into the shift registers before enabling HV
}

void NixieDisplay::enableSegment(byte segment) {
  byte f = 7 - (segment / 8);
  byte b = segment % 8;
  _frame[f] |= 1 << b;
}

void NixieDisplay::enableSegments(byte tube, byte segment) {
  switch (tube) {
    case 1:
      enableSegment(hourTens[segment]);
      break;
    case 2:
      enableSegment(hourUnits[segment]);
      break;
    case 3:
      enableSegment(minuteTens[segment]);
      break;
    case 4:
      enableSegment(minuteUnits[segment]);
      break;
    case 5:
      enableSegment(secondTens[segment]);
      break;
    case 6:
      enableSegment(secondUnits[segment]);
      break;
  }
}

void NixieDisplay::disableSegments(const byte segments[], int count) {
  for(int i = 0; i < count; ++i) {
    byte segment = segments[i];
    disableSegment(segment);
  } 
}

void NixieDisplay::disableAllSegments() {
  for(int i = 0; i < 8; ++i) {
    _frame[i] = 0b00000000;
  }
}

void NixieDisplay::disableSegment(byte segment) {
  byte f = 7 - (segment / 8);
  byte b = segment % 8;
  _frame[f] &= ~(1 << b);  
}

void NixieDisplay::enableAllDots() {
  enableSegment(dotOne);
  enableSegment(dotTwo);
  enableSegment(dotFive);
  enableSegment(dotSix);
  digitalWrite(PIN_DOT_THREE, HIGH);
  digitalWrite(PIN_DOT_FOUR, HIGH);
}

void NixieDisplay::updateDisplay() {
  digitalWrite(PIN_HV_LE, LOW);
  for(int i = 0; i < 8; ++i) {
    shiftOut(PIN_HV_DATA, PIN_HV_CLK, MSBFIRST, _frame[i]);
  }
  digitalWrite(PIN_HV_LE, HIGH);
}

// Anti cathode poisoning spins
void NixieDisplay::runSlotMachine(byte c) {
  for (byte r = 0; r < c; ++r) {
    switch(random(6)) {
      // All segments at the same time
      case 0:
        for (int i = 0; i < 10; ++i) {
          disableAllSegments();
          for (byte x = 1; x <= 6; ++x) {
            enableSegments(x, i);
          }
          enableAllDots();
          updateDisplay();
          delay(150);
        }
        disableAllSegments();
        digitalWrite(PIN_DOT_THREE, LOW);
        digitalWrite(PIN_DOT_FOUR, LOW);
        break;
      // Single segment at a time, starting from the left
      case 1:
        for (int x = 1; x <= 6; ++x) {
          for (int i = 0; i < 10; ++i) {
            disableAllSegments();
            enableSegments(x, i);
            enableAllDots();
            updateDisplay();
            delay(50);
          }
        }
        disableAllSegments();
        digitalWrite(PIN_DOT_THREE, LOW);
        digitalWrite(PIN_DOT_FOUR, LOW);
        updateDisplay();
        break;
      // Single segment at a time, starting from the right
      case 2:
        for (int x = 6; x >= 0; --x) {
          for (int i = 0; i < 10; ++i) {
            disableAllSegments();
            enableSegments(x, i);
            enableAllDots();
            updateDisplay();
            delay(50);
          }
        }
        disableAllSegments();
        digitalWrite(PIN_DOT_THREE, LOW);
        digitalWrite(PIN_DOT_FOUR, LOW);
        updateDisplay();
        break;
      // All segments scrambled
      case 3:
        for (int i = 0, k = 10; (i < 10) && (k > 0); ++i, --k) {
          disableAllSegments();
          for (byte x = 1; x <= 5; x = x + 2) {
            enableSegments(x, i);
          }
          for (byte y = 6; y >= 2; y = y - 2) {
            enableSegments(y, k);
          }
          enableAllDots();
          updateDisplay();
          delay(150);
        }
        disableAllSegments();
        digitalWrite(PIN_DOT_THREE, LOW);
        digitalWrite(PIN_DOT_FOUR, LOW);
        break;
      // Two segments, starting from outside and criss-crossing
      case 4:
        for (int x = 1, y = 6; (x <= 5) && (y >= 2); x = x + 2, y = y - 2) {
          for (int i = 0; i < 10; ++i) {
            disableAllSegments();
            enableSegments(x, i);
            enableSegments(y, i);
            enableAllDots();
            updateDisplay();
            delay(50);
          }
        }
        disableAllSegments();
        digitalWrite(PIN_DOT_THREE, LOW);
        digitalWrite(PIN_DOT_FOUR, LOW);
        updateDisplay();
        break;
      // Two segments, starting from outside and finishing in the middle
      case 5:
        for (int x = 1, y = 6; (x <= 3) && (y >= 4); ++x, --y) {
          for (int i = 0; i < 10; ++i) {
            disableAllSegments();
            enableSegments(x, i);
            enableSegments(y, i);
            enableAllDots();
            updateDisplay();
            delay(50);
          }
        }
        disableAllSegments();
        digitalWrite(PIN_DOT_THREE, LOW);
        digitalWrite(PIN_DOT_FOUR, LOW);
        updateDisplay();
        break;
    } 
  }
}
