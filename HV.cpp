#include "HV.h"

HV::HV() {}

void HV::begin() {
  pinMode(PIN_HV_EN, OUTPUT);
  digitalWrite(PIN_HV_EN, HIGH); // There is now 170V on the board!
  Serial.println("170");
  _hvon = true;
}

bool HV::isOn() {
  return _hvon;
}

void HV::switchOn() {
  digitalWrite(PIN_HV_EN, HIGH);
   Serial.println("1702");
  _hvon = true;
}

void HV::switchOff() {
  digitalWrite(PIN_HV_EN, LOW);
   Serial.println("off");
  _hvon = false;
}
