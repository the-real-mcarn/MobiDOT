/**
 * @file mobidot.cpp
 * Main source file for MobiDOT display library
 * 
 * Arduino library for communicating with the Mobitec MobiDOT flipdot displays based on the RS485 protocol. 
 * 
 * Copyright (c) 2021 Arne van Iterson
 */

#include "./mobidot.hpp"

MobiDOT::MobiDOT(const uint8_t rx, const uint8_t tx, const uint8_t ctrl, const uint8_t light)
{
    // Start software serial
    this->RS485.begin(RS485_BAUDRATE, SWSERIAL_8N1, rx, tx);
    delay(10);

    // Init control pin
    this->PIN_CTRL = ctrl;
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
    digitalWrite(this->PIN_CTRL, RS485_RX_PIN_VALUE);
    this->RS485.end();
}

void MobiDOT::checkInit()
{
    // Wait for the MobiDOT display to be initialised properly
    const uint setupTime = millis();
    Serial.println(setupTime);
    if (setupTime < MOBIDOT_INIT_TIME)
    {
        Serial.println("Waiting...");
        delay(MOBIDOT_INIT_TIME - setupTime);
    }
}

void MobiDOT::selectDisplay(MobiDOT::Display type)
{
    this->DISPLAY_DEFAULT = type;
}

void MobiDOT::print(const char c[])
{
    const MobiDOT::Font font = this->display[(uint)this->DISPLAY_DEFAULT].defaultFont;
    this->print(c, font, 0, 0);
}

void MobiDOT::print(const char c[], uint offsetX, uint offsetY)
{
    const MobiDOT::Font font = this->display[(uint)this->DISPLAY_DEFAULT].defaultFont;
    this->print(c, font, offsetX, offsetY);
}

void MobiDOT::print(const char c[], MobiDOT::Font font)
{
    this->print(c, font, 0, 0);
}

void MobiDOT::print(const char c[], MobiDOT::Font font, uint offsetX, uint offsetY)
{
    // Check if the current buffer is empty, if so add the MobiDOT header
    if (this->BUFFER_DATA[0] != 0xff)
    {
        this->addHeader(this->DISPLAY_DEFAULT, this->BUFFER_DATA, this->BUFFER_SIZE);
    }
    
    this->BUFFER_DATA[this->BUFFER_SIZE] = 0xd2;
    this->BUFFER_DATA[this->BUFFER_SIZE + 1] = offsetX;
    this->BUFFER_DATA[this->BUFFER_SIZE + 2] = 0xd3;
    this->BUFFER_DATA[this->BUFFER_SIZE + 3] = offsetY + this->display[(uint)this->DISPLAY_DEFAULT].offsetY;
    this->BUFFER_DATA[this->BUFFER_SIZE + 4] = 0xd4;
    this->BUFFER_DATA[this->BUFFER_SIZE + 5] = (char)font;
    this->BUFFER_SIZE += 6;

    for (size_t i = 0; i < strlen(c); i++)
    {
        this->BUFFER_DATA[this->BUFFER_SIZE] = c[i];
        this->BUFFER_SIZE += 1;
    }
}

bool MobiDOT::update()
{
    // Add display footer and send
    this->addFooter(this->BUFFER_DATA, this->BUFFER_SIZE);
    bool result = this->sendBuffer(this->BUFFER_DATA, this->BUFFER_SIZE);

    // Clear current display buffer
    memset(this->BUFFER_DATA, 0, sizeof(this->BUFFER_DATA));
    this->BUFFER_SIZE = 0;

    return result;
}

void MobiDOT::addHeader(MobiDOT::Display type, char data[], uint &size)
{
    // Serial.println(size);
    data[size] = (char)MOBIDOT_BYTE_START;                    // Start serial transfer
    data[size + 1] = (char)this->display[(uint)type].address; // Set address
    data[size + 2] = (char)MOBIDOT_MODE_ASCII;                // Set ASCII mode
    data[size + 3] = 0xd0;
    data[size + 4] = (char)this->display[(uint)type].width; // Set display width
    data[size + 5] = 0xd1;
    data[size + 6] = (char)this->display[(uint)type].height; // Set display height
    size += 7;
    return;
}

void MobiDOT::addFooter(char data[], uint &size)
{
    uint checksum = 0;
    for (size_t i = 1; i < size; i++)
    {
        checksum += data[i]; // Add up all numbers of input data
    }
    data[size] = checksum & 0xff; // Cut down checksum to one byte

    if (data[size] == 0xfe)
    {
        data[size + 1] = 0x00;
        size += 2;
    }
    else if (data[size] == 0xff)
    {
        data[size] = 0xfe;
        data[size + 1] = 0x01;
        size += 2;
    }
    else
    {
        size += 1;
    }

    data[size] = MOBIDOT_BYTE_STOP;
    data[size + 1] = 0x00;
    size += 2;
    return;
}

bool MobiDOT::sendBuffer(char data[], uint size)
{
    digitalWrite(this->PIN_CTRL, RS485_TX_PIN_VALUE); // Set RS485 module to transmit
    size_t result = this->RS485.write(data, size);    // Write data
    digitalWrite(this->PIN_CTRL, RS485_RX_PIN_VALUE); // Set RS485 module to receive

    // Check if returned amount of bytes is equal to the input
    if (result == size)
    {
        // Something went wrong, not all bytes were sent correctly or the input data is not set up correctly
        return false;
    }
    else
    {
        // Transmitted data successfully
        return true;
    }
}