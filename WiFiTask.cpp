#include "WiFiTask.h"

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
  
  _settings->rwSettings(26, 0);
  // Use saved credentials from flash
  if (_settings->noSSID) {
    WiFiFail = 1;
    return;
  }

  byte count = 0;
  // Keep track of the WiFi connection status
  WiFiFail = 0;

  SP("Connecting to SSID: ");
  SP(_settings->ssid);
  SP("..");

  while (WiFi.status() != WL_CONNECTED) {
    _fade->fadeIn();
    WiFi.begin(_settings->ssid, _settings->pass);
    _fade->fadeOut();
    count++;
    if (count > 3) {
      SPL(" ");
      SP("Failed to connect to WiFi network: ");
      SPL(_settings->ssid);
      SPL(" ");
      WiFiFail = 1;
      break;
    }
  }

  if (!WiFiFail) {
    SPL("..connected");
    printWifiStatus();
  }
}

// Start the NTP client and set the UTC Offset from saved flash 
void WiFiTask::ntpBegin() {
  _timeclient->begin();
  _timeclient->setPoolServerName(_settings->ntpServerName);
  _timeclient->setTimeOffset(_settings->flashUTCOffset * 3600);
}

// Start the web server
void WiFiTask::serverBegin() {
  _server->begin();
}

// NTP function for sending UDP packet and receiving UNIX time and setting the DS3231 time
void WiFiTask::getNTP() {
  if (WiFiFail) {
    return;
  }

  _nixie->disableAllSegments();
  _nixie->updateDisplay();
  
  // Send UDP packet process the reply
  SPL("Sending NTP request...");
  _fade->fadeIn();
  _timeclient->update();
  _fade->fadeOut();
  delay(5);
  if (_timeclient->getEpochTime() > 100000) {
    _i2c->adjustTime(_timeclient->getEpochTime());
    SP(" with UNIX Time: ");
    SPL(_timeclient->getEpochTime());
  } else {
    SPL(" ");
    SPL("NTP Request Failed");
    SPL(" ");
  }
}

