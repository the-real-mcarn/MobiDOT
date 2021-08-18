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
#define RS485_BUFFER_SIZE 2048

/* Protocol constants */
#define MOBIDOT_BYTE_START 0xff
#define MOBIDOT_BYTE_STOP 0xff
#define MOBIDOT_MODE_ASCII 0xa2
#define MOBIDOT_INIT_TIME 7500

/* Front sign constants */
#define MOBIDOT_ADDRESS_FRONT 0x06
#define MOBIDOT_WIDTH_FRONT 112
#define MOBIDOT_HEIGHT_FRONT 16

/* Rear sign constants */
#define MOBIDOT_ADDRESS_REAR 0x08
#define MOBIDOT_WIDTH_REAR 21
#define MOBIDOT_HEIGHT_REAR 14

/* Side sign constants */
#define MOBIDOT_ADDRESS_SIDE 0x07
#define MOBIDOT_WIDTH_SIDE 84
#define MOBIDOT_HEIGHT_SIDE 7

/**
 * @class MobiDOT class
 */
class MobiDOT
{
public:
    /**
     * @enum Display
     * Contains display types
     */
    enum class Display
    {
        FRONT,
        REAR,
        SIDE
    };

    /**
     * @enum Font
     * Contains all fonts available in ASCII mode
     */
    enum class Font
    {
        TEXT_5PX = 0x72,
        TEXT_6PX = 0x66,
        TEXT_7PX = 0x65,
        TEXT_7PX_BOLD = 0x64,
        TEXT_9PX = 0x75,
        TEXT_9PX_BOLD = 0x70,
        TEXT_9PX_BOLDER = 0x62,
        TEXT_13PX = 0x73,
        TEXT_13PX_BOLD = 0x69,
        TEXT_13PX_BOLDER = 0x61,
        TEXT_13PX_BOLDEST = 0x79,
        NUMBERS_14PX = 0x00,
        TEXT_15PX = 0x71,
        TEXT_16PX = 0x68,
        TEXT_16PX_BOLD = 0x78,
        TEXT_16PX_BOLDER = 0x74,
        SYMBOLS = 0x67,
        BITWISE = 0x77
    };

    /**
     * MobiDOT class constructor
     * Sets up everything to start sending data to the integrated controller of a MobiDOT display
     * @param rx RX(RO) pin of RS485 module / ic
     * @param tx TX(DI) pin of RS485 module / ic
     * @param ctrl DE and RE pins of the RS485 module / ic
     * @param light Pin connected to the control pin of a relais to controls the led's of the display
     */
    MobiDOT(const uint8_t rx, const uint8_t tx, const uint8_t ctrl, const uint8_t light = 255);

    /**
     * MobiDOT class deconstructor
     * End communication with display and closes RS485 serial port
     */
    ~MobiDOT();

    void checkInit();

    void selectDisplay(MobiDOT::Display type);

    /**
     * print function
     * Prints a string to the display using the currently selected font
     * @param c Char array to print
     */
    void print(const char c[]);
    void print(const char c[], uint offsetX, uint offsetY);
    void print(const char c[], MobiDOT::Font font);
    void print(const char c[], MobiDOT::Font font, uint offsetX, uint offsetY);

    bool update();

    /**
     * clear function
     * Clears the display
     */
    bool clear();

private:
    SoftwareSerial RS485;
    uint8_t PIN_CTRL;
    uint8_t PIN_LIGHT = 255;

    MobiDOT::Display DISPLAY_DEFAULT;
    char BUFFER_DATA[RS485_BUFFER_SIZE] = {0};
    uint BUFFER_SIZE = 0;

    bool STATE_LIGHT = false;

    /**
     * @struct DisplayAttribute
     * Contains information about the displays defined in Display
     */
    struct DisplayAttribute
    {
        char address;
        MobiDOT::Font defaultFont;
        uint8_t offsetY;
        uint8_t width;
        uint8_t height;
    };

    /**
     * display array
     * Contains the compiler macros at the top of mobidot.hpp for easy access using Display
     */
    const struct DisplayAttribute display[3] = {
        {MOBIDOT_ADDRESS_FRONT, MobiDOT::Font::TEXT_16PX_BOLD, 19, MOBIDOT_WIDTH_FRONT, MOBIDOT_HEIGHT_FRONT},
        {MOBIDOT_ADDRESS_REAR, MobiDOT::Font::TEXT_13PX_BOLD, 10, MOBIDOT_WIDTH_REAR, MOBIDOT_HEIGHT_REAR},
        {MOBIDOT_ADDRESS_SIDE, MobiDOT::Font::TEXT_7PX_BOLD, 0, MOBIDOT_WIDTH_SIDE, MOBIDOT_HEIGHT_SIDE},
    };

    /**
     * addHeader function
     * Adds MobiDOT header to output data
     * @param type Display type, see Display
     * @param data Input data array
     * @param size Size of input data array
     */
    void addHeader(MobiDOT::Display type, char data[], uint &size);

    /**
     * addFooter function
     * Adds MobiDOT footer to input data
     * @param data Input data array
     * @param size Pointer to the size of input data array
     */
    void addFooter(char data[], uint &size);

    /**
     * sendBuffer function
     * Sends input data to display over the RS485 bus
     * @param data Input data array
     * @param size Size of input data array
     * @returns True if transfer successfull, false if not
     */
    bool sendBuffer(char data[], uint size);
};