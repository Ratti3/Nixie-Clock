#include "Fade.h"

Fade::Fade(Settings* settings) {
  _settings = settings;
}

void Fade::start() {
  // LED Colon
  pinMode(PIN_COLON, OUTPUT);

  // LED Switches
  pinMode(PIN_SW1_LED1, OUTPUT);
  pinMode(PIN_SW2_LED2, OUTPUT);
  pinMode(PIN_SW3_LED3, OUTPUT);

  // Red (1st LED)
  analogWrite(PIN_SW1_LED1, _settings->flashLED1);
  // Green (2nd LED)
  analogWrite(PIN_SW2_LED2, _settings->flashLED2);
  // Blue (3rd LED)
  analogWrite(PIN_SW3_LED3, _settings->flashLED3);
}

void Fade::fadeIn() {
  for (uint8_t i = 0; i != 255; i += 1) {
    analogWrite(PIN_COLON, i);
    delay(2);
  }
}

void Fade::fadeOut() {
  for (uint8_t i = 255; i != 0; i -= 1) {
    analogWrite(PIN_COLON, i);
    delay(2);
  }
}

void Fade::setBrightness() {
  analogWrite(PIN_HV_BL, _settings->flashBrightness);
}

void Fade::setSwitchLEDBrightness() {
  // Red (1st LED)
  analogWrite(PIN_SW1_LED1, _settings->flashLED1);
  // Green (2nd LED)
  analogWrite(PIN_SW2_LED2, _settings->flashLED2);
  // Blue (3rd LED)
  analogWrite(PIN_SW3_LED3, _settings->flashLED3);
}

void Fade::switchLEDFlash(byte LED) {
  switch (LED) {
    case 1:
      for (byte i = 0; i < 2; i++) {
        analogWrite(PIN_SW1_LED1, 0);
        delay(50);
        analogWrite(PIN_SW1_LED1, _settings->flashLED1);
        delay(50);
      }
      break;
    case 2:
      for (byte i = 0; i < 2; i++) {
        analogWrite(PIN_SW2_LED2, 0);
        delay(50);
        analogWrite(PIN_SW2_LED2, _settings->flashLED2);
        delay(50);
      }
      break;
    case 3:
      for (byte i = 0; i < 2; i++) {
        analogWrite(PIN_SW3_LED3, 0);
        delay(50);
        analogWrite(PIN_SW3_LED3, _settings->flashLED3);
        delay(50);
      }
      break;
  }
  setSwitchLEDBrightness();
}