// Prints WiFi connection info
void WiFiTask::printWifiStatus() {
  // Print the SSID of the network you're attached to:
  SP("SSID: ");
  SP(WiFi.SSID());
  // Get the IP address
  IPAddress ip = WiFi.localIP();
  // Store the IP address for later
  ipAddress[0] = ip[0];
  ipAddress[1] = ip[1];
  ipAddress[2] = ip[2];
  ipAddress[3] = ip[3];
  // Print your WiFi IP address:
  SP(", IP Address: ");
  SPL(ip);
  // Print the signal strength:
  long rssi = WiFi.RSSI();
  SP("Signal Strength (RSSI): ");
  SP(rssi);
  SPL(" dBm");
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

  // Holds run state of the clicked buttons
  byte setting = 0;

  // Used to prevent repetitive client request data gathering
  byte i = 0;

  // Used to select correct option dropdowns
  const char* selected = "";

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

            switch (_settings->flashFont) {
              case 1:
                _settings->webFont = "Audiowide";
                break;
              case 2:
                _settings->webFont = "Codystar";
                break;
              case 3:
                _settings->webFont = "Gugi";
                break;
              case 4:
                _settings->webFont = "Michroma";
                break;
              case 5:
                _settings->webFont = "Nova Round";
                break;
              case 6:
                _settings->webFont = "Orbitron";
                break;
              case 7:
                _settings->webFont = "Revalia";
                break;
              case 8:
                _settings->webFont = "Slackey";
                break;
              default:
                _settings->webFont = "Audiowide";
                break;
            }

            // Set the WebUI background based on the saved setting
            switch (_settings->flashBackground) {
              case 1: // Cage
                CP("body {background-color: #ffffff; background-image: url(\"data:image/svg+xml,%3Csvg width='32' height='26' viewBox='0 0 32 26' xmlns='http://www.w3.org/2000/svg' %3E%3Cpath d='M14 0v3.994C14 7.864 10.858 11 7 11c-3.866 0-7-3.138-7-7.006V0h2v4.005C2 6.765 4.24 9 7 9c2.756 0 5-2.236 5-4.995V0h2zm0 26v-5.994C14 16.138 10.866 13 7 13c-3.858 0-7 3.137-7 7.006V26h2v-6.005C2 17.235 4.244 15 7 15c2.76 0 5 2.236 5 4.995V26h2zm2-18.994C16 3.136 19.142 0 23 0c3.866 0 7 3.138 7 7.006v9.988C30 20.864 26.858 24 23 24c-3.866 0-7-3.138-7-7.006V7.006zm2-.01C18 4.235 20.244 2 23 2c2.76 0 5 2.236 5 4.995v10.01C28 19.765 25.756 22 23 22c-2.76 0-5-2.236-5-4.995V6.995z' fill='%23dddddd' fill-opacity='0.4' fill-rule='evenodd'/%3E%3C/svg%3E\");}");
                break;
              case 2: // Dominoes
                CP("body {background-color: #eeeeee; background-image: url(\"data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='126' height='84' viewBox='0 0 126 84'%3E%3Cg fill-rule='evenodd'%3E%3Cg fill='%23cccccc' fill-opacity='0.4'%3E%3Cpath d='M126 83v1H0v-2h40V42H0v-2h40V0h2v40h40V0h2v40h40V0h2v83zm-2-1V42H84v40h40zM82 42H42v40h40V42zm-50-6a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM8 12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm96 12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm-42 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm30-12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM20 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm12 24a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM8 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM8 78a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm12 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm54 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM50 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM50 78a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm54-12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm12 12a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM92 54a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24 0a4 4 0 1 1 0-8 4 4 0 0 1 0 8zM92 78a4 4 0 1 1 0-8 4 4 0 0 1 0 8zm24-42a4 4 0 1 1 0-8 4 4 0 0 1 0 8z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E\");}");
                break;
              case 3: // Grid
                CP("body {background-color: #eeeeee; background-image: url(\"data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='100' height='100' viewBox='0 0 100 100'%3E%3Cg fill-rule='evenodd'%3E%3Cg fill='%23cccccc' fill-opacity='0.4'%3E%3Cpath opacity='.5' d='M96 95h4v1h-4v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4h-9v4h-1v-4H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15v-9H0v-1h15V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h9V0h1v15h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9h4v1h-4v9zm-1 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm9-10v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm9-10v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm9-10v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-10 0v-9h-9v9h9zm-9-10h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9zm10 0h9v-9h-9v9z'/%3E%3Cpath d='M6 5V0H5v5H0v1h5v94h1V6h94V5H6z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E\");}");
                break;
              case 4: // Hexagon
                CP("body {background-color: #eeeeee; background-image: url(\"data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='28' height='49' viewBox='0 0 28 49'%3E%3Cg fill-rule='evenodd'%3E%3Cg id='hexagons' fill='%23cccccc' fill-opacity='0.4' fill-rule='nonzero'%3E%3Cpath d='M13.99 9.25l13 7.5v15l-13 7.5L1 31.75v-15l12.99-7.5zM3 17.9v12.7l10.99 6.34 11-6.35V17.9l-11-6.34L3 17.9zM0 15l12.98-7.5V0h-2v6.35L0 12.69v2.3zm0 18.5L12.98 41v8h-2v-6.85L0 35.81v-2.3zM15 0v7.5L27.99 15H28v-2.31h-.01L17 6.35V0h-2zm0 49v-8l12.99-7.5H28v2.31h-.01L17 42.15V49h-2z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E\");}");
                break;
              case 5: // Squares
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

            CP("a {margin: 15px; text-decoration: none;}");
            CP("a.o, a.r, a.g, a.b, a.p, a.s, a.menu, a.menu_active {font-family: " + String(_settings->webFont) + ", Verdana; font-weight: bold; font-size: 26px; border-radius: 15px; cursor: pointer; display: inline-block;}");
            CP("a.s {border-radius: 30px;}");
            CP(".o {color: #c92200; border: 1px #e65f44 solid; background: linear-gradient(180deg, #f0cb11 5%, #f2ab1e 100%); text-shadow: 1px 1px 1px #ded17c; box-shadow: inset 1px 1px 2px 0px #f9eca0;}");
            CP(".r {color: #ffffff; border: 1px #d83526 solid; background: linear-gradient(180deg, #fe1900 5%, #ce0000 100%); text-shadow: 1px 1px 1px #b23d35; box-shadow: inset 1px 1px 2px 0px #f29d93;}");
            CP(".g {color: #306108; border: 1px #268a16 solid; background: linear-gradient(180deg, #77d42a 5%, #5cb811 100%); text-shadow: 1px 1px 1px #aade7c; box-shadow: inset 1px 1px 2px 0px #c9efab;}");
            CP(".b {color: #ffffff; border: 1px #84bcf3 solid; background: linear-gradient(180deg, #79bcff 5%, #378ee5 100%); text-shadow: 1px 1px 1px #528fcc; box-shadow: inset 1px 1px 2px 0px #bbdaf7;}");
            CP(".p {color: #ffffff; border: 1px #a946f5 solid; background: linear-gradient(180deg, #c579ff 5%, #a341ee 100%); text-shadow: 1px 1px 1px #8628ce; box-shadow: inset 1px 1px 2px 0px #e6cafc;}");
            CP(".s {color: #777777; border: 1px #dcdcdc solid; background: linear-gradient(180deg, #ededed 5%, #dfdfdf 100%); text-shadow: 1px 1px 1px #ffffff; box-shadow: inset 1px 1px 2px 0px #ffffff;}");
            CP(".o:hover, .float:hover {color: #8B0000; background: linear-gradient(180deg, #f2ab1e 5%, #f0cb11 100%);}");
            CP(".r:hover {color: #dddddd; background: linear-gradient(180deg, #ce0000 5%, #fe1900 100%);}");
            CP(".g:hover {color: #004d00; background: linear-gradient(180deg, #5cb811 5%, #77d42a 100%);}");
            CP(".b:hover {color: #dddddd; background: linear-gradient(180deg, #378ee5 5%, #79bcff 100%);}");
            CP(".p:hover {color: #dddddd; background: linear-gradient(180deg, #a341ee 5%, #c579ff 100%);}");
            CP(".s:hover {color: #666666; background: linear-gradient(180deg, #dfdfdf 5%, #ededed 100%);}");

            CP(".btn-icon {padding: 10px 17px; display: block; text-align: center; border-bottom: 1px solid rgba(255, 255, 255, 0.16); box-shadow: rgba(0, 0, 0, 0.14) 0px -1px 0px inset;}");
            CP(".btn-icon svg {vertical-align: middle; position: relative; font-size: 66px;}");
            CP(".btn-text {padding: 10px 0px; width: 400px; display: block; text-align: center;}");
            CP(".btn-slide {padding: 9px 0px; width: 400px; display: block; text-align: center; border-bottom: 1px solid rgba(255, 255, 255, 0.16); box-shadow: rgba(0, 0, 0, 0.14) 0px -1px 0px inset;}");
            CP(".btn-icon-footer {padding: 10px 5px; display: block; text-align: center; border-bottom: 1px solid rgba(255, 255, 255, 0.16); box-shadow: rgba(0, 0, 0, 0.14) 0px -1px 0px inset;}");
            CP(".btn-icon-footer svg {vertical-align: middle; position: relative; font-size: 40px;}");
            CP(".btn-text-footer {padding: 5px 0px; width: 175px; display: block; text-align: center;}");

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

            CP(".menu {color: #c92200; border-radius: 5px; border: 1px #eeb44f solid; background: linear-gradient(180deg, #ffc477 5%, #fb9e25 100%); text-shadow: 1px 1px 1px #cc9f52;  box-shadow: inset 1px 1px 2px 0px #fce2c1; cursor: pointer; display: inline-flex; align-items: center;}");
            CP(".menu:hover {color: #8B0000; background: linear-gradient(180deg, #f2ab1e 5%, #f0cb11 100%);}");
            CP(".menu-icon {padding: 0px 10px; border-right: 1px solid rgba(255, 255, 255, 0.16); box-shadow: rgba(0, 0, 0, 0.14) -1px 0px 0px inset;}");
            CP(".menu-icon svg {vertical-align: middle; position: relative;}");
            CP(".menu-text {padding: 0px 14px 0px 10px;}");

            CP(".menu_active {color: #306108; font-size: 16px; border-radius: 5px; border: 1px #268a16 solid; background: linear-gradient(180deg, #77d42a 5%, #5cb811 100%); text-shadow: 1px 1px 1px #aade7c; box-shadow: inset 1px 1px 2px 0px #c9efab; cursor: pointer; display: inline-flex; align-items: center;}");
            CP(".menu_active:hover {color: #004d00; background: linear-gradient(180deg, #5cb811 5%, #77d42a 100%);}");

            CP(".float {position: fixed;  width: 80px; height: 80px; bottom: 40px; right: 40px; background-color: #f0cb11; color: #c92200; border-radius: 50px; box-shadow: 2px 2px 3px #999;}");
            CP(".floater {margin: 17px;}");

            CP("</style>");

            CP("</head>");
            CP("<body>");

            CP("<script>");
            CP("function settings_hide(value) {");
            CP("  value.style.display = 'none';");
            CP("}");
            CP("function button_style(value) {");
            CP("  document.getElementById(value).className = 'menu';");
            CP("}");

            CP("function displaySettings(menu) {");
            CP("  const buttons = ['B1', 'B2', 'B3', 'B4', 'B5', 'B6'];");
            CP("  const menus = [M1, M2, M3, M4, M5, M6];");
            CP("  if (menus[menu].style.display === 'none') {");
            CP("    menus.forEach(settings_hide);");
            CP("    menus[menu].style.display = 'block';");
            CP("    buttons.forEach(button_style);");
            CP("    document.getElementById(buttons[menu]).className = 'menu_active';");
            CP("  } else {");
            CP("    menus.forEach(settings_hide);");
            CP("    menus[menu].style.display = 'none';");
            CP("    buttons.forEach(button_style);");
            CP("    document.getElementById(buttons[menu]).className = 'menu';");
            CP("  }");
            CP("}");
            CP("</script>");

            CP("<a href='./' class='float'><svg class='floater' width='44' height='44' viewBox='0 0 24 24' fill='currentColor'><path d='M13.5 2c-5.629 0-10.212 4.436-10.475 10h-3.025l4.537 5.917 4.463-5.917h-2.975c.26-3.902 3.508-7 7.475-7 4.136 0 7.5 3.364 7.5 7.5s-3.364 7.5-7.5 7.5c-2.381 0-4.502-1.119-5.876-2.854l-1.847 2.449c1.919 2.088 4.664 3.405 7.723 3.405 5.798 0 10.5-4.702 10.5-10.5s-4.702-10.5-10.5-10.5z'/></svg></a>");

            CP("<div><h1>" + String(_settings->webTitle) + " : " + String(_settings->webName) + "</h1><h3>Firmware Version : " + String(_settings->fwVersion) + "</h3>");

            // WiFi menu [26]
            if ((WiFiFail && setting == 0) || setting == 26) {
              CP("<a class='menu_active' onclick='displaySettings(0)' id='B1'>");
            } else {
              CP("<a class='menu' onclick='displaySettings(0)' id='B1'>");
            }
            CP("<span class='menu-icon'><svg width='16' height='16' viewBox='2 2 16 16' fill='currentColor'><path fill-rule='evenodd' d='M4.5 13.5A.5.5 0 015 13h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 9h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 5h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='menu-text'>WiFi</span></a>");

            // Settings menu [10, 14, 18, 27 & 28]
            if (setting == 10 || setting == 14 || setting == 18 || setting == 27 || setting == 28 || (!WiFiFail && setting == 0)) {
              CP("<a class='menu_active' onclick='displaySettings(1)' id='B2'>");
            } else {
              CP("<a class='menu' onclick='displaySettings(1)' id='B2'>");
            }
            CP("<span class='menu-icon'><svg width='16' height='16' viewBox='2 2 16 16' fill='currentColor'><path fill-rule='evenodd' d='M4.5 13.5A.5.5 0 015 13h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 9h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 5h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='menu-text'>Settings</span></a>");

            // Sensors menu [11, 15, 16 & 29]
            if (setting == 11 || setting == 15 || setting == 16 || setting == 29) {
              CP("<a class='menu_active' onclick='displaySettings(2)' id='B3'>");
            } else {
              CP("<a class='menu' onclick='displaySettings(2)' id='B3'>");
            }
            CP("<span class='menu-icon'><svg width='16' height='16' viewBox='2 2 16 16' fill='currentColor'><path fill-rule='evenodd' d='M4.5 13.5A.5.5 0 015 13h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 9h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 5h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='menu-text'>Sensors</span></a>");

            // Date/Time menu [12, 13, 19, 20, 21, 22 & 23]
            if (setting == 12 || setting == 13 || (setting >= 19 && setting <= 23)) {
              CP("<a class='menu_active' onclick='displaySettings(3)' id='B4'>");
            } else {
              CP("<a class='menu' onclick='displaySettings(3)' id='B4'>");
            }
            CP("<span class='menu-icon'><svg width='16' height='16' viewBox='2 2 16 16' fill='currentColor'><path fill-rule='evenodd' d='M4.5 13.5A.5.5 0 015 13h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 9h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 5h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='menu-text'>Date/Time</span></a>");

            // Power menu [17]
            if (setting == 17) {
              CP("<a class='menu_active' onclick='displaySettings(4)' id='B5'>");
            } else {
              CP("<a class='menu' onclick='displaySettings(4)' id='B5'>");
            }
            CP("<span class='menu-icon'><svg width='16' height='16' viewBox='2 2 16 16' fill='currentColor'><path fill-rule='evenodd' d='M4.5 13.5A.5.5 0 015 13h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 9h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 5h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='menu-text'>Power</span></a>");

            // UI menu [24, 25 & 30]
            if (setting == 24 || setting == 25 || setting == 30) {
              CP("<a class='menu_active' onclick='displaySettings(5)' id='B6'>");
            } else {
              CP("<a class='menu' onclick='displaySettings(5)' id='B6'>");
            }
            CP("<span class='menu-icon'><svg width='16' height='16' viewBox='2 2 16 16' fill='currentColor'><path fill-rule='evenodd' d='M4.5 13.5A.5.5 0 015 13h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 9h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5zm0-4A.5.5 0 015 5h10a.5.5 0 010 1H5a.5.5 0 01-.5-.5z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='menu-text'>UI</span></a>");

            // WiFi menu [26]
            CP("<div class='divTable'>");
            CP("<div class='divTableBody'>");
            if ((WiFiFail && setting == 0) || setting == 26) {
              CP("<div class='divTableRow' id='M1' style='display:block;')>");
            } else {
              CP("<div class='divTableRow' id='M1' style='display:none;')>");
            }
            CP("<div class='divTableCell'>");

            // 26 Change WiFi
            CP("<form name='wifi' action='26' method='get'><a class='b'>");
            CP("<span class='btn-icon' onclick='wifi.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M0 7.244c3.071-3.24 7.314-5.244 12-5.244 4.687 0 8.929 2.004 12 5.244l-2.039 2.15c-2.549-2.688-6.071-4.352-9.961-4.352s-7.412 1.664-9.961 4.352l-2.039-2.15zm5.72 6.034c1.607-1.696 3.827-2.744 6.28-2.744s4.673 1.048 6.28 2.744l2.093-2.208c-2.143-2.261-5.103-3.659-8.373-3.659s-6.23 1.398-8.373 3.659l2.093 2.208zm3.658 3.859c.671-.708 1.598-1.145 2.622-1.145 1.023 0 1.951.437 2.622 1.145l2.057-2.17c-1.197-1.263-2.851-2.044-4.678-2.044s-3.481.782-4.678 2.044l2.055 2.17zm2.622 1.017c-1.062 0-1.923.861-1.923 1.923s.861 1.923 1.923 1.923 1.923-.861 1.923-1.923-.861-1.923-1.923-1.923z'/></svg></span>");
            CP("<span class='btn-slide'>");
            CP("<input type='text' id='ssid' name='ssid' size='25' maxlength='50' placeholder='SSID'>");
            CP("<input type='password' id='pass' name='pass' size='25' maxlength='50' placeholder='Password'></span>");
            CP("<span class='btn-text' onclick='wifi.submit()'>Change WiFi Network");
            CP("</span>");
            CP("</a></form>");

            CP("</div></div></div></div>");

            // Settings menu [10, 14, 18, 27 & 28]
            CP("<div class='divTable'>");
            CP("<div class='divTableBody'>");
            if ((!WiFiFail && setting == 0) || setting == 10 || setting == 14 || setting == 18 || setting == 27 || setting == 28) {
              CP("<div class='divTableRow' id='M2' style='display:block;'>");
            } else {
              CP("<div class='divTableRow' id='M2' style='display:none;'>");
            }
            CP("<div class='divTableCell'>");

            // 10 Run Slot Machine
            CP("<form name='slot' action='10' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='slot.submit()'><svg width='66' height='66' viewBox='0 0 50 50' fill='currentColor'><path d='M49 10.489c.604-.457 1-1.173 1-1.989a2.5 2.5 0 1 0-5 0 2.49 2.49 0 0 0 1 1.989V26h-9v8h10v-.297A3.994 3.994 0 0 0 49.5 30c0-.691-.191-1.332-.5-1.899V10.489zM6 17c1.021 0 1.73.172 3 0 0 0-4.016 2.666-4.016 6H10s.006-5.554 3-9l-1-1s-2 1-4 0c0 0-2 0-4 1v-1H3v6h1s0-2 2-2zm14 0c1.021 0 1.73.172 3 0 0 0-4.016 2.666-4.016 6H24s.006-5.554 3-9l-1-1s-2 1-4 0c0 0-2 0-4 1v-1h-1v6h1s0-2 2-2zM38 7V5H25.88c-.456-2.002-2.239-3.5-4.38-3.5S17.576 2.998 17.12 5H5v2H0v41h44V7h-6zm-22 4h12v14H16V11zM2 11h12v14H2V11zm36 32H5v-7h33v7zm4-18H30V11h12v14zm-8-8c1.021 0 1.73.172 3 0 0 0-4.016 2.666-4.016 6H38s.006-5.554 3-9l-1-1s-2 1-4 0c0 0-2 0-4 1v-1h-1v6h1s0-2 2-2z'/></svg></span>");
            CP("<span class='btn-slide'><input type='range' id='v' name='v' value='" + String(_settings->flashSpin) + "' min='1' max='5' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashSpin) + "</output></span>");
            CP("<span class='btn-text' onclick='slot.submit()'>Set Hourly Spins</span>");
            CP("</a></form>");

            // 14 Enable/Disable display
            if (_hv->isOn()) {
              CP("<a href=\"/14\" class='r'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M19 6.734c0 4.164-3.75 6.98-3.75 10.266h-1.992c.001-2.079.996-3.826 1.968-5.513.913-1.585 1.774-3.083 1.774-4.753 0-3.108-2.518-4.734-5.004-4.734-2.482 0-4.996 1.626-4.996 4.734 0 1.67.861 3.168 1.774 4.753.972 1.687 1.966 3.434 1.967 5.513h-1.991c0-3.286-3.75-6.103-3.75-10.266 0-4.343 3.498-6.734 6.996-6.734 3.502 0 7.004 2.394 7.004 6.734zm-4 11.766c0 .276-.224.5-.5.5h-5c-.276 0-.5-.224-.5-.5s.224-.5.5-.5h5c.276 0 .5.224.5.5zm0 2c0 .276-.224.5-.5.5h-5c-.276 0-.5-.224-.5-.5s.224-.5.5-.5h5c.276 0 .5.224.5.5zm-1.701 3.159c-.19.216-.465.341-.752.341h-1.094c-.287 0-.562-.125-.752-.341l-1.451-1.659h5.5l-1.451 1.659zm-3.629-16.347l-1.188-.153c.259-1.995 1.5-3.473 3.518-3.847l.219 1.177c-1.947.361-2.433 1.924-2.549 2.823z'/></svg></span>");
              CP("<span class='btn-text'>Disable Display</span>");
            } else {
              CP("<a href=\"/14\" class='g'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M10.741 17h-1.991c0-.17-.016-.338-.035-.506l1.703-1.548c.197.653.323 1.332.323 2.054zm-.04 6.659c.19.216.465.341.753.341h1.093c.288 0 .562-.125.752-.341l1.451-1.659h-5.5l1.451 1.659zm3.799-3.659h-5c-.276 0-.5.224-.5.5s.224.5.5.5h5c.276 0 .5-.224.5-.5s-.224-.5-.5-.5zm0-2h-5c-.276 0-.5.224-.5.5s.224.5.5.5h5c.276 0 .5-.224.5-.5s-.224-.5-.5-.5zm1.707-8.315c-1.104 2.28-2.948 4.483-2.949 7.315h1.992c0-3.169 3.479-5.906 3.726-9.832l-2.769 2.517zm6.793-8.201l-20.654 18.75-1.346-1.5 6.333-5.728c-1.062-1.873-2.333-3.843-2.333-6.272 0-4.343 3.498-6.734 6.996-6.734 2.408 0 4.798 1.146 6.064 3.267l3.598-3.267 1.342 1.484zm-14.147 10.142l7.676-6.969c-.833-1.742-2.682-2.657-4.533-2.657-2.483 0-4.996 1.626-4.996 4.734 0 1.713.907 3.246 1.853 4.892z'/></svg></span>");
              CP("<span class='btn-text'>Enable Display</span>");
            }
            CP("</a></form>");

            // 27 Colon LED brightness
            CP("<form name='colon' action='27' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='colon.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M17 12c0 2.762-2.238 5-5 5s-5-2.238-5-5 2.238-5 5-5 5 2.238 5 5zm-9.184-5.599l-3.594-3.594-1.414 1.414 3.594 3.595c.402-.537.878-1.013 1.414-1.415zm4.184-1.401c.34 0 .672.033 1 .08v-5.08h-2v5.08c.328-.047.66-.08 1-.08zm5.598 2.815l3.594-3.595-1.414-1.414-3.594 3.595c.536.402 1.012.878 1.414 1.414zm-12.598 4.185c0-.34.033-.672.08-1h-5.08v2h5.08c-.047-.328-.08-.66-.08-1zm11.185 5.598l3.594 3.593 1.415-1.414-3.594-3.594c-.403.537-.879 1.013-1.415 1.415zm-9.784-1.414l-3.593 3.593 1.414 1.414 3.593-3.593c-.536-.402-1.011-.877-1.414-1.414zm12.519-5.184c.047.328.08.66.08 1s-.033.672-.08 1h5.08v-2h-5.08zm-6.92 8c-.34 0-.672-.033-1-.08v5.08h2v-5.08c-.328.047-.66.08-1 .08z'/></svg></span>");
            CP("<span class='btn-slide'><input type='range' id='v' name='v' value='" + String(_settings->flashColon) + "' min='0' max='255' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashColon) + "</output></span>");
            CP("<span class='btn-text' onclick='colon.submit()'>Colon LED Brightness</span>");
            CP("</a></form>");

            CP("</div><div class='divTableCell'>");

            // 18 Set Brightness
            CP("<form name='brightness' action='18' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='brightness.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 9c1.654 0 3 1.346 3 3s-1.346 3-3 3v-6zm0-2c-2.762 0-5 2.238-5 5s2.238 5 5 5 5-2.238 5-5-2.238-5-5-5zm-4.184-.599l-3.593-3.594-1.415 1.414 3.595 3.595c.401-.537.876-1.013 1.413-1.415zm4.184-1.401c.34 0 .672.033 1 .08v-5.08h-2v5.08c.328-.047.66-.08 1-.08zm5.598 2.815l3.595-3.595-1.414-1.414-3.595 3.595c.537.402 1.012.878 1.414 1.414zm-12.598 4.185c0-.34.033-.672.08-1h-5.08v2h5.08c-.047-.328-.08-.66-.08-1zm11.185 5.598l3.594 3.593 1.415-1.414-3.594-3.593c-.403.536-.879 1.012-1.415 1.414zm-9.784-1.414l-3.593 3.593 1.414 1.414 3.593-3.593c-.536-.402-1.011-.877-1.414-1.414zm12.519-5.184c.047.328.08.66.08 1s-.033.672-.08 1h5.08v-2h-5.08zm-6.92 8c-.34 0-.672-.033-1-.08v5.08h2v-5.08c-.328.047-.66.08-1 .08z'/></svg></span>");
            CP("<span class='btn-slide'><input type='range' id='v' name='v' value='" + String(_settings->flashBrightness) + "' min='1' max='255' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashBrightness) + "</output></span>");
            CP("<span class='btn-text' onclick='brightness.submit()'>Nixie Brightness</span>");
            CP("</a></form>");

            //28 Switch brightness
            CP("<form name='led' action='28' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='led.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 9c1.654 0 3 1.346 3 3s-1.346 3-3 3-3-1.346-3-3 1.346-3 3-3zm0-2c-2.762 0-5 2.238-5 5s2.238 5 5 5 5-2.238 5-5-2.238-5-5-5zm-4.184-.599l-3.593-3.594-1.415 1.414 3.595 3.595c.401-.537.876-1.013 1.413-1.415zm4.184-1.401c.34 0 .672.033 1 .08v-5.08h-2v5.08c.328-.047.66-.08 1-.08zm5.598 2.815l3.595-3.595-1.414-1.414-3.595 3.595c.537.402 1.012.878 1.414 1.414zm-12.598 4.185c0-.34.033-.672.08-1h-5.08v2h5.08c-.047-.328-.08-.66-.08-1zm11.185 5.598l3.594 3.593 1.415-1.414-3.594-3.593c-.403.536-.879 1.012-1.415 1.414zm-9.784-1.414l-3.593 3.593 1.414 1.414 3.593-3.593c-.536-.402-1.011-.877-1.414-1.414zm12.519-5.184c.047.328.08.66.08 1s-.033.672-.08 1h5.08v-2h-5.08zm-6.92 8c-.34 0-.672-.033-1-.08v5.08h2v-5.08c-.328.047-.66.08-1 .08z'/></svg></span>");
            CP("<span class='btn-slide'><input type='range' id='r' name='r' value='" + String(_settings->flashLED1) + "' min='0' max='255' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashLED1) + "</output> : LED 1</span>");
            CP("<span class='btn-slide'><input type='range' id='g' name='g' value='" + String(_settings->flashLED2) + "' min='0' max='255' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashLED2) + "</output> : LED 2</span>");
            CP("<span class='btn-slide'><input type='range' id='b' name='b' value='" + String(_settings->flashLED3) + "' min='0' max='255' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashLED3) + "</output> : LED 3</span>");
            CP("<span class='btn-text' onclick='led.submit()'>Switch LED Brightness</span>");
            CP("</a></form>");

            CP("</div></div></div></div>");

            // Sensors menu [11, 15, 16, 29]
            CP("<div class='divTable'>");
            CP("<div class='divTableBody'>");
            if (setting == 11 || setting == 15 || setting == 16) {
              CP("<div class='divTableRow' id='M3' style='display:block'>");
            } else {
              CP("<div class='divTableRow' id='M3' style='display:none'>");
            }
            CP("<div class='divTableCell'>");

            // 29 Low Lux Level Threshold
            CP("<form name='lux' action='29' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='colon.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 2c5.514 0 10 4.486 10 10s-4.486 10-10 10-10-4.486-10-10 4.486-10 10-10zm0-2c-6.627 0-12 5.373-12 12s5.373 12 12 12 12-5.373 12-12-5.373-12-12-12zm3 12c0 1.657-1.343 3-3 3s-3-1.343-3-3 1.343-3 3-3 3 1.343 3 3zm-3-5c.343 0 .677.035 1 .101v-3.101h-2v3.101c.323-.066.657-.101 1-.101zm-2.755.832l-2.195-2.196-1.414 1.414 2.195 2.195c.372-.561.853-1.042 1.414-1.413zm-2.245 4.168c0-.343.035-.677.101-1h-3.101v2h3.101c-.066-.323-.101-.657-.101-1zm9.169-2.754l2.195-2.195-1.414-1.415-2.195 2.195c.561.372 1.042.853 1.414 1.415zm.73 1.754c.066.323.101.657.101 1s-.035.677-.101 1h3.101v-2h-3.101zm-2.144 5.168l2.195 2.195 1.414-1.414-2.195-2.195c-.372.562-.853 1.043-1.414 1.414zm-6.924-1.414l-2.195 2.196 1.414 1.414 2.195-2.195c-.561-.372-1.042-.853-1.414-1.415zm4.169 2.246c-.343 0-.677-.035-1-.101v3.101h2v-3.101c-.323.066-.657.101-1 .101z'/></svg></span>");
            CP("<span class='btn-slide'><input type='range' id='v' name='v' value='" + String(_settings->flashLux) + "' min='1' max='10' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashLux) + "</output></span>");
            CP("<span class='btn-text' onclick='lux.submit()'>Low Lux Level</span>");
            CP("</a></form>");

            // 11 Display THP
            CP("<a href=\"/11\" class='o'>");
            CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 9.312l-1.762.491 1.562.881-.491.871-1.562-.881.491 1.762-.963.268-.76-2.724-2.015-1.126v1.939l2 2-.707.707-1.293-1.293v1.793h-1v-1.793l-1.293 1.293-.707-.707 2-2v-1.939l-2.015 1.126-.761 2.724-.963-.268.491-1.762-1.562.882-.491-.871 1.562-.881-1.761-.492.269-.962 2.725.76 1.982-1.11-1.983-1.109-2.724.759-.269-.962 1.762-.491-1.563-.882.491-.871 1.562.881-.49-1.762.963-.269.76 2.725 2.015 1.128v-1.94l-2-2 .707-.707 1.293 1.293v-1.793h1v1.793l1.293-1.293.707.707-2 2v1.94l2.016-1.127.76-2.725.963.269-.492 1.761 1.562-.881.491.871-1.562.881 1.762.492-.269.962-2.725-.76-1.982 1.11 1.982 1.109 2.725-.76.269.963zm4-5.812v7.525c0 1.57-.514 2.288-1.41 3.049-1.011.859-1.59 2.107-1.59 3.426 0 2.481 2.019 4.5 4.5 4.5s4.5-2.019 4.5-4.5c0-1.319-.579-2.567-1.589-3.426-.897-.762-1.411-1.48-1.411-3.049v-7.525c0-.827-.673-1.5-1.5-1.5s-1.5.673-1.5 1.5zm5 0v7.525c0 .587.258 1.145.705 1.525 1.403 1.192 2.295 2.966 2.295 4.95 0 3.59-2.909 6.5-6.5 6.5s-6.5-2.91-6.5-6.5c0-1.984.892-3.758 2.295-4.949.447-.381.705-.94.705-1.526v-7.525c0-1.934 1.567-3.5 3.5-3.5s3.5 1.566 3.5 3.5zm0 14c0 1.934-1.567 3.5-3.5 3.5s-3.5-1.566-3.5-3.5c0-1.141.599-2.084 1.393-2.781 1.01-.889 1.607-1.737 1.607-3.221v-.498h1v.498c0 1.486.595 2.33 1.607 3.221.794.697 1.393 1.64 1.393 2.781z'/></svg></span>");
            CP("<span class='btn-text'>&deg;C | RH | mbar</span>");
            CP("</a>");

            CP("</div><div class='divTableCell'>");

            // 15 Toggle PIR
            if (_settings->flashPIR) {
              CP("<a href=\"/15\" class='r'>");
              CP("<span class='btn-icon'><svg width='66' height='66' fill-rule='evenodd' clip-rule='evenodd' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c6.623 0 12 5.377 12 12s-5.377 12-12 12-12-5.377-12-12 5.377-12 12-12zm0 2c5.52 0 10 4.481 10 10 0 5.52-4.48 10-10 10-5.519 0-10-4.48-10-10 0-5.519 4.481-10 10-10zm-2 2.252v1.038c-2.89.862-5 3.542-5 6.71 0 3.863 3.137 7 7 7 1.932 0 3.682-.784 4.949-2.051l.706.706c-1.448 1.449-3.448 2.345-5.655 2.345-4.416 0-8-3.585-8-8 0-3.725 2.551-6.859 6-7.748zm0 3.165v1.119c-1.195.692-2 1.984-2 3.464 0 2.208 1.792 4 4 4 1.104 0 2.104-.448 2.828-1.172l.707.707c-.905.904-2.155 1.465-3.535 1.465-2.76 0-5-2.24-5-5 0-2.049 1.235-3.811 3-4.583zm1 2.851v-6.268c0-.265.105-.52.293-.707.187-.188.442-.293.707-.293.265 0 .52.105.707.293.188.187.293.442.293.707v6.268c.598.346 1 .992 1 1.732 0 1.104-.896 2-2 2s-2-.896-2-2c0-.74.402-1.386 1-1.732z'/></svg></span>");
              CP("<span class='btn-text'>Disable PIR</span>");
            } else {
              CP("<a href=\"/15\" class='g'>");
              CP("<span class='btn-icon'><svg width='66' height='66' fill-rule='evenodd' clip-rule='evenodd' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c6.623 0 12 5.377 12 12s-5.377 12-12 12-12-5.377-12-12 5.377-12 12-12zm0 2c5.52 0 10 4.481 10 10 0 5.52-4.48 10-10 10-5.519 0-10-4.48-10-10 0-5.519 4.481-10 10-10zm-2 2.252v1.038c-2.89.862-5 3.542-5 6.71 0 3.863 3.137 7 7 7 1.932 0 3.682-.784 4.949-2.051l.706.706c-1.448 1.449-3.448 2.345-5.655 2.345-4.416 0-8-3.585-8-8 0-3.725 2.551-6.859 6-7.748zm0 3.165v1.119c-1.195.692-2 1.984-2 3.464 0 2.208 1.792 4 4 4 1.104 0 2.104-.448 2.828-1.172l.707.707c-.905.904-2.155 1.465-3.535 1.465-2.76 0-5-2.24-5-5 0-2.049 1.235-3.811 3-4.583zm1 2.851v-6.268c0-.265.105-.52.293-.707.187-.188.442-.293.707-.293.265 0 .52.105.707.293.188.187.293.442.293.707v6.268c.598.346 1 .992 1 1.732 0 1.104-.896 2-2 2s-2-.896-2-2c0-.74.402-1.386 1-1.732z'/></svg></span>");
              CP("<span class='btn-text'>Enable PIR</span>");
            }
              CP("</a>");
              
            // 16 Toggle Light Sensor
            if (_settings->flashLight) {
              CP("<a href=\"/16\" class='r'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M16.169 14.754l2.195 2.195-1.414 1.414-2.195-2.195c.561-.371 1.042-.852 1.414-1.414zm3.831-3.754h-3.101c.066.323.101.657.101 1s-.035.677-.101 1h3.101v-2zm-7 5.899c-.323.066-.657.101-1 .101s-.677-.035-1-.101v3.101h2v-3.101zm9.055-11.437l-1.457 1.457c.884 1.491 1.402 3.225 1.402 5.081 0 5.514-4.486 10-10 10-1.856 0-3.59-.518-5.081-1.402l-1.457 1.457c1.882 1.226 4.125 1.945 6.538 1.945 6.627 0 12-5.373 12-12 0-2.414-.72-4.656-1.945-6.538zm1.945-4.048l-22.586 22.586-1.414-1.414 2.854-2.854c-1.772-2.088-2.854-4.779-2.854-7.732 0-6.627 5.373-12 12-12 2.953 0 5.644 1.082 7.732 2.853l2.854-2.853 1.414 1.414zm-19.733 16.905l5.03-5.03c-.189-.39-.297-.826-.297-1.289 0-1.657 1.343-3 3-3 .463 0 .899.108 1.289.297l5.03-5.03c-1.724-1.413-3.922-2.267-6.319-2.267-5.514 0-10 4.486-10 10 0 2.397.854 4.595 2.267 6.319zm8.733-11.218v-3.101h-2v3.101c.323-.066.657-.101 1-.101s.677.035 1 .101zm-9 3.899v2h3.101c-.066-.323-.101-.657-.101-1s.035-.677.101-1h-3.101zm3.05-5.364l-1.414 1.414 2.195 2.195c.372-.562.853-1.042 1.414-1.414l-2.195-2.195z'/></svg></span>");
              CP("<span class='btn-text'>Disable Light Sensor</span>");
            } else {
              CP("<a href=\"/16\" class='g'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 2c5.514 0 10 4.486 10 10s-4.486 10-10 10-10-4.486-10-10 4.486-10 10-10zm0-2c-6.627 0-12 5.373-12 12s5.373 12 12 12 12-5.373 12-12-5.373-12-12-12zm3 12c0 1.657-1.343 3-3 3s-3-1.343-3-3 1.343-3 3-3 3 1.343 3 3zm-3-5c.343 0 .677.035 1 .101v-3.101h-2v3.101c.323-.066.657-.101 1-.101zm-2.755.832l-2.195-2.196-1.414 1.414 2.195 2.195c.372-.561.853-1.042 1.414-1.413zm-2.245 4.168c0-.343.035-.677.101-1h-3.101v2h3.101c-.066-.323-.101-.657-.101-1zm9.169-2.754l2.195-2.195-1.414-1.415-2.195 2.195c.561.372 1.042.853 1.414 1.415zm.73 1.754c.066.323.101.657.101 1s-.035.677-.101 1h3.101v-2h-3.101zm-2.144 5.168l2.195 2.195 1.414-1.414-2.195-2.195c-.372.562-.853 1.043-1.414 1.414zm-6.924-1.414l-2.195 2.196 1.414 1.414 2.195-2.195c-.561-.372-1.042-.853-1.414-1.415zm4.169 2.246c-.343 0-.677-.035-1-.101v3.101h2v-3.101c-.323.066-.657.101-1 .101z'/></svg></span>");
              CP("<span class='btn-text'>Enable Light Sensor</span>");
            }
              CP("</a>");

            CP("</div></div></div></div>");

            // Date/Time menu [12, 13, 19, 20, 21, 22 & 23]
            CP("<div class='divTable'>");
            CP("<div class='divTableBody'>");
            if (setting == 12 || setting == 13 || (setting >= 19 && setting <= 23)) {
              CP("<div class='divTableRow' id='M4' style='display:block'>");
            } else {
              CP("<div class='divTableRow' id='M4' style='display:none'>");
            }
            CP("<div class='divTableCell'>");

            // 12 Run NTP Sync
            CP("<a href=\"/12\" class='o'>");
            CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M12 1c6.074 0 11 4.925 11 11 0 1.042-.154 2.045-.425 3h-2.101c.335-.94.526-1.947.526-3 0-4.962-4.037-9-9-9-1.706 0-3.296.484-4.654 1.314l1.857 2.686h-6.994l2.152-7 1.85 2.673c1.683-1.049 3.658-1.673 5.789-1.673zm4.646 18.692c-1.357.826-2.944 1.308-4.646 1.308-4.963 0-9-4.038-9-9 0-1.053.191-2.06.525-3h-2.1c-.271.955-.425 1.958-.425 3 0 6.075 4.925 11 11 11 2.127 0 4.099-.621 5.78-1.667l1.853 2.667 2.152-6.989h-6.994l1.855 2.681zm-6.54-4.7c.485-.514 1.154-.832 1.894-.832s1.408.318 1.894.832l-1.894 2.008-1.894-2.008zm-.874-.927c.709-.751 1.688-1.215 2.768-1.215s2.059.465 2.768 1.216l1.164-1.236c-1.006-1.067-2.396-1.727-3.932-1.727s-2.926.66-3.932 1.727l1.164 1.235zm-2.038-2.163c1.23-1.304 2.929-2.11 4.806-2.11s3.576.807 4.806 2.111l1.194-1.267c-1.535-1.629-3.656-2.636-6-2.636s-4.465 1.007-6 2.636l1.194 1.266z'/></svg></span>");
            CP("<span class='btn-text'>NTP Sync</span>");
            CP("</a>");

            // 20 Set Time
            CP("<form name='settime' action='20' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='settime.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M13 12l-.688-4h-.609l-.703 4c-.596.347-1 .984-1 1.723 0 1.104.896 2 2 2s2-.896 2-2c0-.739-.404-1.376-1-1.723zm-1-8c-5.522 0-10 4.477-10 10s4.478 10 10 10 10-4.477 10-10-4.478-10-10-10zm0 18c-4.411 0-8-3.589-8-8s3.589-8 8-8 8 3.589 8 8-3.589 8-8 8zm-2-19.819v-2.181h4v2.181c-1.438-.243-2.592-.238-4 0zm9.179 2.226l1.407-1.407 1.414 1.414-1.321 1.321c-.462-.484-.964-.926-1.5-1.328z'/></svg></span>");
            CP("<span class='btn-slide'>");
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
            CP("<span class='btn-text' onclick='settime.submit()'>Set Time</span>");
            CP("</a></form>");

            // 22 Set On/Off Hour
            CP("<form name='onoffhour' action='22' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='onoffhour.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M15.91 13.34l2.636-4.026-.454-.406-3.673 3.099c-.675-.138-1.402.068-1.894.618-.736.823-.665 2.088.159 2.824.824.736 2.088.665 2.824-.159.492-.55.615-1.295.402-1.95zm-3.91-10.646v-2.694h4v2.694c-1.439-.243-2.592-.238-4 0zm8.851 2.064l1.407-1.407 1.414 1.414-1.321 1.321c-.462-.484-.964-.927-1.5-1.328zm-18.851 4.242h8v2h-8v-2zm-2 4h8v2h-8v-2zm3 4h7v2h-7v-2zm21-3c0 5.523-4.477 10-10 10-2.79 0-5.3-1.155-7.111-3h3.28c1.138.631 2.439 1 3.831 1 4.411 0 8-3.589 8-8s-3.589-8-8-8c-1.392 0-2.693.369-3.831 1h-3.28c1.811-1.845 4.321-3 7.111-3 5.523 0 10 4.477 10 10z'/></svg></span>");
            CP("<span class='btn-slide'><input type='range' id='o' name='o' value='" + String(_settings->flashOnHour) + "' min='5' max='8' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashOnHour) + "</output> : On Hour</span></span>");
            CP("<span class='btn-slide'><input type='range' id='x' name='x' value='" + String(_settings->flashOffHour) + "' min='21' max='23' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashOffHour) + "</output> : Off Hour</span></span>");
            CP("<span class='btn-text' onclick='onoffhour.submit()'>Set On/Off Hour</span>");
            CP("</a></form>");

            // 23 Set NTP Pool
            CP("<form name='ntppool' action='23' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='ntppool.submit()'><svg width='66' height='66' viewBox='2 2 16 16' fill='currentColor'><path d='M6.887 7.2l-.964-.165A2.5 2.5 0 105.5 12H8v1H5.5a3.5 3.5 0 11.59-6.95 5.002 5.002 0 119.804 1.98A2.501 2.501 0 0115.5 13H12v-1h3.5a1.5 1.5 0 00.237-2.981L14.7 8.854l.216-1.028a4 4 0 10-7.843-1.587l-.185.96z'/><path fill-rule='evenodd' d='M7 14.5a.5.5 0 01.707 0L10 16.793l2.293-2.293a.5.5 0 11.707.707l-2.646 2.647a.5.5 0 01-.708 0L7 15.207a.5.5 0 010-.707z' clip-rule='evenodd'/><path fill-rule='evenodd' d='M10 8a.5.5 0 01.5.5v8a.5.5 0 01-1 0v-8A.5.5 0 0110 8z' clip-rule='evenodd'/></svg></span>");
            CP("<span class='btn-slide'>");
            CP("<select class='selectwide' name='v' id='v'>");
            for(byte i = 1; i < 7; ++i) {
              if (i == _settings->flashNTPPool) {
                selected = "selected";
              } else {
                selected = "";
              }
              switch (i) {
                case 1:
                  CP("<option value='1' " + String(selected) + ">africa.pool.ntp.org</option>");
                  break;
                case 2:
                  CP("<option value='2' " + String(selected) + ">asia.pool.ntp.org</option>");
                  break;
                case 3:
                  CP("<option value='3' " + String(selected) + ">europe.pool.ntp.org</option>");
                  break;
                case 4:
                  CP("<option value='4' " + String(selected) + ">north-america.pool.ntp.org</option>");
                  break;
                case 5:
                  CP("<option value='5' " + String(selected) + ">oceania.pool.ntp.org</option>");
                  break;
                case 6:
                  CP("<option value='6' " + String(selected) + ">south-america.pool.ntp.org</option>");
                  break;
              }
            }
            CP("</select>");
            CP("</span>");
            CP("<span class='btn-text' onclick='ntppool.submit()'>Set NTP Pool</span>");
            CP("</a></form>");

            CP("</div><div class='divTableCell'>");

            // 13 Toggle NTP Sync
            if (_settings->flashNTP) {
              CP("<a href=\"/13\" class='r'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M23 12c0 1.042-.154 2.045-.425 3h-2.101c.335-.94.526-1.947.526-3 0-4.962-4.037-9-9-9-1.706 0-3.296.484-4.654 1.314l1.857 2.686h-6.994l2.152-7 1.85 2.673c1.683-1.049 3.658-1.673 5.789-1.673 6.074 0 11 4.925 11 11zm-6.354 7.692c-1.357.826-2.944 1.308-4.646 1.308-4.963 0-9-4.038-9-9 0-1.053.191-2.06.525-3h-2.1c-.271.955-.425 1.958-.425 3 0 6.075 4.925 11 11 11 2.127 0 4.099-.621 5.78-1.667l1.853 2.667 2.152-6.989h-6.994l1.855 2.681zm-.646-5.108l-2.592-2.594 2.592-2.587-1.416-1.403-2.591 2.589-2.588-2.589-1.405 1.405 2.593 2.598-2.593 2.592 1.405 1.405 2.601-2.596 2.591 2.596 1.403-1.416z'/></svg></span>");
              CP("<span class='btn-text'>Disable NTP</span>");
            } else {
              CP("<a href=\"/13\" class='g'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M23 12c0 1.042-.154 2.045-.425 3h-2.101c.335-.94.526-1.947.526-3 0-4.962-4.037-9-9-9-1.706 0-3.296.484-4.654 1.314l1.857 2.686h-6.994l2.152-7 1.85 2.673c1.683-1.049 3.658-1.673 5.789-1.673 6.074 0 11 4.925 11 11zm-6.354 7.692c-1.357.826-2.944 1.308-4.646 1.308-4.963 0-9-4.038-9-9 0-1.053.191-2.06.525-3h-2.1c-.271.955-.425 1.958-.425 3 0 6.075 4.925 11 11 11 2.127 0 4.099-.621 5.78-1.667l1.853 2.667 2.152-6.989h-6.994l1.855 2.681zm-3.646-7.692v-6h-2v8h7v-2h-5z'/></svg></span>");
              CP("<span class='btn-text'>Enable NTP</span>");
            }
              CP("</a>");

            // 21 Set Date
            CP("<form name='setdate' action='21' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='setdate.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M24 2v22h-24v-22h3v1c0 1.103.897 2 2 2s2-.897 2-2v-1h10v1c0 1.103.897 2 2 2s2-.897 2-2v-1h3zm-2 6h-20v14h20v-14zm-2-7c0-.552-.447-1-1-1s-1 .448-1 1v2c0 .552.447 1 1 1s1-.448 1-1v-2zm-14 2c0 .552-.447 1-1 1s-1-.448-1-1v-2c0-.552.447-1 1-1s1 .448 1 1v2zm6.687 13.482c0-.802-.418-1.429-1.109-1.695.528-.264.836-.807.836-1.503 0-1.346-1.312-2.149-2.581-2.149-1.477 0-2.591.925-2.659 2.763h1.645c-.014-.761.271-1.315 1.025-1.315.449 0 .933.272.933.869 0 .754-.816.862-1.567.797v1.28c1.067 0 1.704.067 1.704.985 0 .724-.548 1.048-1.091 1.048-.822 0-1.159-.614-1.188-1.452h-1.634c-.032 1.892 1.114 2.89 2.842 2.89 1.543 0 2.844-.943 2.844-2.518zm4.313 2.518v-7.718h-1.392c-.173 1.154-.995 1.491-2.171 1.459v1.346h1.852v4.913h1.711z'/></svg></span>");
            CP("<span class='btn-slide'>");
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
            CP("<span class='btn-text' onclick='setdate.submit()'>Set Date</span>");
            CP("</a></form>");

            // 19 Set UTC Offset
            CP("<form name='utcoffset' action='19' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='utcoffset.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M11 6v8h7v-2h-5v-6h-2zm10.854 7.683l1.998.159c-.132.854-.351 1.676-.652 2.46l-1.8-.905c.2-.551.353-1.123.454-1.714zm-2.548 7.826l-1.413-1.443c-.486.356-1.006.668-1.555.933l.669 1.899c.821-.377 1.591-.844 2.299-1.389zm1.226-4.309c-.335.546-.719 1.057-1.149 1.528l1.404 1.433c.583-.627 1.099-1.316 1.539-2.058l-1.794-.903zm-20.532-5.2c0 6.627 5.375 12 12.004 12 1.081 0 2.124-.156 3.12-.424l-.665-1.894c-.787.2-1.607.318-2.455.318-5.516 0-10.003-4.486-10.003-10s4.487-10 10.003-10c2.235 0 4.293.744 5.959 1.989l-2.05 2.049 7.015 1.354-1.355-7.013-2.184 2.183c-2.036-1.598-4.595-2.562-7.385-2.562-6.629 0-12.004 5.373-12.004 12zm23.773-2.359h-2.076c.163.661.261 1.344.288 2.047l2.015.161c-.01-.755-.085-1.494-.227-2.208z'/></svg></span>");
            CP("<span class='btn-slide'><input type='range' id='v' name='v' value='" + String(_settings->flashUTCOffset) + "' min='-12' max='12' oninput='this.nextElementSibling.value = this.value' class='slide'><output>" + String(_settings->flashUTCOffset) + "</output></span>");
            CP("<span class='btn-text' onclick='utcoffset.submit()'>Set UTC Offset</span>");
            CP("</a></form>");

            CP("</div></div></div></div>");

            // Power menu [17]
            CP("<div class='divTable'>");
            CP("<div class='divTableBody'>");
            if (setting == 17) {
              CP("<div class='divTableRow' id='M5' style='display:block'>");
            } else {
              CP("<div class='divTableRow' id='M5' style='display:none'>");
            }
            CP("<div class='divTableCell'>");

            // 17 Toggle between 5V USB power and DC 12V
            if (_settings->flashUSB) {
              CP("<a href=\"/17\" class='p'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 20 20' fill='currentColor'><path d='M0 14v1.498c0 .277.225.502.502.502h.997A.502.502 0 0 0 2 15.498V14c0-.959.801-2.273 2-2.779V9.116C1.684 9.652 0 11.97 0 14zm12.065-9.299l-2.53 1.898c-.347.26-.769.401-1.203.401H6.005C5.45 7 5 7.45 5 8.005v3.991C5 12.55 5.45 13 6.005 13h2.327c.434 0 .856.141 1.203.401l2.531 1.898a3.502 3.502 0 0 0 2.102.701H16V4h-1.832c-.758 0-1.496.246-2.103.701zM17 6v2h3V6h-3zm0 8h3v-2h-3v2z'/></svg></span>");
              CP("<span class='btn-text'>Run on 12V DC Power</span>");
            } else {
              CP("<a href=\"/17\" class='r'>");
              CP("<span class='btn-icon'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M18.67,8.67H18V3a1,1,0,0,0-1-1H7A1,1,0,0,0,6,3V8.67H5.33a1,1,0,0,0-1,1v6.54A2.8,2.8,0,0,0,7.13,19H9v2a1,1,0,0,0,1,1h4a1,1,0,0,0,1-1V19h1.88a2.8,2.8,0,0,0,2.79-2.79V9.67A1,1,0,0,0,18.67,8.67ZM8,4h8V8.67H8Zm5,16H11V19h2Zm4.67-3.79a.8.8,0,0,1-.79.79H7.13a.8.8,0,0,1-.8-.79V10.67H17.67Z'/><rect width='1' height='1' x='10' y='6'/><rect width='1' height='1' x='13' y='6'/></svg></span>");
              CP("<span class='btn-text'>Run on 5V USB Power</span>");
            }
              CP("</a>");

            CP("</div></div></div></div>");

            // UI menu [24, 25 & 30]
            CP("<div class='divTable'>");
            CP("<div class='divTableBody'>");
            if (setting == 24 || setting == 25 || setting == 30) {
              CP("<div class='divTableRow' id='M6' style='display:block'>");
            } else {
              CP("<div class='divTableRow' id='M6' style='display:none'>");
            }
            CP("<div class='divTableCell'>");

            // 24 Change WebUI Font
            CP("<form name='changefont' action='24' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='changefont.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M24 20v1h-4v-1h.835c.258 0 .405-.178.321-.422l-.473-1.371h-2.231l-.575-1.59h2.295l-1.362-4.077-1.154 3.451-.879-2.498.921-2.493h2.222l3.033 8.516c.111.315.244.484.578.484h.469zm-6-1h1v2h-7v-2h.532c.459 0 .782-.453.633-.887l-.816-2.113h-6.232l-.815 2.113c-.149.434.174.887.633.887h1.065v2h-7v-2h.43c.593 0 1.123-.375 1.32-.935l5.507-15.065h3.952l5.507 15.065c.197.56.69.935 1.284.935zm-10.886-6h4.238l-2.259-6.199-1.979 6.199z'/></svg></span>");
            CP("<span class='btn-slide'>");
            CP("<select class='selectwide' name='v' id='v'>");
            for(byte i = 1; i < 9; ++i) {
              if (i == _settings->flashFont) {
                selected = "selected";
              } else {
                selected = "";
              }
              switch (i) {
                case 1:
                  CP("<option value='1' class='audiowide' " + String(selected) + ">Audiowide</option>");
                  break;
                case 2:
                  CP("<option value='2' class='codystar' " + String(selected) + ">Codystar</option>");
                  break;
                case 3:
                  CP("<option value='3' class='gugi' " + String(selected) + ">Gugi</option>");
                  break;
                case 4:
                  CP("<option value='4' class='michroma' " + String(selected) + ">Michroma</option>");
                  break;
                case 5:
                  CP("<option value='5' class='novaround' " + String(selected) + ">Nova Round</option>");
                  break;
                case 6:
                  CP("<option value='6' class='orbitron' " + String(selected) + ">Orbitron</option>");
                  break;
                case 7:
                  CP("<option value='7' class='revalia' " + String(selected) + ">Revalia</option>");
                  break;
                case 8:
                  CP("<option value='8' class='slackey' " + String(selected) + ">Slackey</option>");
                  break;
              }
            }
            CP("</select>");
            CP("</span>");
            CP("<span class='btn-text' onclick='changefont.submit()'>Change Font</span>");
            CP("</a></form>");

            // 30 Change WebUI Title
            CP("<form name='changename' action='30' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='changename.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M4 22h-4v-4h4v4zm0-12h-4v4h4v-4zm0-8h-4v4h4v-4zm3 0v4h17v-4h-17zm0 12h17v-4h-17v4zm0 8h17v-4h-17v4z'/></svg></span>");
            CP("<span class='btn-slide'>");
            CP("<input type='text' id='titl' name='titl' size='30' maxlength='40' value='" + String(_settings->webTitle) + "'>");
            CP("<input type='text' id='name' name='name' size='30' maxlength='25' value='" + String(_settings->webName) + "'></span>");
            CP("<span class='btn-text' onclick='changename.submit()'>Change Name</span>");
            CP("</a></form>");

            CP("</div><div class='divTableCell'>");

            // 25 Change WebUI Background
            CP("<form name='changebg' action='25' method='get'><a class='o'>");
            CP("<span class='btn-icon' onclick='changebg.submit()'><svg width='66' height='66' viewBox='0 0 24 24' fill='currentColor'><path d='M9 12c0-.552.448-1 1.001-1s.999.448.999 1-.446 1-.999 1-1.001-.448-1.001-1zm6.2 0l-1.7 2.6-1.3-1.6-3.2 4h10l-3.8-5zm8.8-5v14h-20v-3h-4v-15h21v4h3zm-20 9v-9h15v-2h-17v11h2zm18-7h-16v10h16v-10z'/></svg></span>");
            CP("<span class='btn-slide'>");
            CP("<select class='selectwide' name='v' id='v'>");
            for(byte i = 1; i < 6; ++i) {
              if (i == _settings->flashBackground) {
                selected = "selected";
              } else {
                selected = "";
              }
              switch (i) {
                case 1:
                  CP("<option value='1' " + String(selected) + ">Cage</option>");
                  break;
                case 2:
                  CP("<option value='2' " + String(selected) + ">Dominoes</option>");
                  break;
                case 3:
                  CP("<option value='3' " + String(selected) + ">Grid</option>");
                  break;
                case 4:
                  CP("<option value='4' " + String(selected) + ">Hexagon</option>");
                  break;
                case 5:
                  CP("<option value='5' " + String(selected) + ">Squares</option>");
                  break;
              }
            }
            CP("</select>");
            CP("</span>");
            CP("<span class='btn-text' onclick='changebg.submit()'>Change Background</span>");
            CP("</a></form>");

            CP("</div></div></div></div>");

            CP("<a class='s'><span class='btn-icon-footer' onclick=window.location.href='https://create.arduino.cc/projecthub/Ratti3'><svg width='40' height='40' viewBox='0 0 32 32' fill='currentColor'><path d='M7.844 9C4.039 9 1 12.129 1 16s3.04 7 6.844 7c2.761 0 4.98-1.719 6.531-3.313A19.43 19.43 0 0 0 16 17.782a19.43 19.43 0 0 0 1.625 1.907C19.175 21.28 21.395 23 24.156 23 27.961 23 31 19.871 31 16s-3.04-7-6.844-7c-2.761 0-4.98 1.719-6.531 3.313A19.43 19.43 0 0 0 16 14.219a17.121 17.121 0 0 0-1.563-1.906C12.899 10.714 10.652 9 7.845 9zm0 2c1.972 0 3.808 1.285 5.156 2.688 1.11 1.152 1.523 1.94 1.75 2.312-.23.371-.688 1.156-1.813 2.313C11.57 19.718 9.75 21 7.845 21 5.117 21 3 18.852 3 16s2.117-5 4.844-5zm16.312 0C26.883 11 29 13.148 29 16s-2.117 5-4.844 5c-1.906 0-3.726-1.281-5.093-2.688-1.125-1.156-1.583-1.94-1.813-2.312.23-.371.688-1.156 1.813-2.313C20.43 12.281 22.25 11 24.155 11zM23 13v2h-2v2h2v2h2v-2h2v-2h-2v-2zM5 15v2h6v-2z'/></svg></span>");
            CP("<span class='btn-text-footer' onclick=window.location.href='https://create.arduino.cc/projecthub/Ratti3'>Arduino</span></a>");
            CP("<a class='s'><span class='btn-icon-footer' onclick=window.location.href='https://github.com/ratti3'><svg width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z'/></svg></span>");
            CP("<span class='btn-text-footer' onclick=window.location.href='https://github.com/ratti3'>GitHub</span></a>");
            CP("<a class='s'><span class='btn-icon-footer' onclick=window.location.href='https://easyeda.com/ratti3'><svg width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M5.829 6c-.412 1.165-1.524 2-2.829 2-1.656 0-3-1.344-3-3s1.344-3 3-3c1.305 0 2.417.835 2.829 2h13.671c2.484 0 4.5 2.016 4.5 4.5s-2.016 4.5-4.5 4.5h-4.671c-.412 1.165-1.524 2-2.829 2-1.305 0-2.417-.835-2.829-2h-4.671c-1.38 0-2.5 1.12-2.5 2.5s1.12 2.5 2.5 2.5h13.671c.412-1.165 1.524-2 2.829-2 1.656 0 3 1.344 3 3s-1.344 3-3 3c-1.305 0-2.417-.835-2.829-2h-13.671c-2.484 0-4.5-2.016-4.5-4.5s2.016-4.5 4.5-4.5h4.671c.412-1.165 1.524-2 2.829-2 1.305 0 2.417.835 2.829 2h4.671c1.38 0 2.5-1.12 2.5-2.5s-1.12-2.5-2.5-2.5h-13.671zm6.171 5c.552 0 1 .448 1 1s-.448 1-1 1-1-.448-1-1 .448-1 1-1z'/></svg></span>");
            CP("<span class='btn-text-footer' onclick=window.location.href='https://easyeda.com/ratti3'>EasyEDA</span></a>");
            CP("<a class='s'><span class='btn-icon-footer' onclick=window.location.href='https://youtube.com/ratti3'><svg width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M12 0c-6.627 0-12 5.373-12 12s5.373 12 12 12 12-5.373 12-12-5.373-12-12-12zm4.441 16.892c-2.102.144-6.784.144-8.883 0-2.276-.156-2.541-1.27-2.558-4.892.017-3.629.285-4.736 2.558-4.892 2.099-.144 6.782-.144 8.883 0 2.277.156 2.541 1.27 2.559 4.892-.018 3.629-.285 4.736-2.559 4.892zm-6.441-7.234l4.917 2.338-4.917 2.346v-4.684z'/></svg></span>");
            CP("<span class='btn-text-footer' onclick=window.location.href='https://youtube.com/ratti3'>YouTube</span></a>");
            CP("<a class='s'><span class='btn-icon-footer' onclick=window.location.href='https://ratti3.blogspot.com'><svg width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M15.5 14.625c0 .484-.387.875-.864.875h-5.273c-.477 0-.863-.392-.863-.875s.387-.875.863-.875h5.272c.478 0 .865.391.865.875zm-6.191-4.375h2.466c.448 0 .809-.392.809-.875s-.361-.875-.81-.875h-2.465c-.447 0-.809.392-.809.875s.362.875.809.875zm14.691 1.75c0 6.627-5.373 12-12 12s-12-5.373-12-12 5.373-12 12-12 12 5.373 12 12zm-5-1.039c0-.383-.311-.692-.691-.692h-1.138c-.583 0-.69-.446-.69-.996-.001-2.36-1.91-4.273-4.265-4.273h-2.952c-2.355 0-4.264 1.913-4.264 4.272v5.455c0 2.36 1.909 4.273 4.264 4.273h5.474c2.353 0 4.262-1.913 4.262-4.272v-3.767z'/></svg></span>");
            CP("<span class='btn-text-footer' onclick=window.location.href='https://ratti3.blogspot.com'>Blogger</span></a>");
            CP("<a class='s'><span class='btn-icon-footer' onclick=window.location.href='https://hackaday.io/Ratti3'><svg width='40' height='40' viewBox='0 0 24 24' fill='currentColor'><path d='M12 3.385c-4.681 0-8.475 3.794-8.475 8.475 0 3.229 1.818 6.019 4.475 7.449v2.582c.672.906 2 1.109 2 1.109v-2h1v1.999h2v-1.999h1v2s1.328-.203 2-1.109v-2.582c2.657-1.431 4.475-4.22 4.475-7.449 0-4.681-3.794-8.475-8.475-8.475zm-4 11.615c-1.381 0-2.5-1.119-2.5-2.5 0-1.38 1.12-2.5 2.5-2.5s2.5 1.12 2.5 2.5c0 1.381-1.119 2.5-2.5 2.5zm2 2l2-2.541 2 2.541h-4zm6-2c-1.38 0-2.5-1.119-2.5-2.5 0-1.38 1.12-2.5 2.5-2.5 1.381 0 2.5 1.12 2.5 2.5 0 1.381-1.119 2.5-2.5 2.5zm4.54 2.904l3.46 3.46-2.636 2.636-3.48-3.48c1.035-.705 1.931-1.595 2.656-2.616zm-17.2-11.928l-3.34-3.339 2.636-2.637 3.32 3.32c-1.021.724-1.911 1.621-2.616 2.656zm14.704-2.656l3.32-3.32 2.636 2.637-3.34 3.339c-.705-1.035-1.595-1.932-2.616-2.656zm-11.928 17.2l-3.48 3.48-2.636-2.636 3.46-3.46c.725 1.021 1.621 1.911 2.656 2.616z'/></svg></span>");
            CP("<span class='btn-text-footer' onclick=window.location.href='https://hackaday.io/Ratti3'>Hackaday</span></a>");
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
        if (currentLine.startsWith("GET /10")) {             // Slot machine
          _settings->flashSpin = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 10;
        } else if (currentLine.startsWith("GET /11")) {      // Run BME280
          setting = 11;
        } else if (currentLine.startsWith("GET /12")) {      // NTP Sync
          setting = 12;
        } else if (currentLine.startsWith("GET /13")) {      // Enable/Disable NTP
          if (i < 1) {
            _settings->flashNTP = !_settings->flashNTP;
            i++;
          }
          setting = 13;
        } else if (currentLine.startsWith("GET /14")) {      // Enable/Disable Display
          if (i < 1) {
            if (_hv->isOn()) {
              _hv->switchOff();
            } else {
              _hv->switchOn();
            }
            i++;
          }
          setting = 14;
        } else if (currentLine.startsWith("GET /15")) {      // Enable/Disable PIR
          if (i < 1) {
            _settings->flashPIR = !_settings->flashPIR;
            i++;
          }
          setting = 15;
        } else if (currentLine.startsWith("GET /16")) {      // Enable Disable Light Sensor
          if (i < 1) {
            _settings->flashLight = !_settings->flashLight;
            i++;
          }
          setting = 16;
        } else if (currentLine.startsWith("GET /17")) {      // Switch between 5V and 12V
          if (i < 1) {
            if (_settings->flashUSB) {
              _hv->switch12VOn();
              _settings->flashUSB = 0;
            } else {
              _hv->switch12VOff();
              _settings->flashUSB = 1;
            }
            i++;
          }
          setting = 17;
        } else if (currentLine.startsWith("GET /18")) {      // Set brightness
          _settings->flashBrightness = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 18;
        } else if (currentLine.startsWith("GET /19")) {      // UTC Offset
          _settings->flashUTCOffset = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 19;
        } else if (currentLine.startsWith("GET /20")) {      // Set time
          _settings->hour = currentLine.substring(currentLine.indexOf("h=") + 2, currentLine.indexOf("m=") - 1).toInt();
          _settings->minute = currentLine.substring(currentLine.indexOf("m=") + 2, currentLine.indexOf("s=") - 1).toInt();
          _settings->second = currentLine.substring(currentLine.indexOf("s=") + 2, currentLine.lastIndexOf(' ')).toInt();
          setting = 20;
        } else if (currentLine.startsWith("GET /21")) {      // Set date
          _settings->day = currentLine.substring(currentLine.indexOf("d=") + 2, currentLine.indexOf("m=") - 1).toInt();
          _settings->month = currentLine.substring(currentLine.indexOf("m=") + 2, currentLine.indexOf("y=") - 1).toInt();
          _settings->year = currentLine.substring(currentLine.indexOf("y=") + 2, currentLine.lastIndexOf(' ')).toInt();
          setting = 21;
        } else if (currentLine.startsWith("GET /22")) {      // Set on/off hour
          _settings->flashOnHour = currentLine.substring(currentLine.indexOf("o=") + 2, currentLine.indexOf("x=") - 1).toInt();
          _settings->flashOffHour = currentLine.substring(currentLine.indexOf("x=") + 2, currentLine.lastIndexOf(' ')).toInt();
          setting = 22;
        } else if (currentLine.startsWith("GET /23")) {      // Set NTP pool
          _settings->flashNTPPool = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 23;
        } else if (currentLine.startsWith("GET /24")) {      // Change WebUI Font
          _settings->flashFont = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 24;
        } else if (currentLine.startsWith("GET /25")) {      // Change WebUI Backgound
          _settings->flashBackground = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 25;
        } else if (currentLine.startsWith("GET /26")) {      // Change WiFi
          _settings->flash_SSID = urlDecode(currentLine.substring(13, currentLine.indexOf('&'))); 
          _settings->flash_PASS = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')));
          setting = 26;
        } else if (currentLine.startsWith("GET /27")) {      // Change Colon LED Brightness
          _settings->flashColon = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 27;
        } else if (currentLine.startsWith("GET /28")) {      // Change Switch LED Brightness
          _settings->flashLED1 = currentLine.substring(currentLine.indexOf("r=") + 2, currentLine.indexOf("g=") - 1).toInt();
          _settings->flashLED2 = currentLine.substring(currentLine.indexOf("g=") + 2, currentLine.indexOf("b=") - 1).toInt();
          _settings->flashLED3 = currentLine.substring(currentLine.indexOf("b=") + 2, currentLine.lastIndexOf(' ')).toInt();
          setting = 28;
        } else if (currentLine.startsWith("GET /29")) {      // Set the Low Lux Level
          _settings->flashLux = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')).toInt();
          setting = 29;
        } else if (currentLine.startsWith("GET /30")) {      // Change the WebUI Title
          _settings->flashTitle = urlDecode(currentLine.substring(13, currentLine.indexOf('&'))); 
          _settings->flashName = urlDecode(currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')));
          setting = 30;
        }
      }
    }
    client.stop();

    // Run functions if a WebUI button was pressed
    switch (setting) {
      case 10: // Save number of hourly spins and run slot machine
        _settings->rwSettings(10, 1);
        _nixie->runSlotMachine(_settings->flashSpin);
        break;
      case 11: // Run BME280
        _i2c->displayTHP();
        break;
      case 12: // NTP Sync
        getNTP();
        break;
      case 13: // Enable/Disable NTP
        _settings->rwSettings(13, 1);
        break;
      case 14: // Enable/Disable Display
        // No save action required
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
        _settings->rwSettings(18, 1);
        _fade->setBrightness();
        break;
      case 19: // UTC Offset
        _timeclient->setTimeOffset(_settings->flashUTCOffset * 3600);
        _settings->rwSettings(19, 1);
        getNTP();
        break;
      case 20: // Set time
        _i2c->adjustDateTime(0);
        break;
      case 21: // Set date
        _i2c->adjustDateTime(1);
        break;
      case 22: // Set on/off hour
        _settings->rwSettings(22, 1);
        break;
      case 23: // Set NTP Pool
        _settings->rwSettings(23, 1);
        break;
      case 24: // Change WebUI Font
        _settings->rwSettings(24, 1);
        break;
      case 25: // Change WebUI Backgound
        _settings->rwSettings(25, 1);
        break;
      case 26: // Change WiFi
        _settings->rwSettings(26, 1);
        SPL("[DEBUG] Got the new WiFi credentials, saving to flash and restarting WiFi");
        // Stop the current AP SSID
        WiFi.disconnect();
        connectWiFi();
        _settings->debug(13);
        getNTP();
        _settings->debug(14);
        displayIP();
        break;
      case 27: // Change Colon LED Brightness
        _settings->rwSettings(27, 1);
        break;
      case 28: // Change Switch LED Brightness
        _settings->rwSettings(28, 1);
        _fade->setSwitchLEDBrightness();
        break;
      case 29: // Set the Low Lux Level
        _settings->rwSettings(29, 1);
        break;
      case 30: // Change the WebUI Title
      CPL(_settings->flashName);
        _settings->rwSettings(30, 1);
        break;
    }
  }
}

// Start the access point
void WiFiTask::startAP() {
  SP("Creating access point named: ");
  SPL(ssid_ap);

  // Create network
  int status = WiFi.beginAP(ssid_ap, pass_ap);
  if (status != WL_AP_LISTENING) {
    SPL("Creating access point failed");
  } else {
    _settings->debug(12);
    serverBegin();
    _settings->debug(14);
    printWifiStatus();
    _settings->debug(14);
    displayIP(); 
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
