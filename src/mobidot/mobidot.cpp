/**
 * @file mobidot.cpp
 * Main source file for MobiDOT display library
 * 
 * Arduino library for communicating with the Mobitec MobiDOT flipdot displays based on the RS485 protocol. 
 * 
 * Copyright (c) 2021 Arne van Iterson
 */

#include "./mobidot.hpp"

/**
 * MobiDOT class constructors
 */

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
    if (light != -1)
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

/**
 * Public functions
 */

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

    uint *size = &this->BUFFER_SIZE;

    this->BUFFER_DATA[*size] = 0xd2;
    this->BUFFER_DATA[*size + 1] = offsetX;
    this->BUFFER_DATA[*size + 2] = 0xd3;
    this->BUFFER_DATA[*size + 3] = offsetY;
    this->BUFFER_DATA[*size + 4] = 0xd4;
    this->BUFFER_DATA[*size + 5] = (char)font;
    *size += 6;

    for (size_t i = 0; i < strlen(c); i++)
    {
        this->BUFFER_DATA[*size] = c[i];
        *size += 1;
    }
}

void MobiDOT::print(const char c[], const GFXfont *font, uint offsetX, uint offsetY, bool invert)
{
    // This function creates a bitmap that the sendBitmap function will handle, no need to add a buffer at this point

    // First char in the font
    const uint8_t first = pgm_read_word(&font->first);

    // Char ascii index
    uint8_t index;

    // Cursor for x position
    uint16_t cursor = 0;

    // Go through string char by char
    for (size_t i = 0; i < strlen(c); i++)
    {
        // Make sure that the selected char is in the scope of the font, otherwise display a space
        if (c[i] >= first && c[i] <= pgm_read_word(&font->last))
        {
            index = c[i] - first;
        }
        else
        {
            index = 0x20 - first;
        }

        // Create glyph instance
        GFXglyph *g = font->glyph + index;

        // Read information from glyph instance
        uint16_t bitmapOffset = (uint16_t)pgm_read_word(&g->bitmapOffset);

        const uint8_t charW = pgm_read_word(&g->width);
        const uint8_t charH = pgm_read_word(&g->height);
        const uint8_t charXadvance = pgm_read_word(&g->xAdvance);

        const uint16_t charSize = charW * charH;
        const uint8_t charByteSize = ceil(charSize / 8.0);

        Serial.printf("\nChar index: 0x%x\nBitmap offset: %d;\nChar width: %d; Char height: %d; \nChar size: %d; in bytes: %d;\n", index, bitmapOffset, charW, charH, charSize, charByteSize);

        // Create array pointer to store char data in
        uint8_t *charData = new uint8_t[charByteSize];

        // Read char data byte by byte from the font bitmap
        for (uint8_t byte = 0; byte <= charByteSize - 1; byte++)
        {
            // Get value from progmem
            const uint8_t value = pgm_read_word(&font->bitmap[bitmapOffset + byte]);

            // Store data in array
            charData[byte] = value;
            Serial.printf("%X, ", value);
        }
        Serial.println(" ");

        // Now that we have the char data, it is time to make it into a bitmap, which means padding it to full bytes
        // Determine width
        // TODO: Might have to change this to the xAdvance instead because some letters be skinny af
        const uint8_t bufferByteW = ceil(charW / 8.0);
        Serial.printf("\nBitmap width: %d; Buffer size: %d;\n", bufferByteW, bufferByteW * charH);

        // Create buffer and zero it
        uint8_t *buffer = new uint8_t[bufferByteW * charH];
        memset(buffer, 0, bufferByteW * charH);

        // Go through char bit by bit
        // TODO: Find a way to optimise this?
        for (uint8_t b = 0; b < charSize; b++)
        {
            // Get the value of the current bit, if it is zero we can skip it
            uint8_t charByteOffset = floor(b / 8.0); // Start with zero
            uint8_t charBitOffset = b % 8;
            const uint8_t value = charData[charByteOffset] >> (7 - charBitOffset) & 0b00000001;

            // Print char data visually
            Serial.printf("%d ", value);
            if ((b + 1) % charW == 0)
            {
                Serial.println(" ");
            }

            if (value)
            {
                // First determine which byte this bit falls in
                const uint8_t line = floor(b / charW); // Start with zero
                uint8_t bufferByteOffset = line * bufferByteW;

                // Get the bit offset in the current line
                uint8_t bufferBitOffset = b - (line * charW);

                // Bytes are 8 bits, if the bit offset is larger than 8 it should be added to the byte offset
                while (bufferBitOffset >= 8)
                {
                    bufferBitOffset = bufferBitOffset - 8;
                    bufferByteOffset++;
                }

                // Copy value to the bitmap buffer
                uint8_t result = 0x01;
                result = result << (7 - bufferBitOffset);
                buffer[bufferByteOffset] = buffer[bufferByteOffset] | result;
            }
        }

        // Print bitmap buffer visually
        for (size_t test = 0; test < (bufferByteW * 8) * charH; test++)
        {
            uint8_t byteO = floor(test / 8.0); // Start with zero
            uint8_t bitO = test % 8;
            const uint8_t value = buffer[byteO] >> (7 - bitO) & 0b00000001;

            Serial.printf("%d ", value);
            if ((test + 1) % (bufferByteW * 8) == 0)
            {
                Serial.println(" ");
            }
        }

        MobiDOT::drawBitmap(buffer, charW, charH, offsetX + cursor, offsetY, !invert);
        cursor = cursor + charXadvance;
    }
};

bool MobiDOT::update()
{
    uint *size = &this->BUFFER_SIZE;

    // Add display footer and send
    this->addFooter(this->BUFFER_DATA, *size);
    bool result = this->sendBuffer(this->BUFFER_DATA, *size);

    // Clear current display buffer
    memset(this->BUFFER_DATA, 0, sizeof(this->BUFFER_DATA));
    *size = 0;

    return result;
}

