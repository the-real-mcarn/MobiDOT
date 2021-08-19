#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>

#include "mobidot/mobidot.hpp"

MobiDOT MobiDOT(/* rx */ 5, /* tx */ 0, /* ctrl */ 4, /* light */ 3);

// Compile time
const char compile_date[] = __DATE__ " " __TIME__;
#define DEBUG_BAUDRATE 115200

// Wifi init
ESP8266WiFiMulti WiFiMulti;

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

  // MobiDOT.checkInit();
  MobiDOT.selectDisplay(MobiDOT::Display::FRONT);
  
  MobiDOT.print("mobitec", MobiDOT::Font::TEXT_16PX, 18, 19);
  MobiDOT.print("TM", MobiDOT::Font::TEXT_5PX, 80, 0);
  MobiDOT.update();
}

void loop()
{
}