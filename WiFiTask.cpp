#include "WiFiTask.h"

#define CP(x) (client.print(x));
#define CPL(x) (client.println(x));

WiFiTask::WiFiTask(NixieDisplay* nixie, TimeTask* timetask, I2C* i2c, NTPClient* timeclient, WiFiServer* server, Settings* settings, Fade* fade, HV* hv) {
  _nixie = nixie;
  _timetask = timetask;
  _i2c = i2c;
  _timeclient = timeclient;
  _server = server;
  _settings = settings;
  _fade = fade;
  _hv = hv;
}

// Connects WiFi
void WiFiTask::connectWiFi() {

  // Get saved credentials from flash
  _settings->rwSettings(25, 0);
  if (_settings->noSSID) {
    WiFiFail = 1;
    return;
  }

  count = 0;
  // Keep track of the WiFi connection status
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

// Start the NTP client
void WiFiTask::ntpBegin() {
  _timeclient->begin();
}

// Start the web server
void WiFiTask::serverBegin() {
  _server->begin();
}

// Main NTP function
void WiFiTask::getNTP() {

  _nixie->disableAllSegments();
  _nixie->updateDisplay();

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

// Displays the AP or connected networks IP address on the Nixies
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

  byte setting = 0;   // Holds run state of the clicked buttons
  byte repeat = 0; //

  // Listen for incoming client connections
  WiFiClient client = _server->available();   

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            CPL("HTTP/1.1 200 OK");
            CPL("Content-type:text/html");
            CPL();

            CP("<!DOCTYPE html>");
            CP("<html>");
            CP("<head>");

            CP("<link href='https://fonts.googleapis.com/css?family=Audiowide' rel='stylesheet'>");
            CP("<link href='https://fonts.googleapis.com/css?family=Codystar' rel='stylesheet'>");
            CP("<link href='https://fonts.googleapis.com/css?family=Gugi' rel='stylesheet'>");
            CP("<link href='https://fonts.googleapis.com/css?family=Michroma' rel='stylesheet'>");
            CP("<link href='https://fonts.googleapis.com/css?family=Nova+Round' rel='stylesheet'>");
            CP("<link href='https://fonts.googleapis.com/css?family=Orbitron' rel='stylesheet'>");
            CP("<link href='https://fonts.googleapis.com/css?family=Revalia' rel='stylesheet'>");
            CP("<link href='https://fonts.googleapis.com/css?family=Slackey' rel='stylesheet'>");

            CP("<title>" + String(_settings->webName) + " : " + String(_settings->fwVersion) + "</title>");

            CP("<style>");

            switch (_settings->flashBackground) {
              case 1:
                // Cage
                CP("body {background-color: #ffffff; background-image: url(\"data:image/svg+xml,%3Csvg width='32' height='26' viewBox='0 0 32 26' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M14 0v3.994C14 7.864 10.858 11 7 11c-3.866 0-7-3.138-7-7.006V0h2v4.005C2 6.765 4.24 9 7 9c2.756 0 5-2.236 5-4.995V0h2zm0 26v-5.994C14 16.138 10.866 13 7 13c-3.858 0-7 3.137-7 7.006V26h2v-6.005C2 17.235 4.244 15 7 15c2.76 0 5 2.236 5 4.995V26h2zm2-18.994C16 3.136 19.142 0 23 0c3.866 0 7 3.138 7 7.006v9.988C30 20.864 26.858 24 23 24c-3.866 0-7-3.138-7-7.006V7.006zm2-.01C18 4.235 20.244 2 23 2c2.76 0 5 2.236 5 4.995v10.01C28 19.765 25.756 22 23 22c-2.76 0-5-2.236-5-4.995V6.995z' fill='%23dddddd' fill-opacity='0.4' fill-rule='evenodd'/%3E%3C/svg%3E\");}");
                break;
              case 2:
                // Dominoes
                CP("body {background-color: #eeeeee; background-image: url(\"data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='126' height='84' viewBox='0 0 126 84'%3E%3Cg fill-rule='evenodd'%3E%3Cg fill='%23cccccc' fill-opacity='0.4'%3E%3Cpath d='M126 83v1H0v-2h40V42H0v-2h40V0h2v40h40V0h2v40h40V0h2v83zm-2-1V42H84v40h40zM82 42H42v40h40V42zm-50-6a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM8 12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm96 12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm-42 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm30-12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM20 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm12 24a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM8 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM8 78a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm12 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm54 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM50 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM50 78a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm54-12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm12 12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM92 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM92 78a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24-42a4 4 0 1 1 0-8 4 4 0 0 1 0 8z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E\");}");
                break;
              case 3:
                // Grid
                CP("body {background-color: #eeeeee; background-image: url(\"data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='100' height='100' viewBox='0 0 100 100'%3E%3Cg fill-rule='evenodd'%3E%3Cg fill='%23cccccc' fill-opacity='0.4'%3E%3Cpath opacity='.5' d='M96 95h4v1h-4v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9zm-1 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm9-10v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm9-10v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm9-10v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9z'/%3E%3Cpath d='M6 5V0H5v5H0v1h5v94h1V6h94V5H6z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E\");}");
                break;
              case 4:
                // Hexagon
                CP("body {background-color: #eeeeee; background-image: url(\"data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='28' height='49' viewBox='0 0 28 49'%3E%3Cg fill-rule='evenodd'%3E%3Cg id='hexagons' fill='%23cccccc' fill-opacity='0.4' fill-rule='nonzero'%3E%3Cpath d='M13.99 9.25l13 7.5v15l-13 7.5L1 31.75v-15l12.99-7.5zM3 17.9v12.7l10.99 6.34 11-6.35V17.9l-11-6.34L3 17.9zM0 15l12.98-7.5V0h-2v6.35L0 12.69v2.3zm0 18.5L12.98 41v8h-2v-6.85L0 35.81v-2.3zM15 0v7.5L27.99 15H28v-2.31h-.01L17 6.35V0h-2zm0 49v-8l12.99-7.5H28v2.31h-.01L17 42.15V49h-2z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E\");}");
                break;
              case 5:
                // Squares
                CP("body {background: radial-gradient(lightgrey 3px, transparent 4px), radial-gradient(lightgrey 3px, transparent 4px), linear-gradient(#f2f2f2 4px, transparent 0), linear-gradient(45deg, transparent 74px, transparent 75px, #a4a4a4 75px, #a4a4a4 76px, transparent 77px, transparent 109px), linear-gradient(-45deg, transparent 75px, transparent 76px, #a4a4a4 76px, #a4a4a4 77px, transparent 78px, transparent 109px), #f2f2f2; background-size: 109px 109px, 109px 109px, 100% 6px, 109px 109px, 109px 109px; background-position: 54px 55px, 0px 0px, 0px 0px, 0px 0px, 0px 0px;}");
                break;
            }

            CP("div {text-align: center; margin: 0 auto;}");

            CP("h1, h3 {font-family: " + String(_settings->webFont) + ", Verdana; font-weight: bold;}");
            CP("h1 {font-size: 34px;}");

            CP(".audiowide {font-family: Audiowide;}");
            CP(".codystar {font-family: Codystar;}");
            CP(".gugi {font-family: Gugi;}");
            CP(".michroma {font-family: Michroma;}");
            CP(".novaround {font-family: Nova Round;}");
            CP(".orbitron {font-family: Orbitron;}");
            CP(".revalia {font-family: Revalia;}");
            CP(".slackey {font-family: Slackey;}");

            CP("a {margin: 15px;}");
            CP("a.o, a.r, a.g, a.b, a.p, a.s {font-family: " + String(_settings->webFont) + ", Verdana; font-weight: bold; font-size: 26px; border-radius: 15px; cursor: pointer; display: inline-block;}");
            CP("a.s {border-radius: 30px;}");
            CP(".o {color: #c92200; border: 1px #e65f44 solid; background: linear-gradient(180deg, #f0cb11 5%, #f2ab1e 100%); text-shadow: 1px 1px 1px #ded17c; box-shadow: inset 1px 1px 2px 0px #f9eca0;}");
            CP(".r {color: #ffffff; border: 1px #d83526 solid; background: linear-gradient(180deg, #fe1900 5%, #ce0000 100%); text-shadow: 1px 1px 1px #b23d35; box-shadow: inset 1px 1px 2px 0px #f29d93;}");
            CP(".g {color: #306108; border: 1px #268a16 solid; background: linear-gradient(180deg, #77d42a 5%, #5cb811 100%); text-shadow: 1px 1px 1px #aade7c; box-shadow: inset 1px 1px 2px 0px #c9efab;}");
            CP(".b {color: #ffffff; border: 1px #84bcf3 solid; background: linear-gradient(180deg, #79bcff 5%, #378ee5 100%); text-shadow: 1px 1px 1px #528fcc; box-shadow: inset 1px 1px 2px 0px #bbdaf7;}");
            CP(".p {color: #ffffff; border: 1px #a946f5 solid; background: linear-gradient(180deg, #c579ff 5%, #a341ee 100%); text-shadow: 1px 1px 1px #8628ce; box-shadow: inset 1px 1px 2px 0px #e6cafc;}");
            CP(".s {color: #777777; border: 1px #dcdcdc solid; background: linear-gradient(180deg, #ededed 5%, #dfdfdf 100%); text-shadow: 1px 1px 1px #ffffff; box-shadow: inset 1px 1px 2px 0px #ffffff;}");
            CP(".o:hover {color: #8B0000; background: linear-gradient(180deg, #f2ab1e 5%, #f0cb11 100%);}");
            CP(".r:hover {color: #dddddd; background: linear-gradient(180deg, #ce0000 5%, #fe1900 100%);}");
            CP(".g:hover {color: #004d00; background: linear-gradient(180deg, #5cb811 5%, #77d42a 100%);}");
            CP(".b:hover {color: #dddddd; background: linear-gradient(180deg, #378ee5 5%, #79bcff 100%);}");
            CP(".p:hover {color: #dddddd; background: linear-gradient(180deg, #a341ee 5%, #c579ff 100%);}");
            CP(".s:hover {color: #666666; background: linear-gradient(180deg, #dfdfdf 5%, #ededed 100%);}");

            CP(".css-btn-icon {padding: 10px 17px; display: block; text-align: center; border-bottom: 1px solid rgba(255, 255, 255, 0.16); box-shadow: rgba(0, 0, 0, 0.14) 0px -1px 0px inset;}");
            CP(".css-btn-icon svg {vertical-align: middle; position: relative; font-size: 66px;}");
            CP(".css-btn-text {padding: 10px 0px; width: 400px; display: block; text-align: center;}");
            CP(".css-btn-slide {padding: 9px 0px; width: 400px; display: block; text-align: center; border-bottom: 1px solid rgba(255, 255, 255, 0.16); box-shadow: rgba(0, 0, 0, 0.14) 0px -1px 0px inset;}");
            CP(".css-btn-icon-footer {padding: 10px 5px; display: block; text-align: center; border-bottom: 1px solid rgba(255, 255, 255, 0.16); box-shadow: rgba(0, 0, 0, 0.14) 0px -1px 0px inset;}");
            CP(".css-btn-icon-footer svg {vertical-align: middle; position: relative; font-size: 40px;}");
            CP(".css-btn-text-footer {padding: 5px 0px; width: 175px; display: block; text-align: center;}");

            CP(".slide {-webkit-appearance: none; width: 355px; height: 15px; margin: 0px 18px; border-radius: 5px; background: #d3d3d3; outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s;}");
            CP(".slide:hover {opacity: 1;}");
            CP(".slide::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 25px; height: 25px; border-radius: 50%; background: #c92200; cursor: pointer;}");
            CP(".slide::-moz-range-thumb {width: 25px; height: 25px; border-radius: 50%; background: #c92200; cursor: pointer;}");

            CP("input::-webkit-outer-spin-button, input::-webkit-inner-spin-button {-webkit-appearance: none; margin: 0; height:40px; text-align:center;}");
            CP("input[type=text], [type=password] {margin: 5px 2px; box-sizing: border-box; height: 40px; border: 2px solid #b3b3b3; border-radius: 15px; font-size: 20px; text-align: center; font-family: " + String(_settings->webFont) + ", Verdana; background: #e6ffe6;}");
            CP("select {height: 45px; width: 85px; line-height:40px; padding: 5px 10px; margin:0px 2px; border-box; border: 2px solid #b3b3b3; border-radius: 15px; font-size: 20px; font-family: " + String(_settings->webFont) + ", Verdana; background: #e6ffe6;}");
            CP(".selectwide {width: 370px;}");

            CP("input[type=text], [type=password], select {opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s;}");
            CP("input[type=text]:hover, [type=password]:hover, select:hover {opacity: 1;}");

            CP("div.divTable {width: 1000px; text-align: center; display: table;}");
            CP(".divTable .divTableCell {padding: 3px 2px; width: 1000px;}");
            CP(".divTableRow {display: table-row;}");
            CP(".divTableCell {display: table-cell;}");
            CP(".divTableBody {display: table-row-group;}");
            CP("</style>");

            CP("</head>");
            CP("<body>");

            CP("<div><h1>" + String(_settings->webTitle) + " : " + String(_settings->webName) + "</h1><h3>Firmware Version : " + String(_settings->fwVersion) + "</h3>");

            CP("<div class='divTable'>");
            CP("<div class='divTableBody'>");
            CP("<div class='divTableRow'>");
            CP("<div class='divTableCell'>");

            CP("<form name='slot' action='10' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='slot.submit()'><svg width='66' height='66' viewBox='2 2 16 16' fill='currentColor' xmlns='http://www.w3.org/2000/svg'><path fill-rule='evenodd' d='M13 5.5a.5.5 0 01.5.5v9a.5.5 0 01-1 0V6a.5.5 0 01.5-.5z' clip-rule='evenodd'/><path fill-rule='evenodd' d='M12.646 4.646a.5.5 0 01.708 0l3 3a.5.5 0 01-.708.708L13 5.707l-2.646 2.647a.5.5 0 01-.708-.708l3-3zm-9 7a.5.5 0 01.708 0L7 14.293l2.646-2.647a.5.5 0 01.708.708l-3 3a.5.5 0 01-.708 0l-3-3a.5.5 0 010-.708z' clip-rule='evenodd'/><path fill-rule='evenodd' d='M7 4.5a.5.5 0 01.5.5v9a.5.5 0 01-1 0V5a.5.5 0 01.5-.5z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='css-btn-slide'><input type='range' id='v' name='v' value='5' min='1' max='20' oninput='this.nextElementSibling.value = this.value' class='slide'><output>5</output> : Spin Cycles</span>");
            CP("<span class='css-btn-text' onclick='slot.submit()'>Run Slot Machine</span>");
            CP("</a></form>");

            CP("<form name='thp' action='11' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='thp.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 9.312l-1.762.491 1.562.881-.491.871-1.562-.881.491 1.762-.963.268-.76-2.724-2.015-1.126v1.939l2 2-.707.707-1.293-1.293v1.793h-1v-1.793l-1.293 1.293-.707-.707 2-2v-1.939l-2.015 1.126-.761 2.724-.963-.268.491-1.762-1.562.882-.491-.871 1.562-.881-1.761-.492.269-.962 2.725.76 1.982-1.11-1.983-1.109-2.724.759-.269-.962 1.762-.491-1.563-.882.491-.871 1.562.881-.49-1.762.963-.269.76 2.725 2.015 1.128v-1.94l-2-2 .707-.707 1.293 1.293v-1.793h1v1.793l1.293-1.293.707.707-2 2v1.94l2.016-1.127.76-2.725.963.269-.492 1.761 1.562-.881.491.871-1.562.881 1.762.492-.269.962-2.725-.76-1.982 1.11 1.982 1.109 2.725-.76.269.963zm4-5.812v7.525c0 1.57-.514 2.288-1.41 3.049-1.011.859-1.59 2.107-1.59 3.426 0 2.481 2.019 4.5 4.5 4.5s4.5-2.019 4.5-4.5c0-1.319-.579-2.567-1.589-3.426-.897-.762-1.411-1.48-1.411-3.049v-7.525c0-.827-.673-1.5-1.5-1.5s-1.5.673-1.5 1.5zm5 0v7.525c0 .587.258 1.145.705 1.525 1.403 1.192 2.295 2.966 2.295 4.95 0 3.59-2.909 6.5-6.5 6.5s-6.5-2.91-6.5-6.5c0-1.984.892-3.758 2.295-4.949.447-.381.705-.94.705-1.526v-7.525c0-1.934 1.567-3.5 3.5-3.5s3.5 1.566 3.5 3.5zm0 14c0 1.934-1.567 3.5-3.5 3.5s-3.5-1.566-3.5-3.5c0-1.141.599-2.084 1.393-2.781 1.01-.889 1.607-1.737 1.607-3.221v-.498h1v.498c0 1.486.595 2.33 1.607 3.221.794.697 1.393 1.64 1.393 2.781z'/></svg></span>");
            CP("<span class='css-btn-text' onclick='thp.submit()'>&deg;C | RH | mbar</span>");
            CP("</a></form>");

            CP("<form name='ntpsync' action='12' method='get'><a class='o' onclick='ntpsync.submit();'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 1c6.074 0 11 4.925 11 11 0 1.042-.154 2.045-.425 3h-2.101c.335-.94.526-1.947.526-3 0-4.962-4.037-9-9-9-1.706 0-3.296.484-4.654 1.314l1.857 2.686h-6.994l2.152-7 1.85 2.673c1.683-1.049 3.658-1.673 5.789-1.673zm4.646 18.692c-1.357.826-2.944 1.308-4.646 1.308-4.963 0-9-4.038-9-9 0-1.053.191-2.06.525-3h-2.1c-.271.955-.425 1.958-.425 3 0 6.075 4.925 11 11 11 2.127 0 4.099-.621 5.78-1.667l1.853 2.667 2.152-6.989h-6.994l1.855 2.681zm-6.54-4.7c.485-.514 1.154-.832 1.894-.832s1.408.318 1.894.832l-1.894 2.008-1.894-2.008zm-.874-.927c.709-.751 1.688-1.215 2.768-1.215s2.059.465 2.768 1.216l1.164-1.236c-1.006-1.067-2.396-1.727-3.932-1.727s-2.926.66-3.932 1.727l1.164 1.235zm-2.038-2.163c1.23-1.304 2.929-2.11 4.806-2.11s3.576.807 4.806 2.111l1.194-1.267c-1.535-1.629-3.656-2.636-6-2.636s-4.465 1.007-6 2.636l1.194 1.266z'/></svg></span>");
            CP("<span class='css-btn-text'>NTP Sync</span>");
            CP("</a></form>");

            if (_settings->flashNTP) {
              CP("<form name='ntpoff' action='13' method='get'><a class='r' onclick='ntpoff.submit();'>");
              CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M23 12c0 1.042-.154 2.045-.425 3h-2.101c.335-.94.526-1.947.526-3 0-4.962-4.037-9-9-9-1.706 0-3.296.484-4.654 1.314l1.857 2.686h-6.994l2.152-7 1.85 2.673c1.683-1.049 3.658-1.673 5.789-1.673 6.074 0 11 4.925 11 11zm-6.354 7.692c-1.357.826-2.944 1.308-4.646 1.308-4.963 0-9-4.038-9-9 0-1.053.191-2.06.525-3h-2.1c-.271.955-.425 1.958-.425 3 0 6.075 4.925 11 11 11 2.127 0 4.099-.621 5.78-1.667l1.853 2.667 2.152-6.989h-6.994l1.855 2.681zm-.646-5.108l-2.592-2.594 2.592-2.587-1.416-1.403-2.591 2.589-2.588-2.589-1.405 1.405 2.593 2.598-2.593 2.592 1.405 1.405 2.601-2.596 2.591 2.596 1.403-1.416z'/></svg></span>");
              CP("<span class='css-btn-text'>Disable NTP</span>");
              CP("</a></form>");
            } else {
              CP("<form name='ntpon' action='13' method='get'><a class='g' onclick='ntpon.submit();'>");
              CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M23 12c0 1.042-.154 2.045-.425 3h-2.101c.335-.94.526-1.947.526-3 0-4.962-4.037-9-9-9-1.706 0-3.296.484-4.654 1.314l1.857 2.686h-6.994l2.152-7 1.85 2.673c1.683-1.049 3.658-1.673 5.789-1.673 6.074 0 11 4.925 11 11zm-6.354 7.692c-1.357.826-2.944 1.308-4.646 1.308-4.963 0-9-4.038-9-9 0-1.053.191-2.06.525-3h-2.1c-.271.955-.425 1.958-.425 3 0 6.075 4.925 11 11 11 2.127 0 4.099-.621 5.78-1.667l1.853 2.667 2.152-6.989h-6.994l1.855 2.681zm-3.646-7.692v-6h-2v8h7v-2h-5z'/></svg></span>");
              CP("<span class='css-btn-text'>Enable NTP</span>");
              CP("</a></form>");
            }

            CP("<form name='displayon' action='14' method='get'><a class='g' onclick='displayon.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M19 6.734c0 4.164-3.75 6.98-3.75 10.266h-1.992c.001-2.079.996-3.826 1.968-5.513.913-1.585 1.774-3.083 1.774-4.753 0-3.108-2.518-4.734-5.004-4.734-2.482 0-4.996 1.626-4.996 4.734 0 1.67.861 3.168 1.774 4.753.972 1.687 1.966 3.434 1.967 5.513h-1.991c0-3.286-3.75-6.103-3.75-10.266 0-4.343 3.498-6.734 6.996-6.734 3.502 0 7.004 2.394 7.004 6.734zm-4 11.766c0 .276-.224.5-.5.5h-5c-.276 0-.5-.224-.5-.5s.224-.5.5-.5h5c.276 0 .5.224.5.5zm0 2c0 .276-.224.5-.5.5h-5c-.276 0-.5-.224-.5-.5s.224-.5.5-.5h5c.276 0 .5.224.5.5zm-1.701 3.159c-.19.216-.465.341-.752.341h-1.094c-.287 0-.562-.125-.752-.341l-1.451-1.659h5.5l-1.451 1.659zm-3.629-16.347l-1.188-.153c.259-1.995 1.5-3.473 3.518-3.847l.219 1.177c-1.947.361-2.433 1.924-2.549 2.823z'/></svg></span>");
            CP("<span class='css-btn-text'>Enable Display</span>");
            CP("</a></form>");

            CP("<form name='displayoff' action='14' method='get'><a class='r' onclick='displayoff.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M10.741 17h-1.991c0-.17-.016-.338-.035-.506l1.703-1.548c.197.653.323 1.332.323 2.054zm-.04 6.659c.19.216.465.341.753.341h1.093c.288 0 .562-.125.752-.341l1.451-1.659h-5.5l1.451 1.659zm3.799-3.659h-5c-.276 0-.5.224-.5.5s.224.5.5.5h5c.276 0 .5-.224.5-.5s-.224-.5-.5-.5zm0-2h-5c-.276 0-.5.224-.5.5s.224.5.5.5h5c.276 0 .5-.224.5-.5s-.224-.5-.5-.5zm1.707-8.315c-1.104 2.28-2.948 4.483-2.949 7.315h1.992c0-3.169 3.479-5.906 3.726-9.832l-2.769 2.517zm6.793-8.201l-20.654 18.75-1.346-1.5 6.333-5.728c-1.062-1.873-2.333-3.843-2.333-6.272 0-4.343 3.498-6.734 6.996-6.734 2.408 0 4.798 1.146 6.064 3.267l3.598-3.267 1.342 1.484zm-14.147 10.142l7.676-6.969c-.833-1.742-2.682-2.657-4.533-2.657-2.483 0-4.996 1.626-4.996 4.734 0 1.713.907 3.246 1.853 4.892z'/></svg></span>");
            CP("<span class='css-btn-text'>Disable Display</span>");
            CP("</a></form>");

            CP("<form name='pirenable' action='15' method='get'><a class='g' onclick='pirenable.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' fill-rule='evenodd' clip-rule='evenodd' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c6.623 0 12 5.377 12 12s-5.377 12-12 12-12-5.377-12-12 5.377-12 12-12zm0 2c5.52 0 10 4.481 10 10 0 5.52-4.48 10-10 10-5.519 0-10-4.48-10-10 0-5.519 4.481-10 10-10zm-2 2.252v1.038c-2.89.862-5 3.542-5 6.71 0 3.863 3.137 7 7 7 1.932 0 3.682-.784 4.949-2.051l.706.706c-1.448 1.449-3.448 2.345-5.655 2.345-4.416 0-8-3.585-8-8 0-3.725 2.551-6.859 6-7.748zm0 3.165v1.119c-1.195.692-2 1.984-2 3.464 0 2.208 1.792 4 4 4 1.104 0 2.104-.448 2.828-1.172l.707.707c-.905.904-2.155 1.465-3.535 1.465-2.76 0-5-2.24-5-5 0-2.049 1.235-3.811 3-4.583zm1 2.851v-6.268c0-.265.105-.52.293-.707.187-.188.442-.293.707-.293.265 0 .52.105.707.293.188.187.293.442.293.707v6.268c.598.346 1 .992 1 1.732 0 1.104-.896 2-2 2s-2-.896-2-2c0-.74.402-1.386 1-1.732z'/></svg></span>");
            CP("<span class='css-btn-text'>Enable PIR</span>");
            CP("</a></form>");

            CP("<form name='pirdisable' action='15' method='get'><a class='r' onclick='pirdisable.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' fill-rule='evenodd' clip-rule='evenodd' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c6.623 0 12 5.377 12 12s-5.377 12-12 12-12-5.377-12-12 5.377-12 12-12zm0 2c5.52 0 10 4.481 10 10 0 5.52-4.48 10-10 10-5.519 0-10-4.48-10-10 0-5.519 4.481-10 10-10zm-2 2.252v1.038c-2.89.862-5 3.542-5 6.71 0 3.863 3.137 7 7 7 1.932 0 3.682-.784 4.949-2.051l.706.706c-1.448 1.449-3.448 2.345-5.655 2.345-4.416 0-8-3.585-8-8 0-3.725 2.551-6.859 6-7.748zm0 3.165v1.119c-1.195.692-2 1.984-2 3.464 0 2.208 1.792 4 4 4 1.104 0 2.104-.448 2.828-1.172l.707.707c-.905.904-2.155 1.465-3.535 1.465-2.76 0-5-2.24-5-5 0-2.049 1.235-3.811 3-4.583zm1 2.851v-6.268c0-.265.105-.52.293-.707.187-.188.442-.293.707-.293.265 0 .52.105.707.293.188.187.293.442.293.707v6.268c.598.346 1 .992 1 1.732 0 1.104-.896 2-2 2s-2-.896-2-2c0-.74.402-1.386 1-1.732z'/></svg></span>");
            CP("<span class='css-btn-text'>Disable PIR</span>");
            CP("</a></form>");

            CP("<form name='lxenable' action='16' method='get'><a class='g' onclick='lxenable.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 2c5.514 0 10 4.486 10 10s-4.486 10-10 10-10-4.486-10-10 4.486-10 10-10zm0-2c-6.627 0-12 5.373-12 12s5.373 12 12 12 12-5.373 12-12-5.373-12-12-12zm3 12c0 1.657-1.343 3-3 3s-3-1.343-3-3 1.343-3 3-3 3 1.343 3 3zm-3-5c.343 0 .677.035 1 .101v-3.101h-2v3.101c.323-.066.657-.101 1-.101zm-2.755.832l-2.195-2.196-1.414 1.414 2.195 2.195c.372-.561.853-1.042 1.414-1.413zm-2.245 4.168c0-.343.035-.677.101-1h-3.101v2h3.101c-.066-.323-.101-.657-.101-1zm9.169-2.754l2.195-2.195-1.414-1.415-2.195 2.195c.561.372 1.042.853 1.414 1.415zm.73 1.754c.066.323.101.657.101 1s-.035.677-.101 1h3.101v-2h-3.101zm-2.144 5.168l2.195 2.195 1.414-1.414-2.195-2.195c-.372.562-.853 1.043-1.414 1.414zm-6.924-1.414l-2.195 2.196 1.414 1.414 2.195-2.195c-.561-.372-1.042-.853-1.414-1.415zm4.169 2.246c-.343 0-.677-.035-1-.101v3.101h2v-3.101c-.323.066-.657.101-1 .101z'/></svg></span>");
            CP("<span class='css-btn-text'>Enable Light Sensor</span>");
            CP("</a></form>");

            CP("<form name='lxdisable' action='16' method='get'><a class='r' onclick='lxdisable.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M16.169 14.754l2.195 2.195-1.414 1.414-2.195-2.195c.561-.371 1.042-.852 1.414-1.414zm3.831-3.754h-3.101c.066.323.101.657.101 1s-.035.677-.101 1h3.101v-2zm-7 5.899c-.323.066-.657.101-1 .101s-.677-.035-1-.101v3.101h2v-3.101zm9.055-11.437l-1.457 1.457c.884 1.491 1.402 3.225 1.402 5.081 0 5.514-4.486 10-10 10-1.856 0-3.59-.518-5.081-1.402l-1.457 1.457c1.882 1.226 4.125 1.945 6.538 1.945 6.627 0 12-5.373 12-12 0-2.414-.72-4.656-1.945-6.538zm1.945-4.048l-22.586 22.586-1.414-1.414 2.854-2.854c-1.772-2.088-2.854-4.779-2.854-7.732 0-6.627 5.373-12 12-12 2.953 0 5.644 1.082 7.732 2.853l2.854-2.853 1.414 1.414zm-19.733 16.905l5.03-5.03c-.189-.39-.297-.826-.297-1.289 0-1.657 1.343-3 3-3 .463 0 .899.108 1.289.297l5.03-5.03c-1.724-1.413-3.922-2.267-6.319-2.267-5.514 0-10 4.486-10 10 0 2.397.854 4.595 2.267 6.319zm8.733-11.218v-3.101h-2v3.101c.323-.066.657-.101 1-.101s.677.035 1 .101zm-9 3.899v2h3.101c-.066-.323-.101-.657-.101-1s.035-.677.101-1h-3.101zm3.05-5.364l-1.414 1.414 2.195 2.195c.372-.562.853-1.042 1.414-1.414l-2.195-2.195z'/></svg></span>");
            CP("<span class='css-btn-text'>Disable Light Sensor</span>");
            CP("</a></form>");

            CP("<form name='dc' action='17' method='get'><a class='p' onclick='dc.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M19 8v8h-17v-8h17zm2-2h-21v12h21v-12zm1 9h.75c.69 0 1.25-.56 1.25-1.25v-3.5c0-.69-.56-1.25-1.25-1.25h-.75v6zm-6.501-2h-1.499v-2h1.499c.277 0 .501-.224.501-.5s-.224-.5-.501-.5h-1.499v-1c-5.037 0-4.618 2-6.002 2h-2.998v2h3c1.373 0 1 2 6 2v-1h1.499c.277 0 .501-.224.501-.5s-.224-.5-.501-.5z'/></svg></span>");
            CP("<span class='css-btn-text'>12V DC Power</span>");
            CP("</a></form>");

            CP("<form name='usb' action='17' method='get'><a class='p' onclick='usb.submit()'>");
            CP("<span class='css-btn-icon'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M19 8v8h-17v-8h17zm2-2h-21v12h21v-12zm1 9h.75c.69 0 1.25-.56 1.25-1.25v-3.5c0-.69-.56-1.25-1.25-1.25h-.75v6zm-17-4.488l6.592 4.488v-2.61l4.408 1.296-6.401-4.686v2.843l-4.599-1.331z'/></svg></span>");
            CP("<span class='css-btn-text'>5V USB Power</span>");
            CP("</a></form>");

            CP("</div><div class='divTableCell'>");

            CP("<form name='brightness' action='18' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='brightness.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 9c1.654 0 3 1.346 3 3s-1.346 3-3 3v-6zm0-2c-2.762 0-5 2.238-5 5s2.238 5 5 5 5-2.238 5-5-2.238-5-5-5zm-4.184-.599l-3.593-3.594-1.415 1.414 3.595 3.595c.401-.537.876-1.013 1.413-1.415zm4.184-1.401c.34 0 .672.033 1 .08v-5.08h-2v5.08c.328-.047.66-.08 1-.08zm5.598 2.815l3.595-3.595-1.414-1.414-3.595 3.595c.537.402 1.012.878 1.414 1.414zm-12.598 4.185c0-.34.033-.672.08-1h-5.08v2h5.08c-.047-.328-.08-.66-.08-1zm11.185 5.598l3.594 3.593 1.415-1.414-3.594-3.593c-.403.536-.879 1.012-1.415 1.414zm-9.784-1.414l-3.593 3.593 1.414 1.414 3.593-3.593c-.536-.402-1.011-.877-1.414-1.414zm12.519-5.184c.047.328.08.66.08 1s-.033.672-.08 1h5.08v-2h-5.08zm-6.92 8c-.34 0-.672-.033-1-.08v5.08h2v-5.08c-.328.047-.66.08-1 .08z'/></svg></span>");
            CP("<span class='css-btn-slide'><input type='range' id='v' name='v' value='100' min='1' max='255' oninput='this.nextElementSibling.value = this.value' class='slide'><output>100</output></span>");
            CP("<span class='css-btn-text' onclick='brightness.submit()'>Set Brightness</span>");
            CP("</a></form>");

            CP("<form name='utcoffset' action='19' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='utcoffset.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M11 6v8h7v-2h-5v-6h-2zm10.854 7.683l1.998.159c-.132.854-.351 1.676-.652 2.46l-1.8-.905c.2-.551.353-1.123.454-1.714zm-2.548 7.826l-1.413-1.443c-.486.356-1.006.668-1.555.933l.669 1.899c.821-.377 1.591-.844 2.299-1.389zm1.226-4.309c-.335.546-.719 1.057-1.149 1.528l1.404 1.433c.583-.627 1.099-1.316 1.539-2.058l-1.794-.903zm-20.532-5.2c0 6.627 5.375 12 12.004 12 1.081 0 2.124-.156 3.12-.424l-.665-1.894c-.787.2-1.607.318-2.455.318-5.516 0-10.003-4.486-10.003-10s4.487-10 10.003-10c2.235 0 4.293.744 5.959 1.989l-2.05 2.049 7.015 1.354-1.355-7.013-2.184 2.183c-2.036-1.598-4.595-2.562-7.385-2.562-6.629 0-12.004 5.373-12.004 12zm23.773-2.359h-2.076c.163.661.261 1.344.288 2.047l2.015.161c-.01-.755-.085-1.494-.227-2.208z'/></svg></span>");
            CP("<span class='css-btn-slide'><input type='range' id='v' name='v' value='1' min='-23' max='23' oninput='this.nextElementSibling.value = this.value' class='slide'><output>1</output></span>");
            CP("<span class='css-btn-text' onclick='utcoffset.submit()'>Set UTC Offset</span>");
            CP("</a></form>");

            CP("<form name='settime' action='20' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='settime.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M13 12l-.688-4h-.609l-.703 4c-.596.347-1 .984-1 1.723 0 1.104.896 2 2 2s2-.896 2-2c0-.739-.404-1.376-1-1.723zm-1-8c-5.522 0-10 4.477-10 10s4.478 10 10 10 10-4.477 10-10-4.478-10-10-10zm0 18c-4.411 0-8-3.589-8-8s3.589-8 8-8 8 3.589 8 8-3.589 8-8 8zm-2-19.819v-2.181h4v2.181c-1.438-.243-2.592-.238-4 0zm9.179 2.226l1.407-1.407 1.414 1.414-1.321 1.321c-.462-.484-.964-.926-1.5-1.328z'/></svg></span>");
            CP("<span class='css-btn-slide'>");
            CP("<select name='h' id='h'>");
            CP("<option value='0'>HH</option>");
            for (byte i = 0; i <= 9; i++) {
              CP("<option value='" + String(i) + "'>0" + String(i) + "</option>");
            }
            for (byte i = 10; i <= 23; i++) {
              CP("<option value='" + String(i) + "'>" + String(i) + "</option>");
            }
            CP("</select>");
            CP("<select name='m' id='m'>");
            CP("<option value='0'>MM</option>");
            for (byte i = 0; i <= 9; i++) {
              CP("<option value='" + String(i) + "'>0" + String(i) + "</option>");
            }
            for (byte i = 10; i <= 59; i++) {
              CP("<option value='" + String(i) + "'>" + String(i) + "</option>");
            }
            CP("</select>");
            CP("<select name='s' id='s'>");
            CP("<option value='0'>SS</option>");
            for (byte i = 0; i <= 9; i++) {
              CP("<option value='" + String(i) + "'>0" + String(i) + "</option>");
            }
            for (byte i = 10; i <= 59; i++) {
              CP("<option value='" + String(i) + "'>" + String(i) + "</option>");
            }
            CP("</select>");
            CP("</span>");
            CP("<span class='css-btn-text' onclick='settime.submit()'>Set Time</span>");
            CP("</a></form>");

            CP("<form name='setdate' action='21' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='setdate.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M24 2v22h-24v-22h3v1c0 1.103.897 2 2 2s2-.897 2-2v-1h10v1c0 1.103.897 2 2 2s2-.897 2-2v-1h3zm-2 6h-20v14h20v-14zm-2-7c0-.552-.447-1-1-1s-1 .448-1 1v2c0 .552.447 1 1 1s1-.448 1-1v-2zm-14 2c0 .552-.447 1-1 1s-1-.448-1-1v-2c0-.552.447-1 1-1s1 .448 1 1v2zm6.687 13.482c0-.802-.418-1.429-1.109-1.695.528-.264.836-.807.836-1.503 0-1.346-1.312-2.149-2.581-2.149-1.477 0-2.591.925-2.659 2.763h1.645c-.014-.761.271-1.315 1.025-1.315.449 0 .933.272.933.869 0 .754-.816.862-1.567.797v1.28c1.067 0 1.704.067 1.704.985 0 .724-.548 1.048-1.091 1.048-.822 0-1.159-.614-1.188-1.452h-1.634c-.032 1.892 1.114 2.89 2.842 2.89 1.543 0 2.844-.943 2.844-2.518zm4.313 2.518v-7.718h-1.392c-.173 1.154-.995 1.491-2.171 1.459v1.346h1.852v4.913h1.711z'/></svg></span>");
            CP("<span class='css-btn-slide'>");
            CP("<select name='d' id='d'>");
            CP("<option value='1'>DD</option>");
            for (byte i = 0; i <= 9; i++) {
              CP("<option value='" + String(i) + "'>0" + String(i) + "</option>");
            }
            for (byte i = 10; i <= 31; i++) {
              CP("<option value='" + String(i) + "'>" + String(i) + "</option>");
            }
            CP("</select>");
            CP("<select name='m' id='m'>");
            CP("<option value='1'>MM</option>");
            for (byte i = 0; i <= 9; i++) {
              CP("<option value='" + String(i) + "'>0" + String(i) + "</option>");
            }
            CP("<option value='2'>10</option>");
            CP("<option value='3'>11</option>");
            CP("<option value='3'>12</option>");
            CP("</select>");
            CP("<select name='y' id='y'>");
            CP("<option value='21'>YY</option>");
            for (byte i = 21; i <= 99; i++) {
              CP("<option value='" + String(i) + "'>" + String(i) + "</option>");
            }
            CP("</select>");
            CP("</span>");
            CP("<span class='css-btn-text' onclick='setdate.submit()'>Set Date</span>");
            CP("</a></form>");

            CP("<form name='onoffhour' action='22' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='onoffhour.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M15.91 13.34l2.636-4.026-.454-.406-3.673 3.099c-.675-.138-1.402.068-1.894.618-.736.823-.665 2.088.159 2.824.824.736 2.088.665 2.824-.159.492-.55.615-1.295.402-1.95zm-3.91-10.646v-2.694h4v2.694c-1.439-.243-2.592-.238-4 0zm8.851 2.064l1.407-1.407 1.414 1.414-1.321 1.321c-.462-.484-.964-.927-1.5-1.328zm-18.851 4.242h8v2h-8v-2zm-2 4h8v2h-8v-2zm3 4h7v2h-7v-2zm21-3c0 5.523-4.477 10-10 10-2.79 0-5.3-1.155-7.111-3h3.28c1.138.631 2.439 1 3.831 1 4.411 0 8-3.589 8-8s-3.589-8-8-8c-1.392 0-2.693.369-3.831 1h-3.28c1.811-1.845 4.321-3 7.111-3 5.523 0 10 4.477 10 10z'/></svg></span>");
            CP("<span class='css-btn-slide'><input type='range' id='o' name='o' value='6' min='5' max='8' oninput='this.nextElementSibling.value = this.value' class='slide'><output>1</output> : On Hour</span></span>");
            CP("<span class='css-btn-slide'><input type='range' id='x' name='x' value='23' min='21' max='23' oninput='this.nextElementSibling.value = this.value' class='slide'><output>1</output> : Off Hour</span></span>");
            CP("<span class='css-btn-text' onclick='onoffhour.submit()'>Set On/Off Hour</span>");
            CP("</a></form>");

            CP("<form name='changefont' action='23' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='changefont.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M24 20v1h-4v-1h.835c.258 0 .405-.178.321-.422l-.473-1.371h-2.231l-.575-1.59h2.295l-1.362-4.077-1.154 3.451-.879-2.498.921-2.493h2.222l3.033 8.516c.111.315.244.484.578.484h.469zm-6-1h1v2h-7v-2h.532c.459 0 .782-.453.633-.887l-.816-2.113h-6.232l-.815 2.113c-.149.434.174.887.633.887h1.065v2h-7v-2h.43c.593 0 1.123-.375 1.32-.935l5.507-15.065h3.952l5.507 15.065c.197.56.69.935 1.284.935zm-10.886-6h4.238l-2.259-6.199-1.979 6.199z'/></svg></span>");
            CP("<span class='css-btn-slide'>");
            CP("<select class='selectwide' name='v' id='v'>");
            CP("<option value='1' class='audiowide'>Audiowide</option>");
            CP("<option value='2' class='codystar'>Codystar</option>");
            CP("<option value='3' class='gugi'>Gugi</option>");
            CP("<option value='4' class='michroma'>Michroma</option>");
            CP("<option value='5' class='novaround'>Nova Round</option>");
            CP("<option value='6' class='orbitron'>Orbitron</option>");
            CP("<option value='7' class='revalia'>Revalia</option>");
            CP("<option value='8' class='slackey'>Slackey</option>");
            CP("</select>");
            CP("</span>");
            CP("<span class='css-btn-text' onclick='changefont.submit()'>Change Font</span>");
            CP("</a></form>");

            CP("<form name='changebg' action='24' method='get'><a class='o'>");
            CP("<span class='css-btn-icon' onclick='changebg.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M9 12c0-.552.448-1 1.001-1s.999.448.999 1-.446 1-.999 1-1.001-.448-1.001-1zm6.2 0l-1.7 2.6-1.3-1.6-3.2 4h10l-3.8-5zm8.8-5v14h-20v-3h-4v-15h21v4h3zm-20 9v-9h15v-2h-17v11h2zm18-7h-16v10h16v-10z'/></svg></span>");
            CP("<span class='css-btn-slide'>");
            CP("<select class='selectwide' name='v' id='v'>");
            CP("<option value='1'>Cage</option>");
            CP("<option value='2'>Dominoes</option>");
            CP("<option value='3'>Grid</option>");
            CP("<option value='4'>Hexagon</option>");
            CP("<option value='5'>Squares</option>");
            CP("</select>");
            CP("</span>");
            CP("<span class='css-btn-text' onclick='changebg.submit()'>Change Background</span>");
            CP("</a></form>");

            CP("<form name='wifi' action='25' method='get'><a class='b'>");
            CP("<span class='css-btn-icon' onclick='wifi.submit()'><svg xmlns='http://www.w3.org/2000/svg' width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M0 7.244c3.071-3.24 7.314-5.244 12-5.244 4.687 0 8.929 2.004 12 5.244l-2.039 2.15c-2.549-2.688-6.071-4.352-9.961-4.352s-7.412 1.664-9.961 4.352l-2.039-2.15zm5.72 6.034c1.607-1.696 3.827-2.744 6.28-2.744s4.673 1.048 6.28 2.744l2.093-2.208c-2.143-2.261-5.103-3.659-8.373-3.659s-6.23 1.398-8.373 3.659l2.093 2.208zm3.658 3.859c.671-.708 1.598-1.145 2.622-1.145 1.023 0 1.951.437 2.622 1.145l2.057-2.17c-1.197-1.263-2.851-2.044-4.678-2.044s-3.481.782-4.678 2.044l2.055 2.17zm2.622 1.017c-1.062 0-1.923.861-1.923 1.923s.861 1.923 1.923 1.923 1.923-.861 1.923-1.923-.861-1.923-1.923-1.923z'/></svg></span>");
            CP("<span class='css-btn-slide'>");
            CP("<input type='text' id='ssid' name='ssid' size='25' maxlength='50' placeholder='SSID'>");
            CP("<input type='password' id='pass' name='pass' size='25' maxlength='50' placeholder='Password'></span>");
            CP("<span class='css-btn-text' onclick='wifi.submit()'>Change WiFi Network");
            CP("</span>");
            CP("</a></form>");

            CP("</div></div></div></div>");

            CP("<a class='s'><span class='css-btn-icon-footer' onclick=window.location.href='https://create.arduino.cc/projecthub/Ratti3'><svg xmlns='http://www.w3.org/2000/svg' width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M19 17c0 1.104-.896 2-2 2h-11c-1.104 0-2-.896-2-2v-11c0-1.104.896-2 2-2h11c1.104 0 2 .896 2 2v11zm-11 3v3h-1v-3h1zm4 0v3h-1v-3h1zm2 0v3h-1v-3h1zm-4 0v3h-1v-3h1zm6 0v3h-1v-3h1zm-8-20v3h-1v-3h1zm4 0v3h-1v-3h1zm2 0v3h-1v-3h1zm-4 0v3h-1v-3h1zm6 0v3h-1v-3h1zm4 15h3v1h-3v-1zm0-4h3v1h-3v-1zm0-2h3v1h-3v-1zm0 4h3v1h-3v-1zm0-6h3v1h-3v-1zm-20 8h3v1h-3v-1zm0-4h3v1h-3v-1zm0-2h3v1h-3v-1zm0 4h3v1h-3v-1zm0-6h3v1h-3v-1z'/></svg></span>");
            CP("<span class='css-btn-text-footer' onclick=window.location.href='https://create.arduino.cc/projecthub/Ratti3'>Arduino</span></a>");
            CP("<a class='s'><span class='css-btn-icon-footer' onclick=window.location.href='https://github.com/ratti3'><svg xmlns='http://www.w3.org/2000/svg' width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z'/></svg></span>");
            CP("<span class='css-btn-text-footer' onclick=window.location.href='https://github.com/ratti3'>GitHub</span></a>");
            CP("<a class='s'><span class='css-btn-icon-footer' onclick=window.location.href='https://easyeda.com/ratti3'><svg xmlns='http://www.w3.org/2000/svg' width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M5.829 6c-.412 1.165-1.524 2-2.829 2-1.656 0-3-1.344-3-3s1.344-3 3-3c1.305 0 2.417.835 2.829 2h13.671c2.484 0 4.5 2.016 4.5 4.5s-2.016 4.5-4.5 4.5h-4.671c-.412 1.165-1.524 2-2.829 2-1.305 0-2.417-.835-2.829-2h-4.671c-1.38 0-2.5 1.12-2.5 2.5s1.12 2.5 2.5 2.5h13.671c.412-1.165 1.524-2 2.829-2 1.656 0 3 1.344 3 3s-1.344 3-3 3c-1.305 0-2.417-.835-2.829-2h-13.671c-2.484 0-4.5-2.016-4.5-4.5s2.016-4.5 4.5-4.5h4.671c.412-1.165 1.524-2 2.829-2 1.305 0 2.417.835 2.829 2h4.671c1.38 0 2.5-1.12 2.5-2.5s-1.12-2.5-2.5-2.5h-13.671zm6.171 5c.552 0 1 .448 1 1s-.448 1-1 1-1-.448-1-1 .448-1 1-1z'/></svg></span>");
            CP("<span class='css-btn-text-footer' onclick=window.location.href='https://easyeda.com/ratti3'>EasyEDA</span></a>");
            CP("<a class='s'><span class='css-btn-icon-footer' onclick=window.location.href='https://youtube.com/ratti3'><svg xmlns='http://www.w3.org/2000/svg' width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c-6.627 0-12 5.373-12 12s5.373 12 12 12 12-5.373 12-12-5.373-12-12-12zm4.441 16.892c-2.102.144-6.784.144-8.883 0-2.276-.156-2.541-1.27-2.558-4.892.017-3.629.285-4.736 2.558-4.892 2.099-.144 6.782-.144 8.883 0 2.277.156 2.541 1.27 2.559 4.892-.018 3.629-.285 4.736-2.559 4.892zm-6.441-7.234l4.917 2.338-4.917 2.346v-4.684z'/></svg></span>");
            CP("<span class='css-btn-text-footer' onclick=window.location.href='https://youtube.com/ratti3'>YouTube</span></a>");
            CP("<a class='s'><span class='css-btn-icon-footer' onclick=window.location.href='https://ratti3.blogspot.com'><svg xmlns='http://www.w3.org/2000/svg' width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M15.5 14.625c0 .484-.387.875-.864.875h-5.273c-.477 0-.863-.392-.863-.875s.387-.875.863-.875h5.272c.478 0 .865.391.865.875zm-6.191-4.375h2.466c.448 0 .809-.392.809-.875s-.361-.875-.81-.875h-2.465c-.447 0-.809.392-.809.875s.362.875.809.875zm14.691 1.75c0 6.627-5.373 12-12 12s-12-5.373-12-12 5.373-12 12-12 12 5.373 12 12zm-5-1.039c0-.383-.311-.692-.691-.692h-1.138c-.583 0-.69-.446-.69-.996-.001-2.36-1.91-4.273-4.265-4.273h-2.952c-2.355 0-4.264 1.913-4.264 4.272v5.455c0 2.36 1.909 4.273 4.264 4.273h5.474c2.353 0 4.262-1.913 4.262-4.272v-3.767z'/></svg></span>");
            CP("<span class='css-btn-text-footer' onclick=window.location.href='https://ratti3.blogspot.com'>Blogger</span></a>");
            CP("<a class='s'><span class='css-btn-icon-footer' onclick=window.location.href='https://hackaday.io/Ratti3'><svg xmlns='http://www.w3.org/2000/svg' width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M12 3.385c-4.681 0-8.475 3.794-8.475 8.475 0 3.229 1.818 6.019 4.475 7.449v2.582c.672.906 2 1.109 2 1.109v-2h1v1.999h2v-1.999h1v2s1.328-.203 2-1.109v-2.582c2.657-1.431 4.475-4.22 4.475-7.449 0-4.681-3.794-8.475-8.475-8.475zm-4 11.615c-1.381 0-2.5-1.119-2.5-2.5 0-1.38 1.12-2.5 2.5-2.5s2.5 1.12 2.5 2.5c0 1.381-1.119 2.5-2.5 2.5zm2 2l2-2.541 2 2.541h-4zm6-2c-1.38 0-2.5-1.119-2.5-2.5 0-1.38 1.12-2.5 2.5-2.5 1.381 0 2.5 1.12 2.5 2.5 0 1.381-1.119 2.5-2.5 2.5zm4.54 2.904l3.46 3.46-2.636 2.636-3.48-3.48c1.035-.705 1.931-1.595 2.656-2.616zm-17.2-11.928l-3.34-3.339 2.636-2.637 3.32 3.32c-1.021.724-1.911 1.621-2.616 2.656zm14.704-2.656l3.32-3.32 2.636 2.637-3.34 3.339c-.705-1.035-1.595-1.932-2.616-2.656zm-11.928 17.2l-3.48 3.48-2.636-2.636 3.46-3.46c.725 1.021 1.621 1.911 2.656 2.616z'/></svg></span>");
            CP("<span class='css-btn-text-footer' onclick=window.location.href='https://hackaday.io/Ratti3'>Hackaday</span></a>");
            CP("<h3>&copy; 2021 : Ratti3 Technologies Corp</h3>");
            CP("</div></body></html>");
            CPL();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        // Check the client requests
        if (currentLine.startsWith("GET /10?v=")) {    // Slot machine
          repeat = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' '))).toInt();
          setting = 10;
        } else if (currentLine.endsWith("GET /11?")) { // Run BME280
          setting = 11;
        } else if (currentLine.endsWith("GET /12?")) { // NTP Sync
          setting = 12;
        } else if (currentLine.endsWith("GET /13?")) { // Enable/Disable NTP
          setting = 13;
        } else if (currentLine.endsWith("GET /14?")) { // Enable/Disable Display
          setting = 14;
        } else if (currentLine.endsWith("GET /15?")) { // Enable/Disable PIR
          setting = 15;
        } else if (currentLine.endsWith("GET /16?")) { // Enable Disable Light Sensor
          setting = 16;
        } else if (currentLine.endsWith("GET /17?")) { // Switch between 5V and 12V
          setting = 17;
        } else if (currentLine.startsWith("GET /18?v=")) {
          _settings->flashBrightness = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' '))).toInt();
          Serial.println(_settings->flashBrightness);
          setting = 18;
        } else if (currentLine.endsWith("GET /19")) {
          setting = 19;
        } else if (currentLine.endsWith("GET /20")) {
          setting = 20;
        } else if (currentLine.endsWith("GET /21")) {
          _timetask->showDate();
        } else if (currentLine.startsWith("GET /br?disp=")) {
          setting = 21;
        } else if (currentLine.endsWith("GET /20")) {
          setting = 22;
        } else if (currentLine.startsWith("GET /23?v=")) {
          _settings->flashFont = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' '))).toInt();
          Serial.println(_settings->flashFont);
          setting = 23;
        } else if (currentLine.startsWith("GET /24?v=")) {
          _settings->flashBackground = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' '))).toInt();
          Serial.println(_settings->flashBackground);
          setting = 24;
        } else if (currentLine.endsWith("GET /20?")) {
          setting = 25;
        }
      }
    }
    client.stop();
    // Run functions if a button was pressed
    switch (setting) {
      case 10: // Slot machine
        _nixie->runSlotMachine(repeat);
        break;
      case 11: // Run BME280
        _i2c->displayTHP();
        break;
      case 12: // NTP Sync
        getNTP();
        break;
      case 13: // Enable/Disable NTP
        _settings->flashNTP = !_settings->flashNTP;
        _settings->rwSettings(13, 1);
        break;
      case 14: // Enable/Disable Display
        _hv->switchOff();
        break;
      case 15: // Enable/Disable PIR
        _settings->rwSettings(15, 1);
        break;
      case 16: // Enable Disable Light Sensor
        _settings->rwSettings(16, 1);
        break;
      case 17: // Switch between 5V and 12V
        _settings->rwSettings(17, 1);
        break;
      case 18: // Set brightness
        _fade->setBrightness();
        break;
      case 19: // UTC Offset
        
        break;
      case 20: // Set time
        
        break;
      case 21: // Set date
        
        break;
      case 22: // Set on/off hour
      
        break;
      case 23: // Change WebUI Font
        _settings->rwSettings(23, 1);
        break;
      case 24: // Change WebUI Backgound
        _settings->rwSettings(24, 1);
        break;
      case 25: // Change WiFi
        _settings->rwSettings(25, 1);
        Serial.println("[DEBUG] Got the new WiFi credentials, saving to flash and restarting WiFi");
        break;
    }
  }
}

