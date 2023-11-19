#pragma once

#include "stdint.h"

namespace bitmap {
    struct bitmap_t {
        uint16_t width, height;
        uint8_t bmp[];
    };
}
