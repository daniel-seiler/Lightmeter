#include "bitmap.hpp"

void bitmap::draw(Adafruit_SSD1306 &display, const Sprite &sprite, uint8_t x, uint8_t y, uint8_t frame) {
    Sprite s;
    memcpy_P(&s, &sprite, sizeof(Sprite));

    const uint8_t screenWidth = display.width();
    const uint8_t screenPages = display.height() / 8;
    const uint8_t pages = (s.height + 7) / 8;
    const uint8_t shift = y & 7;
    const uint8_t columns = (x + s.width > screenWidth) ? screenWidth - x : s.width;
    const uint8_t *src = s.data + (uint16_t) frame * pages * s.width;

    for (uint8_t page = 0; page < pages; page++, src += s.width) {
        uint8_t row = (y >> 3) + page;
        if (row >= screenPages) {
            break;
        }
        uint8_t *dst = display.getBuffer() + (uint16_t) row * screenWidth + x;
        bool spill = shift && row + 1 < screenPages;
        for (uint8_t col = 0; col < columns; col++) {
            uint8_t b = pgm_read_byte(src + col);
            dst[col] |= b << shift;
            if (spill) {
                dst[col + screenWidth] |= b >> (8 - shift);
            }
        }
    }
}