// Start the access point
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

// Listen for incoming client connections
void WiFiTask::apServer() {
  WiFiClient client = _server->available();   

  if (client) {
    String currentLine = "";
    bool gotCreds = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            CPL("HTTP/1.1 200 OK");
            CPL("Content-type:text/html");
            CPL();
            CP("<html><head><style>");
            CP("div {text-align: center; font-family: sans-serif;}");
            CP("input[type=submit] {background-color: #4CAF50; border: none; color: white; padding: 16px 32px; text-decoration: none; margin: 4px 2px; width: 400px; height: 80px; font-size: 40px;}");
            CP("input[type=text], [type=password] { width: 400px; padding: 12px 20px; margin: 8px 0; box-sizing: border-box; height: 80px; border: 4px solid red; border-radius: 10px; font-size: 40px;}");
            CP("label {font-size: 40px;}");
            CP("table {margin: auto;}");
            CP("</style></head><body>");
            CP("<div>");
            CP("<h2>Arduino Nano 33 IoT Nixie Clock : Living Room</h2>");
            CP("<h3>WiFi parameters:</h3><br>");
            CP("<form action='ap' method='get'>");
            CP("<table><tr><th><label>SSID: </label></th><th><label>Password: </label></th></tr>");
            CP("<tr><td><input type='text' name='ssid' maxlength='50'></td><td><input type='password' name='password' maxlength='50'></td></tr></table>");
            CP("<input type='submit' value='Save &amp; Connect'>");
            CP("</form>");
            CP("<br>&copy; 2021 : Ratti3 Technologies Corp</div>");
            CP("</body></html>");
            CPL();
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
      _settings->rwSettings(25, 1);
      // Stop the current AP SSID
      WiFi.disconnect();
      connectWiFi();
      _timeclient->begin();
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
