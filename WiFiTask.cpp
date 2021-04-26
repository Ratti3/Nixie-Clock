#include "WiFiTask.h"

WiFiTask::WiFiTask(NixieDisplay* nixie, I2C* i2c, NTPClient* timeclient, Settings* settings) {
  _nixie = nixie;
  _i2c = i2c;
  _timeclient = timeclient;
  _settings = settings;
}


void WiFiTask::begin() {
  _timeclient->begin();
}

// Main NTP function
void WiFiTask::getNTP() {

  // Holds the total attempts so far
  byte retry_count = 1;

  // Start the WiFi
  connectWiFi();
  
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





// Connects WiFi
void WiFiTask::connectWiFi() {
  count = 0;
  WiFiFail = 0;
  Serial.print("Connecting to SSID: ");
  Serial.print(ssid);
  Serial.print("..");
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
//    displayOther(1, 10, 10, 10, 10, 10, 10);
    count++;
    if (count > 3) {
      Serial.println(" ");
      Serial.print("Fail to connect to WiFi network: ");
      Serial.println(ssid);
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
    } else if (b > 1) {
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

Serial.print("A ");
Serial.println(ip1);
Serial.print("B ");
Serial.println(ip2);
Serial.print("C ");
Serial.println(ip3);
Serial.print("D ");
Serial.println(ip4);
Serial.print("E ");
Serial.println(ip5);
Serial.print("F ");
Serial.println(ip6);
    delay(1000);

  }

}
