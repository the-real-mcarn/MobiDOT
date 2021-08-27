#include <Arduino.h>
#include "mobidot/mobidot.hpp"

MobiDOT MobiDOT(/* rx */ 5, /* tx */ 0, /* ctrl */ 4, /* light */ 3);

// Compile time
const char compile_date[] = __DATE__ " " __TIME__;
#define DEBUG_BAUDRATE 115200

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

  MobiDOT.selectDisplay(MobiDOT::Display::FRONT);

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
      MobiDOT.print(angle_str, MobiDOT::Font::TEXT_13PX_BOLD, 0, 0);
      MobiDOT.update();
      idx = -1;
    }
    idx++;
  }
}