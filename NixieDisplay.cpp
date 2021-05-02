#include "NixieDisplay.h"

NixieDisplay::NixieDisplay() {}

void NixieDisplay::begin() {
  // HV5530 Pins
  pinMode(PIN_HV_LE, OUTPUT);
  pinMode(PIN_HV_BL, OUTPUT);
  pinMode(PIN_HV_DATA, OUTPUT);
  pinMode(PIN_HV_CLK, OUTPUT);
  analogWrite(PIN_HV_BL, 100);

  // Transistor controlled IN-12B Decimal Points (tubes 3 and 4)
  pinMode(PIN_DOT_THREE, OUTPUT);
  pinMode(PIN_DOT_FOUR, OUTPUT);

  // LED Colon
  pinMode(PIN_COLON, OUTPUT);

  // 12V DC Booster Module Enable
  pinMode(PIN_SHDN, OUTPUT);
  digitalWrite(PIN_SHDN, HIGH);

  // HV Booster Module Enable
  pinMode(PIN_HV_EN, OUTPUT);
  
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

void NixieDisplay::runSlotMachine() {

  switch(random(3)) {
    case 0:
      for (int j = 0; j < 1; ++j) {
        for (int i = 0; i < 10; ++i) {
          disableAllSegments();
          for (byte x = 1; x <= 6; ++x) {
            enableSegments(x, i);
          }
          enableAllDots();
          updateDisplay();
          delay(250);
        }
      }
      disableAllSegments();
      digitalWrite(PIN_DOT_THREE, LOW);
      digitalWrite(PIN_DOT_FOUR, LOW);
      break;
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
  }
}
