#include "WiFiTask.h"

WiFiTask::WiFiTask(NixieDisplay* nixie, TimeTask* timetask, I2C* i2c, NTPClient* timeclient, WiFiServer* server, Settings* settings, Fade* fade) {
  _nixie = nixie;
  _timetask = timetask;
  _i2c = i2c;
  _timeclient = timeclient;
  _server = server;
  _settings = settings;
  _fade = fade;
}

// Connects WiFi
void WiFiTask::connectWiFi() {

  // Get saved credentials from flash
  _settings->wifiCredentials(0);
  if (_settings->noSSID) {
    WiFiFail = 1;
    return;
  }

  count = 0;
  WiFiFail = 0;

  Serial.print("Connecting to SSID: ");
  Serial.print(_settings->ssid);
  Serial.print("..");

  while (WiFi.status() != WL_CONNECTED) {
    _fade->fadeIn();
    WiFi.begin(_settings->ssid, _settings->pass);
    _fade->fadeOut();
    count++;
    if (count > 3) {
      Serial.println(" ");
      Serial.print("Failed to connect to WiFi network: ");
      Serial.println(_settings->ssid);
      Serial.println(" ");
      WiFiFail = 1;
      break;
    }
  }

  if (!WiFiFail) {
    Serial.println("..connected");
    printWifiStatus();
  }

}

void WiFiTask::ntpBegin() {

  _timeclient->begin();

}

void WiFiTask::serverBegin() {

  _server->begin();

}

// Main NTP function
void WiFiTask::getNTP() {

  // Holds the total attempts so far
  byte retry_count = 1;
  
  while (retry_count <= retry_max) {
    if (WiFiFail) {
      break;
    }
    Serial.print("[Attempt ");
    Serial.print(retry_count);
    Serial.print(" of ");
    Serial.print(retry_max);
    Serial.println("]");
    
    // Send UDP packet process the reply
    getNTPTime();
    
    retry_count++;
    
    if (cb) {
      break;
    }
    
    if (retry_count > retry_max) {
      Serial.println(" ");
      Serial.println("NTP Request Fail");
      Serial.println(" ");
    }
  }

}


// NTP routine for sending UDP packet and receiving UNIX time and setting the DS3231 time
void WiFiTask::getNTPTime() {
  count = 1;
  int i = 0;
  Serial.print("Sending NTP request attempt ");
  Serial.print(count);
  Serial.print(" of ");
  Serial.println(_settings->try_count);
  _timeclient->update();
  while (!cb) {
    if (count >= _settings->try_count) {
      break;
    }
    _fade->fadeIn();
    if (i == 400) { //resend NTP packet every 2 seconds
      count++;
      _timeclient->update();
      Serial.print("Sending NTP request attempt ");
      Serial.print(count);
      Serial.print(" of ");
      Serial.println(_settings->try_count);
      delay(5);
      i = 0;
    }
    _fade->fadeOut();
    delay(5);
    if (_timeclient->getEpochTime() > 100000) {
      _i2c->adjustTime(_timeclient->getEpochTime());
      cb = 1;
      Serial.print(" with UNIX Time: ");
      Serial.println(_timeclient->getEpochTime());
    }
    i++;
  }
  
} //// End void getNTPTime()

// Prints WiFi connection info
void WiFiTask::printWifiStatus() {

  // Print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());
  // Get the IP address
  IPAddress ip = WiFi.localIP();
  // Store the IP address for later
  ipAddress[0] = ip[0];
  ipAddress[1] = ip[1];
  ipAddress[2] = ip[2];
  ipAddress[3] = ip[3];
  // Print your WiFi IP address:
  Serial.print(", IP Address: ");
  Serial.println(ip);
  // Print the signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal Strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

}


