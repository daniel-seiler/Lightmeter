#ifndef LIGHTMETER_METER_HPP
#define LIGHTMETER_METER_HPP

#include "stdint.h"

namespace meter {
    enum Warning : int8_t {
        UNDER = -1,
        OK = 0,
        OVER = 1
    };

    void update(uint16_t raw);

    // Exposure value at ISO 100, in full stops
    int8_t ev();

    // Sets whichever of f and T is not selected to match the measured light
    Warning expose();
}

#endif //LIGHTMETER_METER_HPP
