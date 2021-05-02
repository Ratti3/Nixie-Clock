#include "HV.h"

HV::HV() {}

void HV::begin() {
  pinMode(PIN_HV_EN, OUTPUT);
  // Turn on 170V
  digitalWrite(PIN_HV_EN, HIGH);
  _hvon = true;
}

bool HV::isOn() {
  return _hvon;
}

void HV::switchOn() {
  digitalWrite(PIN_HV_EN, HIGH);
  _hvon = true;
}

void HV::switchOff() {
  digitalWrite(PIN_HV_EN, LOW);
  _hvon = false;
}
