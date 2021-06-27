#include "HV.h"

HV::HV() {}

void HV::begin(bool USBpower) {
  // 12V DC Booster Module Enable
  pinMode(PIN_SHDN, OUTPUT);
  if (USBpower) {
    digitalWrite(PIN_SHDN, HIGH);
  } else {
    digitalWrite(PIN_SHDN, LOW);
  }

  // HV Booster Module Enable
  pinMode(PIN_HV_EN, OUTPUT);
  digitalWrite(PIN_HV_EN, HIGH);
  _hvon = true;
}

// Store the state of the HV on/off logic
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

void HV::switch12VOn() {
  digitalWrite(PIN_SHDN, HIGH);
}

void HV::switch12VOff() {
  digitalWrite(PIN_SHDN, LOW);
}
