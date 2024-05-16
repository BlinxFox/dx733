/*
    Little project to turn on/off a Optoma DX733 projector via serial port
    Didn't get the original remote when I bought it. Now using a spare
    button on the remote of my X5 Xnano Android TV box

    IR-Receiver <-> ESP32-C3 <-> MAX3232 <-> DX733
*/

int RECV_PIN = 2;
int LED_PIN = 12;
int RX_PIN = 10;
int TX_PIN = 3;

const char *ssid = "...";
const char *password = "...";

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#include <IRremote.h>
#include "beamer.h"
#include "ircodes.h"

Beamer beamer(Serial1);
WebServer server(80);

#include <ArduinoJson.h>
#include <FS.h>   // Include the SPIFFS library
#include "SPIFFS.h"

bool beamer_on = false;

uint32_t reverse(uint32_t value) {
  uint32_t ret = 0;

  for (int x = 0; x < 32; x++) {
    ret = ret << 1;
    ret |= value & 0x01;
    value = value >> 1;
  }
  return ret;
}

void setup() {
  Serial.begin(115200);

  Serial1.setPins(RX_PIN, TX_PIN);
  Serial1.begin(9600);

  pinMode(LED_PIN, OUTPUT);

  IrReceiver.begin(RECV_PIN, false);

  SPIFFS.begin();

  WiFi.mode ( WIFI_STA );

  WiFi.disconnect();
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++)
  {
    Serial.println(WiFi.SSID(i));
  }

  WiFi.begin ( ssid, password );
  Serial.println ( "" );

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });
  server.on("/api", HTTP_POST, handleApiCall);
  server.begin();
  Serial.println("HTTP server started");
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("host: " + server.hostHeader());
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

void handleApiCall() {
  for (int c = 0; c < server.args(); c++) {
    Serial.println(server.argName(c));
  }

  String command = "";
  if (server.hasArg("command")) {
    command = server.arg("command");
    Serial.print("Command: ");
    Serial.println(command);
  }


  if (command == "powerOn") {
    beamer_on = true;
    beamer.powerOn();
  } else if (command == "powerOff") {
    beamer_on = false;
    beamer.powerOff();
  }
  digitalWrite(LED_PIN, beamer_on);

  server.send(200, "application/json", "{\"result\":\"OK\"}");
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}


uint32_t lastbutton = -1;
uint32_t lastseen = 0;
int repeated = 0;

bool  processbutton(uint32_t button, bool repeat) {
  if (!repeat && button == reverse(IR_X5_TV_POWER)) {
    beamer_on = !beamer_on;
    if (beamer_on) {
      beamer.powerOn();
    } else {
      beamer.powerOff();
    }
    digitalWrite(LED_PIN, beamer_on);
  }
  return false;
}

void loop() {
  server.handleClient();
  if (IrReceiver.decode()) {
    uint32_t button = IrReceiver.decodedIRData.decodedRawData;
    Serial.println(button, HEX);
    if (button == IR_REPEAT) {
      if (millis() - 1000 < lastseen) {
        repeated++;
        if (repeated > 5) {
          repeated = 0;
          processbutton(lastbutton, true);
        }
      }
    } else {
      lastbutton = button;
      repeated = 0;
      processbutton(button, false);
    }

    lastseen = millis();
    IrReceiver.resume();
  }
}
