#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

#include "mobidot/mobidot.hpp"
#include "Fonts/prstartk4pt7b.h"
#include "Fonts/Picopixel.h"
#include "Fonts/SilomBol8pt7b.h"

MobiDOT MobiDOT(/* rx */ 5, /* tx */ 0, /* ctrl */ 4, /* light */ D7);

// Compile time
const char compile_date[] = __DATE__ " " __TIME__;
#define DEBUG_BAUDRATE 115200

// Wifi init
ESP8266WiFiMulti WiFiMulti;

// SERIAL RECEIVE VARS
#define LF 0x0A

char angle_str[10];
int idx = 0;

void setup()
{
  // Serial setup
  Serial.begin(DEBUG_BAUDRATE);
  delay(10);

  // Print firmware version
  Serial.print("\n\nMobitec version: ");
  Serial.println(compile_date);

  // Wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFiMulti.addAP("Langeboomgaard", "ACvI4152EK");

  Serial.print("\nWifi connecting");
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Display setup
  MobiDOT.selectDisplay(MobiDOT::Display::FRONT);
  MobiDOT.toggleLight();

  MobiDOT.drawLine(5, 0, 36, 5);
  MobiDOT.drawLine(16, 15, 36, 5);
  MobiDOT.drawLine(5, 0, 15, 15);

  MobiDOT.print("Progress:", &prstartk4pt7b, 40, 1, false);
  MobiDOT.drawRect(50, 6, 40, 9, false);
  MobiDOT.drawRect(30, 6, 40, 9, true);
  MobiDOT.print("60 %", &Picopixel, 92, 9, false);

  MobiDOT.update();
}

void loop()
{
}