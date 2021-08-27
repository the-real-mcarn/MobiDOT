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

    MobiDOT.selectDisplay(MobiDOT::Display::REAR);

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

    delay(5000);

    // Inverted
    MobiDOT.drawBitmap(bitmap, 8, 8, 0, 0, true);
    MobiDOT.update();

    delay(5000);

    // Other location
    MobiDOT.drawBitmap(bitmap, 8, 8, 10, 4, true);
    MobiDOT.update();

    delay(5000);

    // Multiple
    MobiDOT.drawBitmap(bitmap, 8, 8, 0, 0, false);
    MobiDOT.drawBitmap(bitmap, 8, 8, 10, 4, true);
    MobiDOT.update();
}

void loop()
{
}