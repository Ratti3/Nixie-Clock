#include "Settings.h"

FlashStorage(fsWiFi, savedWiFi);
FlashStorage(fsBrightness, savedBrightness);
FlashStorage(fsNTP, savedNTP);
FlashStorage(fsPIR, savedPIR);
FlashStorage(fsLight, savedLight);
FlashStorage(fsUSB, savedUSB);
FlashStorage(fsFont, savedFont);
FlashStorage(fsBackground, savedBackground);

Settings::Settings() {}

void Settings::rwSettings(byte setting, bool save) {
  switch (setting) {
    case 13: // Enable/Disable NTP
      savedNTP savedntp;
      savedntp = fsNTP.read();
      if (save) {
        savedntp.flashNTP = flashNTP;
        savedntp.validNTP = true;
        fsNTP.write(savedntp);
        Serial.println("[DEBUG] NTP settings have been saved");
      } else {
        if (savedntp.validNTP) {
          flashNTP = savedntp.flashNTP;
          Serial.print("Read: ");
          Serial.println(flashNTP);
        } else {
          Serial.println("No Value!");
        }
      }
      break;
    case 15: // Enable/Disable PIR
      savedPIR savedpir;
      savedpir = fsPIR.read();
      if (save) {
        savedpir.flashPIR = flashPIR;
        savedpir.validPIR = true;
        fsPIR.write(savedpir);
        Serial.println("[DEBUG] PIR settings have been saved");
      } else {
        if (savedpir.validPIR) {
          flashPIR = savedpir.flashPIR;
          Serial.print("Read: ");
          Serial.println(flashPIR);
        } else {
          Serial.println("No Value!");
        }
      }
      break;
    case 16: // Enable Disable Light Sensor
      savedLight savedlight;
      savedlight = fsLight.read();
      if (save) {
        savedlight.flashLight = flashLight;
        savedlight.validLight = true;
        fsLight.write(savedlight);
        Serial.println("[DEBUG] Light settings have been saved");
      } else {
        if (savedlight.validLight) {
          flashLight = savedlight.flashLight;
          Serial.print("Read: ");
          Serial.println(flashLight);
        } else {
          Serial.println("No Value!");
        }
      }
      break;
    case 17: // Switch between 5V and 12V
      savedUSB savedusb;
      savedusb = fsUSB.read();
      if (save) {
        savedusb.flashUSB = flashUSB;
        savedusb.validUSB = true;
        fsUSB.write(savedusb);
        Serial.println("[DEBUG] Power settings have been saved");
      } else {
        if (savedusb.validUSB) {
          flashUSB = savedusb.flashUSB;
          Serial.print("Read: ");
          Serial.println(flashUSB);
        } else {
          Serial.println("No Value!");
        }
      }
      break;
    case 18: // Set brightness
      savedBrightness savedbrightness;
      savedbrightness = fsBrightness.read();
      if (save) {
        savedbrightness.flashBrightness = flashBrightness;
        savedbrightness.validBrightness = true;
        fsBrightness.write(savedbrightness);
        Serial.println("[DEBUG] Brightness settings have been saved");
      } else {
        if (savedbrightness.validBrightness) {
          flashBrightness = savedbrightness.flashBrightness;
          Serial.print("Read: ");
          Serial.println(flashBrightness);
        } else {
          Serial.println("No Value!");
        }
      }
      break;
    case 19: // UTC Offset
      
      break;
    case 22: // Set on/off hour
      
      break;
    case 23: // Change WebUI Font
      savedFont savedfont;
      savedfont = fsFont.read();
      if (save) {
        savedfont.flashFont = flashFont;
        savedfont.validFont = true;
        fsFont.write(savedfont);
        Serial.println("[DEBUG] WebUI font settings have been saved");
      } else {
        if (savedfont.validFont) {
          flashFont = savedfont.flashFont;
          Serial.print("Read: ");
          Serial.println(flashFont);
        } else {
          Serial.println("No Value!");
        }
      }
      Serial.println(flashFont);
      switch (flashFont) {
        case 1:
          webFont = "Audiowide";
          break;
        case 2:
          webFont = "Codystar";
          break;
        case 3:
          webFont = "Gugi";
          break;
        case 4:
          webFont = "Michroma";
          break;
        case 5:
          webFont = "Nova Round";
          break;
        case 6:
          webFont = "Orbitron";
          break;
        case 7:
          webFont = "Revalia";
          break;
        case 8:
          webFont = "Slackey";
          break;
        default:
          webFont = "Audiowide";
          break;
      }
      break;
    case 24: // Change WebUI Background
      savedBackground savedbackground;
      savedbackground = fsBackground.read();
      if (save) {
        savedbackground.flashBackground = flashBackground;
        savedbackground.validBackground = true;
        fsBackground.write(savedbackground);
        Serial.println("[DEBUG] WebUI background settings have been saved");
      } else {
        if (savedbackground.validBackground) {
          flashBackground = savedbackground.flashBackground;
          Serial.print("Read: ");
          Serial.println(flashBackground);
        } else {
          Serial.println("No Value!");
        }
      }
      break;
    case 25: // Change WiFi
      savedWiFi savedwifi;
      savedwifi = fsWiFi.read();
    
      if (save) {
        flash_SSID.toCharArray(savedwifi.flash_SSID, 50);
        flash_PASS.toCharArray(savedwifi.flash_PASS, 50);
        savedwifi.valid = true;
        fsWiFi.write(savedwifi);
        Serial.println("[DEBUG] WiFi credentials have been saved");
      } else {
        if (savedwifi.valid) {
          ssid = savedwifi.flash_SSID;
          pass = savedwifi.flash_PASS;
          noSSID = 0;
        } else {
          noSSID = 1;
          debug(15);
        }
      }
      break;
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
