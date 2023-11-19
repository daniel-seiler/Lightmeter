#pragma once

#include "stdint.h"

namespace bitmap {
    struct bitmap_t {
        uint16_t width, height;
        uint8_t bmp[];
    };

    static const PROGMEM uint16_t ARROW_TL_X     = 20;
    static const PROGMEM uint16_t ARROW_TL_Y     = 8;
    static const PROGMEM uint16_t ARROW_TR_X     = 20;
    static const PROGMEM uint16_t ARROW_TR_Y     = 40;
    static const PROGMEM uint16_t ARROW_BL_X     = 95;
    static const PROGMEM uint16_t ARROW_BL_Y     = 8;
    static const PROGMEM uint16_t ARROW_BR_X     = 95;
    static const PROGMEM uint16_t ARROW_BR_Y     = 40;
    static const PROGMEM uint16_t CIRCLE_RIGHT_X = 112;
    static const PROGMEM uint16_t CIRCLE_RIGHT_Y = 16;
    static const PROGMEM uint16_t CIRCLE_LEFT_X  = 0;
    static const PROGMEM uint16_t CIRCLE_LEFT_Y  = 22;
    static const PROGMEM uint16_t GEAR_RIGHT_X   = 33;
    static const PROGMEM uint16_t GEAR_RIGHT_Y   = 0;
    static const PROGMEM uint16_t GEAR_LEFT_X    = 71;
    static const PROGMEM uint16_t GEAR_LEFT_Y    = 0;
}