void MobiDOT::clear(bool value)
{
    // Check if the current buffer is empty, if so add the MobiDOT header
    if (this->BUFFER_DATA[0] != 0xff)
    {
        this->addHeader(this->DISPLAY_DEFAULT, this->BUFFER_DATA, this->BUFFER_SIZE);
    }

    uint *size = &this->BUFFER_SIZE;
    const uint height = this->display[(uint)this->DISPLAY_DEFAULT].height;
    const uint width = this->display[(uint)this->DISPLAY_DEFAULT].width;

    for (size_t i = 0; i < ceil((float)height / 5); i++)
    {
        this->BUFFER_DATA[*size] = 0xd2;
        this->BUFFER_DATA[*size + 1] = 0;
        this->BUFFER_DATA[*size + 2] = 0xd3;
        this->BUFFER_DATA[*size + 3] = 4 + (i * 5);
        this->BUFFER_DATA[*size + 4] = 0xd4;
        this->BUFFER_DATA[*size + 5] = (char)MobiDOT::Font::BITWISE;
        *size += 6;

        for (size_t j = 0; j < width; j++)
        {
            this->BUFFER_DATA[*size] = (value) ? 0x3f : 0x20;
            *size += 1;
        }
    }
}

void MobiDOT::drawBitmap(const unsigned char data[], uint width, uint height, bool invert)
{
    this->drawBitmap(data, width, height, 0, 0, invert);
}

void MobiDOT::drawBitmap(const unsigned char data[], uint width, uint height, uint x, uint y, bool invert)
{
    // TODO: remove debug stuff
    // Check if the current buffer is empty, if so add the MobiDOT header
    if (this->BUFFER_DATA[0] != 0xff)
    {
        this->addHeader(this->DISPLAY_DEFAULT, this->BUFFER_DATA, this->BUFFER_SIZE);
    }

    uint *size = &this->BUFFER_SIZE;
    const uint16_t bytesOverWidth = ceil(width / 8.0);

    // Divide bitmap in rows of 5 pixels because that is how the font works
    for (int16_t i = 0; i < ceil((float)height / 5); i++)
    {
        Serial.print("\ni: ");
        Serial.println(i);

        // Add bitmap header
        this->BUFFER_DATA[*size] = 0xd2;
        this->BUFFER_DATA[*size + 1] = x;
        this->BUFFER_DATA[*size + 2] = 0xd3;
        this->BUFFER_DATA[*size + 3] = y + 4 + (i * 5);
        this->BUFFER_DATA[*size + 4] = 0xd4;
        this->BUFFER_DATA[*size + 5] = (char)MobiDOT::Font::BITWISE;
        *size += 6;

        // debug
        Serial.print("width: ");
        Serial.println(width);

        Serial.print("bytes: ");
        Serial.println(bytesOverWidth);

        Serial.print("bit total: ");
        Serial.println((bytesOverWidth * 8));

        Serial.print("bit surplus: ");
        Serial.println((bytesOverWidth * 8) - width);
        // end debug

        // Go through bitmap line by line (width)
        // for (int16_t j = (bytesOverWidth * 8 - 1); j >= (int)((bytesOverWidth * 8) - width); j--)
        for (int16_t j = 0; j <= (int)width - 1; j++)
        {
            Serial.print("\nj: ");
            Serial.println(j);

            // Result byte needs to start with 001xxxxx
            char result = 0x01;

            // Go through bitmap data bit by bit backwards because font
            for (int8_t k = 4; k >= 0; k--) // Height
            {
                // Shift byte left
                result = result << 1;

                // Check if the current font line is not out of range of the bitmap, if it is, it's value should be 0
                // e.g. if the bitmap is 12px in height thus not divideable by 5
                if ((i * 5 + k) < (int)height)
                {
                    // If it is in range, check the bitmap for the data
                    // Bitmaps are padded on the right in order to fit each row into bytes, keep this in mind

                    // Determine the height we are at in the bitmap, i includes previous lines drawn, k is for the current line
                    int16_t byteOffset = ((i * 5) * bytesOverWidth) + (k * bytesOverWidth);

                    // Now that we have the height sorted, we have to account for the current position in width j
                    int16_t bitOffset = j;

                    // Bytes are 8 bits, if the bit offset is larger than 8 it should be added to the byte offset
                    while (bitOffset >= 8)
                    {
                        bitOffset = bitOffset - 8;
                        byteOffset++;
                    }

                    // Because we are going to shift these bits in, we need to reverse the order
                    // TODO: One line this once you know it works with fonts
                    bitOffset = 7 - bitOffset;

                    // Get value of current bit
                    const int8_t value = data[byteOffset] >> bitOffset & B00000001;

                    // debug
                    Serial.print(" byte: ");
                    Serial.print(byteOffset);
                    Serial.print("; value:\t");
                    Serial.print(data[byteOffset], BIN);
                    Serial.print("\tbit: ");
                    Serial.print(bitOffset);
                    Serial.print("; value: ");
                    Serial.println(value);
                    // end debug

                    // Invert the data if requested
                    if (invert == value)
                    {
                        result = result | B00000001;
                    }
                }
            }

            // Add result to command buffer
            this->BUFFER_DATA[*size] = result;
            *size += 1;

            // debug
            Serial.print("result: ");
            Serial.println(result, BIN);
            Serial.println("");
            // end debug
        }
    }
}

/**
 * Private functions
 */

void MobiDOT::addHeader(MobiDOT::Display type, char data[], uint &size)
{
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

    // Change the checksum if it has a certain value, this probably required to prevent a conflict somewhere
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