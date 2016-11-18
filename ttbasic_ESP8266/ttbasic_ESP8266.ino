/*
  TOYOSHIKI Tiny BASIC for Arduino
  (C)2012 Tetsuya Suzuki
*/
//#include <PCD85448266.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "host.h"


// PCD8544 lcd;

/* Set these to your desired credentials. */
const char *ssid = "ttb";
const char *password = "";

ESP8266WebServer server(80);


void basic(void);

void setup(void) {
  // put your setup code here, to run once:
  Serial.begin(9600);

  randomSeed(analogRead(0));
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
  server.handleClient();

  host_display_init();


}

void loop(void) {
  // put your main code here, to run repeatedly:
  basic();
}

void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}


