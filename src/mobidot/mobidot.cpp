/**
 * @file mobidot.cpp
 * Main source file for MobiDOT display library
 * 
 * Arduino library for communicating with the Mobitec MobiDOT flipdot displays based on the RS485 protocol. 
 * 
 * Copyright (c) 2021 Arne van Iterson
 */

#include mobidot.hpp

MobiDOT::MobiDOT(const uint8_t RX, const uint8_t TX, const uint8_t CTRL, const uint8_t light)
{
    // Start software serial
    this->RS485.begin(RS485_BAURDRATE, SWSERIAL_8N1, RX, TX);

    // Init control pin
    this->PIN_CTRL = CTRL;
    pinMode(this->PIN_CTRL, OUTPUT);
    digitalWrite(this->PIN_CTRL, RS485_RX_PIN_VALUE);

    // Init light relais pin
    if (light != 255)
    {
        this->PIN_LIGHT = light;
        pinMode(this->PIN_LIGHT, OUTPUT);
    }
    
}

MobiDOT::~MobiDOT()
{
    this.RS485.end();
}