void WiFiTask::displayIP() {

  byte ip1, ip2, ip3, ip4, ip5, ip6;
  for (byte i = 0; i <= 1; i++) {
    byte a = ipAddress[i + i];
    byte b = ipAddress[i + i + 1];
    if (a > 99) {
      ip1 = (a/100)%10;
      ip2 = (a/10)%10;
      ip3 = a%10;
    } else if (a > 9) {
      ip1 = 0;
      ip2 = (a/10)%10;
      ip3 = a%10;
    } else if (a > 0) {
      ip1 = 0;
      ip2 = 0;
      ip3 = a%10;
    } else {
      ip1 = 0;
      ip2 = 0;
      ip3 = 0;
    }
    if (b > 99) {
      ip4 = (b/100)%10;
      ip5 = (b/10)%10;
      ip6 = b%10;
    } else if (b > 9) {
      ip4 = 0;
      ip5 = (b/10)%10;
      ip6 = b%10;
    } else if (b > 0) {
      ip4 = 0;
      ip5 = 0;
      ip6 = b%10;
    } else {
      ip4 = 0;
      ip5 = 0;
      ip6 = 0;
    }

    // Display the IP Address
    // Enable and disable the right segments
    _nixie->disableSegments(hourTens, 10);
    _nixie->disableSegments(hourUnits, 10);
    _nixie->disableSegments(minuteTens, 10);
    _nixie->disableSegments(minuteUnits, 10);
    _nixie->disableSegments(secondTens, 10);
    _nixie->disableSegments(secondUnits, 10);

    _nixie->enableSegment(hourTens[ip1]);
    _nixie->enableSegment(hourUnits[ip2]);
    _nixie->enableSegment(minuteTens[ip3]);
    _nixie->enableSegment(minuteUnits[ip4]);
    _nixie->enableSegment(secondTens[ip5]);
    _nixie->enableSegment(secondUnits[ip6]);

    // Write to display
    _nixie->updateDisplay();

    delay(2000);
  }

}

// Web server for interacting with the Nano 33 IoT
void WiFiTask::clientServer() {

  // Listen for incoming client connections
  WiFiClient client = _server->available();   

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<html><head><style>");
            client.print("div {text-align: center; font-family: sans-serif;}");
            client.print("label {font-size: 40px;}");
            client.print("</style></head><body>");
            client.print("<div><h2>Arduino Nano 33 IoT Nixie Clock : Living Room</h2>");
//            if (displayOff) {
              client.print("<a href=\"/02\"><img src='https://drive.google.com/uc?id=1dGVztV0ahnLaB_RNltlMo3H0TSPJ-ATU' style='padding:10px' /></a>"); // Display On
//            } else {
              client.print("<a href=\"/01\"><img src='https://drive.google.com/uc?id=1xbmeCXErmswAt3xh42ooL4HaqfX4cYVQ' style='padding:10px' /></a>"); // Display Off
//            }
            client.print("<a href=\"/03\"><img src='https://drive.google.com/uc?id=1qGAOFwhd63jFJXEMi4QyQXEUwAsPzdcu' style='padding:10px' /></a><br>");   // NTP
            client.print("<a href=\"/04\"><img src='https://drive.google.com/uc?id=1HbvlF_ACW4zPGfcpgxSn2TPsJy-SI8cF' style='padding:10px' /></a>");   // Pattern
            client.print("<a href=\"/05\"><img src='https://drive.google.com/uc?id=1F4nhD4GAediuS67L6H_8wDDK95n5MvWQ' style='padding:10px' /></a><br>");   // Temp
            client.print("<a href=\"/06\"><img src='https://drive.google.com/uc?id=1CFa38_ViMUeKnZFmLsKrr1aPov9G-0_t' style='padding:10px' /></a><br>");   // Date          

            client.print("<a href='https://www.youtube.com/Ratti3'><img src='https://drive.google.com/uc?id=1Tv8hR-kJAJrHA-2Ls3I5tKF5iBxXHeG6' style='padding:10px' /></a> ");
            client.print("<a href='https://ratti3.blogspot.com'><img src='https://drive.google.com/uc?id=1rV_gw0rkG-NPXOBOwKx_3fO1hXbctU80' style='padding:10px' /></a> ");
            client.print("<a href='https://easyeda.com/Ratti3'><img src='https://drive.google.com/uc?id=1DCPnmIjOvhmMe5kVEDUbUmj8hHiEav33' style='padding:10px' /></a>");
            client.print("<br>&copy; 2021 : Ratti3 Technologies Corp</span></div>");
            client.print("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        // Check the client requests
        if (currentLine.endsWith("GET /01")) {
 //         displayOff = 1;
            startAP();
            digitalWrite(PIN_HV_LE, HIGH);
        } else if (currentLine.endsWith("GET /02")) {
//          displayOff = 0;
            digitalWrite(PIN_HV_LE, LOW);
        } else if (currentLine.endsWith("GET /03")) {
            getNTP();
        } else if (currentLine.endsWith("GET /04")) {
 //         spin(random(patterns));
            _nixie->runSlotMachine();
        } else if (currentLine.endsWith("GET /05")) {
            _i2c->displayTHP();
        } else if (currentLine.endsWith("GET /06")) {
            _timetask->showDate();
        } else if (currentLine.startsWith("GET /?b=")) {

                    String bright_norm;
                    bright_norm = urlDecode(currentLine.substring(14, currentLine.indexOf('&'))); 
                    Serial.println(bright_norm);

                    String bright_lo;
                    bright_lo = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')));
                    Serial.println(bright_lo);

        }
      }
    }
    client.stop();
  }
  
}

