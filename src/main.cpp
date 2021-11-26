#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

#include "mobidot/mobidot.hpp"
#include "Fonts/prstartk4pt7b.h"
#include "Fonts/Picopixel.h"
#include "Fonts/SilomBol8pt7b.h"

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

  // MobiDOT.print("Hi", &prstartk4pt7b, 2, 1, true);
  MobiDOT.drawRect(12, 8, 0, 0, false);
  MobiDOT.drawRect(2, 2, 2, 2, true);

  // const unsigned char xs[] = {
  //     0x92, 0x40, 0xc7, 0x00, 0x92, 0x40, 0x38, 0xc0, 0x92, 0x40, 0xc7, 0x00, 0x92, 0x40, 0x38, 0xc0,
  //     0x92, 0x40, 0xc7, 0x00};

  // MobiDOT.drawBitmap(xs, 10, 10, 0, 2);

  MobiDOT.print("there!", &Picopixel, 1, 9, false);
  MobiDOT.update();

  // MobiDOT.update();

  // const unsigned char smile[] = {
  //     0xff,
  //     0xdb,
  //     0xdb,
  //     0xff,
  //     0xff,
  //     0xbd,
  //     0xc3,
  //     0xff,
  // };
}

void loop()
{
}