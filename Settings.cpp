#include "Settings.h"

FlashStorage(fsWiFi, savedWiFi);              // Stores WiFi creds
FlashStorage(fsBrightness, savedBrightness);  // Stores
FlashStorage(fsNTP, savedNTP);                // Stores
FlashStorage(fsNTPPool, savedNTPPool);        // Stores
FlashStorage(fsPIR, savedPIR);                // Stores
FlashStorage(fsLight, savedLight);            // Stores 
FlashStorage(fsUTCOffset, savedUTCOffset);    // Stores UTC Offset
FlashStorage(fsUSB, savedUSB);                // Stores USB/DC power supply state
FlashStorage(fsFont, savedFont);              // Stores WebUI font settings
FlashStorage(fsBackground, savedBackground);  // Stores WebUI background settimgs

Settings::Settings() {}


void Settings::begin() {
  rwSettings(13, 0);
  for (byte i = 15; i <= 26; i++) {
    rwSettings(i, 0);
  }
}

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
          Serial.println("No flashNTP Value!");
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
          Serial.println("No flashPIR Value!");
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
          Serial.println("No flashLight Value!");
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
          Serial.println("No flashUSB Value!");
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
          Serial.println("No flashBrightness Value!");
        }
      }
      break;
    case 19: // UTC Offset
      savedUTCOffset savedutcoffset;
      savedutcoffset = fsUTCOffset.read();
      if (save) {
        savedutcoffset.flashUTCOffset = flashUTCOffset;
        savedutcoffset.validUTCOffset = true;
        fsUTCOffset.write(savedutcoffset);
        Serial.println("[DEBUG] UTC Offset settings have been saved");
      } else {
        if (savedutcoffset.validUTCOffset) {
          flashUTCOffset = savedutcoffset.flashUTCOffset;
          Serial.print("Read: ");
          Serial.println(flashUTCOffset);
        } else {
          Serial.println("No flashUTCOffset Value!");
        }
      }
      break;
    case 22: // Set on/off hour
      
      break;
    case 23: // Set NTP Pool
      savedNTPPool savedntppool;
      savedntppool = fsNTPPool.read();
      if (save) {
        savedntppool.flashNTPPool = flashNTPPool;
        savedntppool.validNTPPool = true;
        fsNTPPool.write(savedntppool);
        Serial.println("[DEBUG] NTP Pool settings have been saved");
      } else {
        if (savedntppool.validNTPPool) {
          flashNTPPool = savedntppool.flashNTPPool;
          Serial.print("Read: ");
          Serial.println(flashNTPPool);
        } else {
          Serial.println("No NTP Pool Value!");
        }
      }
      switch (flashNTPPool) {
        case 1:
          ntpServerName = "africa.pool.ntp.org";
          break;
        case 2:
          ntpServerName = "asia.pool.ntp.org";
          break;
        case 3:
          ntpServerName = "europe.pool.ntp.org";
          break;
        case 4:
          ntpServerName = "north-america.pool.ntp.org";
          break;
        case 5:
          ntpServerName = "oceania.pool.ntp.org";
          break;
        case 6:
          ntpServerName = "south-america.pool.ntp.org";
          break;
      }
      break;
    case 24: // Change WebUI Font
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
          Serial.println("No flashFont Value!");
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
    case 25: // Change WebUI Background
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
          Serial.println("No flashBackground Value!");
        }
      }
      break;
    case 26: // Change WiFi
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
    case 0:
      Serial.println("[DEBUG] Load saved settings");
      break;
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
