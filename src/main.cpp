#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

#include "mobidot/mobidot.hpp"
#include "Fonts/FreeMono12pt7b.h"

MobiDOT MobiDOT(/* rx */ 5, /* tx */ 0, /* ctrl */ 4, /* light */ 3);

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
  MobiDOT.selectDisplay(MobiDOT::Display::REAR);

  MobiDOT.print("Test", &FreeMono12pt7b, 0, 0);

  const char bitmap[] = {
      0xff,
      0xdb,
      0xdb,
      0xff,
      0xff,
      0xbd,
      0xc3,
      0xff,
  };

  // Normal
  MobiDOT.drawBitmap(bitmap, 8, 8, 0, 0, false);
  MobiDOT.update();

  delay(3000);

  MobiDOT.clear();
  MobiDOT.update();
}

void loop()
{
  if (Serial.available() > 0)
  {
    angle_str[idx] = Serial.read();
    if (angle_str[idx] == LF)
    {
      Serial.print("Printing: ");
      angle_str[idx - 1] = 0;
      Serial.println(angle_str);
      MobiDOT.print(angle_str, MobiDOT::Font::TEXT_6PX, 0, 0);
      MobiDOT.update();
      idx = -1;
    }
    idx++;
  }
}