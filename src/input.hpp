#ifndef LIGHTMETER_INPUT_HPP
#define LIGHTMETER_INPUT_HPP

#include "stdint.h"

namespace input {
    void begin();

    void update(uint32_t now);

    bool button1();

    bool button2();

    uint16_t pot();

    uint16_t light();
}

#endif //LIGHTMETER_INPUT_HPP
