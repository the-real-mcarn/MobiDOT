#include <Arduino.h>

#include "mobidot/mobidot.hpp"
#include "Fonts/FreeSerifItalic9pt7b.h"

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

    // Display setup
    MobiDOT.selectDisplay(MobiDOT::Display::REAR);

    MobiDOT.print("Yea", &FreeSerifItalic9pt7b, 0, 0);
    MobiDOT.update();
}

void loop()
{
}