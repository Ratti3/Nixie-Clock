#include "HV.h"

HV::HV() {}

void HV::begin(bool USBpower) {
  // 12V DC Booster Module Enable
  pinMode(PIN_SHDN, OUTPUT);
  if (USBpower) {
    digitalWrite(PIN_SHDN, HIGH);
    _12von = true;
  } else {
    digitalWrite(PIN_SHDN, LOW);
    _12von = false;
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

// Store the state of the 12V on/off logic
bool HV::is12VOn() {
  return _12von;
}

void HV::switch12VOn() {
  digitalWrite(PIN_SHDN, HIGH);
  _12von = true;
}

void HV::switch12VOff() {
  digitalWrite(PIN_SHDN, LOW);
  _12von = false;
}
