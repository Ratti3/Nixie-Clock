#include "Settings.h"

FlashStorage(fsWiFi, savedWiFi);              // Stores WiFi creds
FlashStorage(fsBrightness, savedBrightness);  // Stores
FlashStorage(fsNTP, savedNTP);                // Stores
FlashStorage(fsNTPPool, savedNTPPool);        // Stores
FlashStorage(fsOnOffHour, savedOnOffHour);    // 
FlashStorage(fsPIR, savedPIR);                // Stores
FlashStorage(fsLight, savedLight);            // Stores 
FlashStorage(fsUTCOffset, savedUTCOffset);    // Stores UTC Offset
FlashStorage(fsUSB, savedUSB);                // Stores USB/DC power supply state
FlashStorage(fsFont, savedFont);              // Stores WebUI font settings
FlashStorage(fsBackground, savedBackground);  // Stores WebUI background settings
FlashStorage(fsColon, savedColon);            // Stores Colon LED PWM value
FlashStorage(fsLED1, savedLED1);              // Stores Switch LED1 PWM value
FlashStorage(fsLED2, savedLED2);              // Stores Switch LED2 PWM value
FlashStorage(fsLED3, savedLED3);              // Stores Switch LED3 PWM value
FlashStorage(fsSpin, savedSpin);              // Stores Nixie Hourly Spin Cycles

Settings::Settings() {}

// Load all saved settings from flash
void Settings::begin() {
  rwSettings(10, 0);
  rwSettings(13, 0);
  for (byte i = 15; i <= 28; i++) {
    rwSettings(i, 0);
  }
}

void Settings::rwSettings(byte setting, bool save) {
  switch (setting) {
    case 10: // Set Nixie Hourly Spin Cycles
      savedSpin savedspin;
      savedspin = fsSpin.read();
      if (save) {
        savedspin.flashSpin = flashSpin;
        savedspin.validSpin = true;
        fsSpin.write(savedspin);
        Serial.println("[DEBUG] Spin settings have been saved");
      } else {
        if (savedspin.validSpin) {
          flashSpin = savedspin.flashSpin;
          Serial.print("Read: ");
          Serial.println(flashSpin);
        } else {
          Serial.println("No flashSpin Value!");
        }
      }
      break;
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
      savedOnOffHour savedonoffhour;
      savedonoffhour = fsOnOffHour.read();
      if (save) {
        savedonoffhour.flashOnHour = flashOnHour;
        savedonoffhour.flashOffHour = flashOffHour;
        savedonoffhour.validOnOffHour = true;
        fsOnOffHour.write(savedonoffhour);
        Serial.println("[DEBUG] On/Off Hour settings have been saved");
      } else {
        if (savedonoffhour.validOnOffHour) {
          flashOnHour = savedonoffhour.flashOnHour;
          flashOffHour = savedonoffhour.flashOffHour;
          Serial.print("Read: ");
          Serial.println(flashOnHour);
          Serial.println(flashOffHour);
        } else {
          Serial.println("No flashOnHour/flashOffHour Value!");
        }
      }
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
    case 27:
      savedColon savedcolon;
      savedcolon = fsColon.read();
      if (save) {
        savedcolon.flashColon = flashColon;
        savedcolon.validColon = true;
        fsColon.write(savedcolon);
        Serial.println("[DEBUG] Colon LED settings have been saved");
      } else {
        if (savedcolon.validColon) {
          flashColon = savedcolon.flashColon;
          Serial.print("Read flashColon: ");
          Serial.println(flashColon);
        } else {
          Serial.println("No flashColon Value!");
        }
      }
      break;
    case 28:
      savedLED1 savedled1;
      savedLED2 savedled2;
      savedLED3 savedled3;
      savedled1 = fsLED1.read();
      savedled2 = fsLED2.read();
      savedled3 = fsLED3.read();
      if (save) {
        savedled1.flashLED1 = flashLED1;
        savedled2.flashLED2 = flashLED2;
        savedled3.flashLED3 = flashLED3;
        savedled1.validLED1 = true;
        savedled2.validLED2 = true;
        savedled3.validLED3 = true;
        fsLED1.write(savedled1);
        Serial.println("[DEBUG] LED1 settings have been saved");
        fsLED2.write(savedled2);
        Serial.println("[DEBUG] LED2 settings have been saved");
        fsLED3.write(savedled3);
        Serial.println("[DEBUG] LED3 settings have been saved");
      } else {
        if (savedled1.validLED1 && savedled2.validLED2 && savedled3.validLED3) {
          flashLED1 = savedled1.flashLED1;
          flashLED2 = savedled2.flashLED2;
          flashLED3 = savedled3.flashLED3;
          Serial.print("Read: ");
          Serial.println(flashLED1);
          Serial.println(flashLED2);
          Serial.println(flashLED3);
        } else {
          Serial.println("No flashLED1-3 Value!");
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
