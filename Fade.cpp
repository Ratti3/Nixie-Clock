#include "Fade.h"

Fade::Fade(Settings* settings) {
  _settings = settings;
}

void Fade::fadeIn() {
  for(uint8_t i = 0; i != 255; i += 1 ){
    analogWrite(PIN_COLON, i);
    delay(2);
  }
}

void Fade::fadeOut() {
  for(uint8_t i = 255; i != 0; i -= 1){
    analogWrite(PIN_COLON, i);
    delay(2);
  }
}

void Fade::setBrightness() {
  analogWrite(PIN_HV_BL, _settings->flashBrightness);
}
