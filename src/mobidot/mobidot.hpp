/**
 * @file mobidot.hpp
 * Main header file for MobiDOT display library
 * 
 * Arduino library for communicating with the Mobitec MobiDOT flipdot displays based on the RS485 protocol. 
 * 
 * Copyright (c) 2021 Arne van Iterson
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

/* Library constants */
#define MOBIDOT_DEBUG true

/* RS485 constants */
#define RS485_TX_PIN_VALUE HIGH
#define RS485_RX_PIN_VALUE LOW
#define RS485_BAUDRATE 4800

/* Protocol constants */
#define MOBIDOT_BYTE_START 0xff
#define MOBIDOT_BYTE_STOP 0xff
#define MOBIDOT_MODE_ASCII 0xa2

/* Front sign constants */
#define MOBIDOT_ADDRESS_FRONT 0x06
#define MOBIDOT_WIDTH_FRONT
#define MOBIDOT_HEIGHT_FRONT

/* Rear sign constants */
#define MOBIDOT_ADDRESS_REAR 0x08
#define MOBIDOT_WIDTH_REAR
#define MOBIDOT_HEIGHT_REAR

/* Side sign constants */
#define MOBIDOT_ADDRESS_SIDE 0x07
#define MOBIDOT_WIDTH_SIDE
#define MOBIDOT_HEIGHT_SIDE

/**
 * @class MobiDOT class
 */
class MobiDOT
{
private:
    SoftwareSerial RS485;
    uint8_t PIN_CTRL;
    uint8_t PIN_LIGHT = 255;

public:
    /**
     * MobiDOT class constructor
     * @param RX RX(RO) pin of RS485 module / ic
     * @param TX TX(DI) pin of RS485 module / ic
     * @param CTRL DE and RE pins of the RS485 module / ic
     * @param light Pin connected to the control pin of a relais to controls the led's of the display
     */
    MobiDOT(const uint8_t RX, const uint8_t TX, const uint8_t CTRL, const uint8_t light = 255);

    /**
     * MobiDOT class deconstructor
     */
    ~MobiDOT();

    enum class Display {
        MOBIDOT_FRONT = 0x01,
        MOBIDOT_REAR = 0x02,
        MOBIDOT_SIDE = 0x03
    }


};