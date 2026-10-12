#ifndef LIGHTMETER_BITMAP_HPP
#define LIGHTMETER_BITMAP_HPP

#include <Adafruit_SSD1306.h>
#include "stdint.h"

namespace bitmap {
    // Stored in PROGMEM. The data uses the page layout of the display buffer: frame by frame,
    // page by page (8 rows each), one byte per column with the top pixel in bit 0.
    struct Sprite {
        uint8_t width, height, frames;
        const uint8_t *data;
    };

    void draw(Adafruit_SSD1306 &display, const Sprite &sprite, uint8_t x, uint8_t y, uint8_t frame = 0);
}

#endif //LIGHTMETER_BITMAP_HPP
