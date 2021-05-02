#include "Settings.h"

FlashStorage(wifi_flash_store, wifiCreds);

Settings::Settings() {}

void Settings::wifiCredentials(bool save) {
  wifiCreds wificreds;
  wificreds = wifi_flash_store.read();

  if (save) {
    // Fill the "wificreds" structure with the data entered by the user
    flash_SSID.toCharArray(wificreds.flash_SSID, 100);
    flash_PASS.toCharArray(wificreds.flash_PASS, 100);
    // set "valid" to true, so the next time we know that we have valid data inside
    wificreds.valid = true;
    // Save to the "wifi_flash_store"
    wifi_flash_store.write(wificreds);
    Serial.println("[DEBUG] WiFi credentials have been saved");
  } else {
    if (wificreds.valid) {
      ssid = wificreds.flash_SSID;
      pass = wificreds.flash_PASS;
      noSSID = 0;
    } else {
      noSSID = 1;
      debug(15);
    }
  }
}

void Settings::debug(byte n) {
  switch (n) {
    case 1:
      Serial.println("[DEBUG] Begin HV5530");
      break;
    case 2:
      Serial.println("[DEBUG] Begin HV enable pins");
      break;
    case 3:
      Serial.println("[DEBUG] Begin I2C");
      break;
    case 4:
      Serial.println("[DEBUG] Could not find a valid DS3231 IC");
      break;
    case 5:
      Serial.println("[DEBUG] Found a valid DS3231 IC");
      break;
    case 6:
      Serial.println("[DEBUG] Could not find a valid BME280 sensor");
      break;
    case 7:
      Serial.println("[DEBUG] Found a valid BME280 sensor");
      break;
    case 8:
      Serial.println("[DEBUG] Could not find a valid VEML7700 sensor");
      break;
    case 9:
      Serial.println("[DEBUG] Foubd a valid VEML7700 sensor");
      break;
    case 10:
      Serial.println("[DEBUG] Connecting to WiFi");
      break;
    case 11:
      Serial.println("[DEBUG] Initialise NTP Client");
      break;
    case 12:
      Serial.println("[DEBUG] Initialise Web Server");
      break;
    case 13:
      Serial.println("[DEBUG] NTP Time Sync");
      break;
    case 14:
      Serial.println("[DEBUG] Display WiFi IP Address");
      break;
    case 15:
      Serial.println("[DEBUG] Saved WiFi credentials not found in flash");
      break;
    case 16:
      Serial.println("");
      break;
    case 17:
      Serial.println("");
      break;
  }
}
