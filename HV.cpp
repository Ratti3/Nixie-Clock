#include "HV.h"

HV::HV() {}

void HV::begin(bool USBpower) {
  // 12V DC Booster Module Enable
  pinMode(PIN_SHDN, OUTPUT);
  SP("Turning 5 to 12V Booster ");
  if (USBpower) {
    digitalWrite(PIN_SHDN, HIGH);
    SPL("On");
  } else {
    digitalWrite(PIN_SHDN, LOW);
    SPL("Off");
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
  _hvon = 1;
}

void HV::switchOff() {
  digitalWrite(PIN_HV_EN, LOW);
  _hvon = 0;
}

void HV::switch12VOn() {
  digitalWrite(PIN_SHDN, HIGH);
  SPL("Turning 5 to 12V Booster On");
}

void HV::switch12VOff() {
  digitalWrite(PIN_SHDN, LOW);
  SPL("Turning 5 to 12V Booster Off");
}
