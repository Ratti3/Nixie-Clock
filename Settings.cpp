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
FlashStorage(fsLED, savedLED);                // Stores Switch LED1-3 PWM value
FlashStorage(fsSpin, savedSpin);              // Stores Nixie Hourly Spin Cycles
FlashStorage(fsLux, savedLux);                // Stores the low Lux level threshold value
FlashStorage(fsName, savedName);              // Stores the WebUI Titles

Settings::Settings() {
}

// Load all saved settings from flash
void Settings::begin() {
  rwSettings(10, 0);
  rwSettings(13, 0);
  for (byte i = 15; i <= 30; i++) {
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
        SPL("[DEBUG] Spin settings have been saved");
      } else {
        if (savedspin.validSpin) {
          flashSpin = savedspin.flashSpin;
          SP("Read: ");
          SPL(flashSpin);
        } else {
          SPL("No flashSpin Value!");
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
        SPL("[DEBUG] NTP settings have been saved");
      } else {
        if (savedntp.validNTP) {
          flashNTP = savedntp.flashNTP;
          SP("Read: ");
          SPL(flashNTP);
        } else {
          SPL("No flashNTP Value!");
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
        SPL("[DEBUG] PIR settings have been saved");
      } else {
        if (savedpir.validPIR) {
          flashPIR = savedpir.flashPIR;
          SP("Read: ");
          SPL(flashPIR);
        } else {
          SPL("No flashPIR Value!");
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
        SPL("[DEBUG] Light settings have been saved");
      } else {
        if (savedlight.validLight) {
          flashLight = savedlight.flashLight;
          SP("Read: ");
          SPL(flashLight);
        } else {
          SPL("No flashLight Value!");
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
        SPL("[DEBUG] Power settings have been saved");
      } else {
        if (savedusb.validUSB) {
          flashUSB = savedusb.flashUSB;
          SP("Read: ");
          SPL(flashUSB);
        } else {
          SPL("No flashUSB Value!");
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
        SPL("[DEBUG] Brightness settings have been saved");
      } else {
        if (savedbrightness.validBrightness) {
          flashBrightness = savedbrightness.flashBrightness;
          SP("Read: ");
          SPL(flashBrightness);
        } else {
          SPL("No flashBrightness Value!");
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
        SPL("[DEBUG] UTC Offset settings have been saved");
      } else {
        if (savedutcoffset.validUTCOffset) {
          flashUTCOffset = savedutcoffset.flashUTCOffset;
          SP("Read: ");
          SPL(flashUTCOffset);
        } else {
          SPL("No flashUTCOffset Value!");
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
        SPL("[DEBUG] On/Off Hour settings have been saved");
      } else {
        if (savedonoffhour.validOnOffHour) {
          flashOnHour = savedonoffhour.flashOnHour;
          flashOffHour = savedonoffhour.flashOffHour;
          SP("Read: ");
          SPL(flashOnHour);
          SPL(flashOffHour);
        } else {
          SPL("No flashOnHour/flashOffHour Value!");
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
        SPL("[DEBUG] NTP Pool settings have been saved");
      } else {
        if (savedntppool.validNTPPool) {
          flashNTPPool = savedntppool.flashNTPPool;
          SP("Read: ");
          SPL(flashNTPPool);
        } else {
          SPL("No NTP Pool Value!");
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
        SPL("[DEBUG] WebUI font settings have been saved");
      } else {
        if (savedfont.validFont) {
          flashFont = savedfont.flashFont;
          SP("Read: ");
          SPL(flashFont);
        } else {
          SPL("No flashFont Value!");
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
        SPL("[DEBUG] WebUI background settings have been saved");
      } else {
        if (savedbackground.validBackground) {
          flashBackground = savedbackground.flashBackground;
          SP("Read: ");
          SPL(flashBackground);
        } else {
          SPL("No flashBackground Value!");
        }
      }
      break;
    case 26: // Change WiFi
      savedWiFi savedwifi;
      savedwifi = fsWiFi.read();
    
      if (save) {
        flash_SSID.toCharArray(savedwifi.flash_SSID, 100);
        flash_PASS.toCharArray(savedwifi.flash_PASS, 100);
        savedwifi.valid = true;
        fsWiFi.write(savedwifi);
        SPL("[DEBUG] WiFi credentials have been saved");
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
        SPL("[DEBUG] Colon LED settings have been saved");
      } else {
        if (savedcolon.validColon) {
          flashColon = savedcolon.flashColon;
          SP("Read flashColon: ");
          SPL(flashColon);
        } else {
          SPL("No flashColon Value!");
        }
      }
      break;
    case 28:
      savedLED savedled;
      savedled = fsLED.read();
      if (save) {
        savedled.flashLED1 = flashLED1;
        savedled.flashLED2 = flashLED2;
        savedled.flashLED3 = flashLED3;
        savedled.validLED = true;
        fsLED.write(savedled);
        SPL("[DEBUG] LED1 settings have been saved");
      } else {
        if (savedled.validLED) {
          flashLED1 = savedled.flashLED1;
          flashLED2 = savedled.flashLED2;
          flashLED3 = savedled.flashLED3;
          SP("Read: ");
          SPL(flashLED1);
          SPL(flashLED2);
          SPL(flashLED3);
        } else {
          SPL("No flashLED1-3 Value!");
        }
      }
      break;
    case 29: // Lux threshold level
      savedLux savedlux;
      savedlux = fsLux.read();
      if (save) {
        savedlux.flashLux = flashLux;
        savedlux.validLux = true;
        fsLux.write(savedlux);
        SPL("[DEBUG] Lux settings have been saved");
      } else {
        if (savedlux.validLux) {
          flashLux = savedlux.flashLux;
          SP("Read: ");
          SPL(flashLux);
        } else {
          SPL("No flashLux Value!");
        }
      }
      break;
    case 30: // Change WebUI Title
      savedName savedname;
      savedname = fsName.read();
      if (save) {
        flashTitle.toCharArray(savedname.flashTitle, 50);
        flashName.toCharArray(savedname.flashName, 50);
        savedname.validName = true;
        fsName.write(savedname);
        SPL("[DEBUG] WebUI title has been saved");
          webTitle = savedname.flashTitle;
          webName = savedname.flashName;
      } else {
        if (savedname.validName) {
          webTitle = savedname.flashTitle;
          webName = savedname.flashName;
        } else {
          SPL("No WebUI Title Data Found");
        }
      }
      break;
  }
}

void Settings::debug(byte n) {
  switch (n) {
    case 0:
      SPL("[DEBUG] Load saved settings");
      break;
    case 1:
      SPL("[DEBUG] Begin HV5530");
      break;
    case 2:
      SPL("[DEBUG] Begin HV enable pins");
      break;
    case 3:
      SPL("[DEBUG] Begin I2C");
      break;
    case 4:
      SPL("[DEBUG] Could not find a valid DS3231 IC");
      break;
    case 5:
      SPL("[DEBUG] Found a valid DS3231 IC");
      break;
    case 6:
      SPL("[DEBUG] Could not find a valid BME280 sensor");
      break;
    case 7:
      SPL("[DEBUG] Found a valid BME280 sensor");
      break;
    case 8:
      SPL("[DEBUG] Could not find a valid VEML7700 sensor");
      break;
    case 9:
      SPL("[DEBUG] Foubd a valid VEML7700 sensor");
      break;
    case 10:
      SPL("[DEBUG] Connecting to WiFi");
      break;
    case 11:
      SPL("[DEBUG] Initialise NTP Client");
      break;
    case 12:
      SPL("[DEBUG] Initialise Web Server");
      break;
    case 13:
      SPL("[DEBUG] NTP Time Sync");
      break;
    case 14:
      SPL("[DEBUG] Display WiFi IP Address");
      break;
    case 15:
      SPL("[DEBUG] Saved WiFi credentials not found in flash");
      break;
    case 16:
      SPL("");
      break;
    case 17:
      SPL("");
      break;
  }
}
