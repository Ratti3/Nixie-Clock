#include "Fade.h"

Fade::Fade() {}

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
  analogWrite(PIN_HV_BL, 100);
}

/*
  
  digitalWrite(PIN_HV_EN, LOW);
  delay(1000);

*/
