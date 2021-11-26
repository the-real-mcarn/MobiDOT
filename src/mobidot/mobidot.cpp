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

    // Init light relay pin
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

void MobiDOT::print(const char c[], int offsetX, int offsetY)
{
    const MobiDOT::Font font = this->display[(uint)this->DISPLAY_DEFAULT].defaultFont;
    this->print(c, font, offsetX, offsetY);
}

void MobiDOT::print(const char c[], MobiDOT::Font font)
{
    this->print(c, font, 0, 0);
}

void MobiDOT::print(const char c[], MobiDOT::Font font, int offsetX, int offsetY)
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

void MobiDOT::print(const char c[], const GFXfont *font, bool invert)
{
    this->print(c, font, 0, 0, invert);
}

void MobiDOT::print(const char c[], const GFXfont *font, int offsetX, int offsetY, bool invert)
{
    // First char in the font
    const uint8_t first = pgm_read_word(&font->first);

    // Char ascii index
    uint8_t index;

    // Cursor for x position
    uint16_t cursor = 0;

    // Hightest letter in the string determines the size of the buffer
    int16_t bufferHeight = 0;

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

        const uint8_t charH = pgm_read_word(&g->height);
        if (charH > bufferHeight)
        {
            bufferHeight = charH;
        }
    }

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
        const int8_t charXadvance = pgm_read_word(&g->xAdvance);
        const int8_t charXoffset = pgm_read_word(&g->xOffset);
        const int8_t charYoffset = pgm_read_word(&g->yOffset);

        const uint16_t charSize = charW * charH;
        const uint8_t charByteSize = ceil(charSize / 8.0);

        // Create array pointer to store char data in
        uint8_t *charData = new uint8_t[charByteSize];

        // Read char data byte by byte from the font bitmap
        for (uint8_t byte = 0; byte <= charByteSize - 1; byte++)
        {
            // Get value from progmem
            const uint8_t value = pgm_read_word(&font->bitmap[bitmapOffset + byte]);

            // Store data in array
            charData[byte] = value;
        }

        // Now that we have the char data, it is time to make it into a bitmap, which means padding it to full bytes
        // Determine width
        uint8_t bufferByteW;
        if (invert && charXadvance % 8 == 0)
        {
            bufferByteW = ceil((charXadvance + 2) / 8.0);
        }
        else
        {
            bufferByteW = ceil((charXadvance) / 8.0);
        }

        // Create buffer and zero it
        uint8_t *buffer = new uint8_t[bufferByteW * bufferHeight];
        memset(buffer, 0, bufferByteW * bufferHeight);

        // Go through char bit by bit
        // TODO: Find a way to optimise this?
        for (uint8_t b = 0; b < charSize; b++)
        {
            // Get the value of the current bit, if it is zero we can skip it
            uint8_t charByteOffset = floor(b / 8.0); // Start with zero
            uint8_t charBitOffset = b % 8;
            const uint8_t value = charData[charByteOffset] >> (7 - charBitOffset) & 0b00000001;

            // Skip if value is zero, shifting zero's is a waste of time
            if (value)
            {
                // First determine which byte this bit falls in
                const uint8_t line = floor(b / charW); // Start with zero
                uint8_t bufferByteOffset = line * bufferByteW;
                const int8_t letterLineOffset = (bufferHeight + charYoffset - (charH + charYoffset)) * bufferByteW;

                // Get the bit offset in the current line
                uint8_t bufferBitOffset = b - (line * charW) + charXoffset;

                // Shift everything one bit if inverting text to make sure that text does not dissapear into the background
                if (invert)
                {
                    bufferBitOffset++;
                }

                // Bytes are 8 bits, if the bit offset is larger than 8 it should be added to the byte offset
                while (bufferBitOffset >= 8)
                {
                    bufferBitOffset = bufferBitOffset - 8;
                    bufferByteOffset++;
                }

                // Copy value to the bitmap buffer
                char result = 0x01;
                result = result << (7 - bufferBitOffset);
                buffer[bufferByteOffset + letterLineOffset] = buffer[bufferByteOffset + letterLineOffset] | result;
            }
        }

        // Draw the char
        MobiDOT::drawBitmap(
            buffer,                                     // Bitmap buffer
            (invert) ? charXadvance + 1 : charXadvance, // Width including whitespace after char and before char if inverting
            bufferHeight,                               // Height
            offsetX + cursor,                           // X offset
            offsetY,                                    // Y offset
            !invert                                     // Invert
        );

        // Update cursor for the next char
        cursor = cursor + charXadvance;
    }
};

void MobiDOT::drawRect(uint width, uint height, bool fill)
{
    MobiDOT::drawRect(width, height, 0, 0, fill);
}

void MobiDOT::drawRect(uint width, uint height, int x, int y, bool fill)
{
    // Determine width of the buffer
    const uint8_t bufferW = ceil((width) / 8.0);

    // Create buffer
    uint8_t *buffer = new uint8_t[bufferW * height];

    // Just the outline or fill the entire thing?
    if (fill)
    {
        // Easy, just fill the buffer with 1's and send it to drawBitmap
        memset(buffer, 0xff, bufferW * height);
    }
    else
    {
        // Slightly more complicated
        memset(buffer, 0, bufferW * height);

        // Horizontal lines
        for (size_t i = 0; i < bufferW; i++)
        {
            buffer[i] = 0xff;
            buffer[i + (height - 1) * bufferW] = 0xff;
        }

        // Vertical lines
        uint8_t byteOffset = 0;
        uint8_t bitOffset = width;

        while (bitOffset > 8)
        {
            bitOffset = bitOffset - 8;
            byteOffset++;
        }

        bitOffset = 8 - bitOffset;

        for (size_t i = 0; i < height; i++)
        {
            buffer[i * bufferW] = buffer[i * bufferW] | 0x80;

            char value = 0x01 << bitOffset;
            buffer[i * bufferW + byteOffset] = buffer[i * bufferW + byteOffset] | value;
        }
    }

    // Draw the rectangle
    MobiDOT::drawBitmap(
        buffer, // Bitmap buffer
        width,  // Width including whitespace after char and before char if inverting
        height, // Height
        x,      // X offset
        y,      // Y offset
        true    // Invert
    );
}

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

void MobiDOT::drawBitmap(const unsigned char data[], uint width, uint height, int x, int y, bool invert)
{
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
        // Add bitmap header
        this->BUFFER_DATA[*size] = 0xd2;
        this->BUFFER_DATA[*size + 1] = x;
        this->BUFFER_DATA[*size + 2] = 0xd3;
        this->BUFFER_DATA[*size + 3] = y + 4 + (i * 5);
        this->BUFFER_DATA[*size + 4] = 0xd4;
        this->BUFFER_DATA[*size + 5] = (char)MobiDOT::Font::BITWISE;
        *size += 6;

        // Go through bitmap line by line (width)
        // for (int16_t j = (bytesOverWidth * 8 - 1); j >= (int)((bytesOverWidth * 8) - width); j--)
        for (int16_t j = 0; j <= (int)width - 1; j++)
        {
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
                    // Get value of current bit
                    const int8_t value = data[byteOffset] >> (7 - bitOffset) & B00000001;

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