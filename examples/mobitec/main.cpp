#include <Arduino.h>
#include "mobidot/mobidot.hpp"

MobiDOT MobiDOT(/* rx */ 5, /* tx */ 0, /* ctrl */ 4, /* light */ 3);

// Compile time
const char compile_date[] = __DATE__ " " __TIME__;
#define DEBUG_BAUDRATE 115200

void setup()
{
    // Serial setup
    Serial.begin(DEBUG_BAUDRATE);
    delay(10);

    // Print firmware version
    Serial.print("\n\nMobitec version: ");
    Serial.println(compile_date);

    MobiDOT.selectDisplay(MobiDOT::Display::FRONT);

    MobiDOT.print("mobitec", MobiDOT::Font::TEXT_16PX, 18, 19);
    MobiDOT.print("TM", MobiDOT::Font::TEXT_5PX, 80, 0);
    MobiDOT.update();
}

void loop()
{
}