void WiFiTask::startAP() {
  
  Serial.print("Creating access point named: ");
  Serial.println(ssid_ap);

  // Create open network. Change this line if you want to create an WEP network:
  int status = WiFi.beginAP(ssid_ap, pass_ap);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    apStatus = 0;
  } else {
    apStatus = 1;
    _settings->debug(12);
    serverBegin();
    _settings->debug(14);
    printWifiStatus();
    _settings->debug(14);
    displayIP(); 
  }
}


void WiFiTask::apServer() {
  // Listen for incoming client connections
  WiFiClient client = _server->available();   

  if (client) {
    String currentLine = "";
    bool gotCreds = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<html><head><style>");
            client.print("div {text-align: center; font-family: sans-serif;}");
            client.print("input[type=submit] {background-color: #4CAF50; border: none; color: white; padding: 16px 32px; text-decoration: none; margin: 4px 2px; width: 400px; height: 80px; font-size: 40px;}");
            client.print("input[type=text], [type=password] { width: 400px; padding: 12px 20px; margin: 8px 0; box-sizing: border-box; height: 80px; border: 4px solid red; border-radius: 10px; font-size: 40px;}");
            client.print("label {font-size: 40px;}");
            client.print("table {margin: auto;}");
            client.print("</style></head><body>");
            client.print("<div>");
            client.print("<h2>Arduino Nano 33 IoT Nixie Clock : Living Room</h2>");
            client.print("<h3>WiFi parameters:</h3><br>");
            client.print("<form action='ap' method='get'>");
            client.print("<table><tr><th><label>SSID: </label></th><th><label>Password: </label></th></tr>");
            client.print("<tr><td><input type='text' name='ssid'></td><td><input type='password' name='password'></td></tr></table>");
            client.print("<input type='submit' value='Save &amp; Connect'>");
            client.print("</form>");
            client.print("<br>&copy; 2021 : Ratti3 Technologies Corp</div>");
            client.print("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        // Check the client requests
        if (currentLine.startsWith("GET /ap?ssid=")) {
          _settings->flash_SSID = urlDecode(currentLine.substring(13, currentLine.indexOf('&'))); 
          _settings->flash_PASS = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')));
          gotCreds = 1;
        }
      }
    }
    client.stop();
    if (gotCreds) {
      Serial.println("[DEBUG] Got the new WiFi credentials, saving to flash and restarting WiFi");
      _settings->wifiCredentials(1);
      // Stop the current AP SSID
      WiFi.disconnect();
      connectWiFi();
      _settings->debug(13);
      getNTP();
      _settings->debug(14);
      displayIP();
    }
  }
}

// Decodes URL and grabs the values
String WiFiTask::urlDecode(const String& text) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = text.length();
  unsigned int i = 0;
  while (i < len) {
    char decodedChar;
    char encodedChar = text.charAt(i++);
    if ((encodedChar == '%') && (i + 1 < len)) {
      temp[2] = text.charAt(i++);
      temp[3] = text.charAt(i++);
      decodedChar = strtol(temp, NULL, 16);
    } else {
      if (encodedChar == '+') {
        decodedChar = ' ';
      } else {
        decodedChar = encodedChar;
      }
    }
    decoded += decodedChar;
  }
  return decoded;
}
