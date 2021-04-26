WiFiServer server(80);

// Web server for interacting with the Nano 33 IoT
void clientServer() {

  // Listen for incoming client connections
  WiFiClient client = server.available();   

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
            client.print("<div style='text-align:center'>");
            client.print("<span style='font-family:verdana; font-size:40px; font-weight:bold'>");
            client.print("<h2 style='text-align:center'>Arduino Nano 33 IoT Nixie Clock : Living Room</h2>");
//            if (displayOff) {
              client.print("<a href=\"/02\"><img src='https://drive.google.com/uc?id=1dGVztV0ahnLaB_RNltlMo3H0TSPJ-ATU' style='padding:10px' /></a>"); // Display On
//            } else {
              client.print("<a href=\"/01\"><img src='https://drive.google.com/uc?id=1xbmeCXErmswAt3xh42ooL4HaqfX4cYVQ' style='padding:10px' /></a>"); // Display Off
//            }
            client.print("<a href=\"/03\"><img src='https://drive.google.com/uc?id=1qGAOFwhd63jFJXEMi4QyQXEUwAsPzdcu' style='padding:10px' /></a><br>");   // NTP
            client.print("<a href=\"/04\"><img src='https://drive.google.com/uc?id=1HbvlF_ACW4zPGfcpgxSn2TPsJy-SI8cF' style='padding:10px' /></a>");   // Pattern
            client.print("<a href=\"/05\"><img src='https://drive.google.com/uc?id=1F4nhD4GAediuS67L6H_8wDDK95n5MvWQ' style='padding:10px' /></a><br>");   // Temp
            client.print("<a href=\"/06\"><img src='https://drive.google.com/uc?id=1CFa38_ViMUeKnZFmLsKrr1aPov9G-0_t' style='padding:10px' /></a><br>");   // Date
            client.print("");
            client.print("<a href='https://www.youtube.com/Ratti3'><img src='https://drive.google.com/uc?id=1Tv8hR-kJAJrHA-2Ls3I5tKF5iBxXHeG6' style='padding:10px' /></a> ");
            client.print("<a href='https://ratti3.blogspot.com'><img src='https://drive.google.com/uc?id=1rV_gw0rkG-NPXOBOwKx_3fO1hXbctU80' style='padding:10px' /></a> ");
            client.print("<a href='https://easyeda.com/Ratti3'><img src='https://drive.google.com/uc?id=1DCPnmIjOvhmMe5kVEDUbUmj8hHiEav33' style='padding:10px' /></a>");
            client.print("<br>&copy; 2021 : Ratti3 Technologies Corp</span></div>");
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
            digitalWrite(PIN_HV_LE, HIGH);
        } else if (currentLine.endsWith("GET /02")) {
//          displayOff = 0;
            digitalWrite(PIN_HV_LE, LOW);
        } else if (currentLine.endsWith("GET /03")) {
//            getNTP();
        } else if (currentLine.endsWith("GET /04")) {
 //         spin(random(patterns));
            nixie.runSlotMachine();
        } else if (currentLine.endsWith("GET /05")) {
            i2c.displayTHP();
        } else if (currentLine.endsWith("GET /06")) {
            timeTask.showDate();
        }   
      }
    }
    client.stop();
  }
  
} //// End void clientServer()

void ServerBegin() {
  server.begin();
}
