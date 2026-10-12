#ifndef LIGHTMETER_SETTINGS_HPP
#define LIGHTMETER_SETTINGS_HPP

#include "stdint.h"

namespace settings {
    enum Param : uint8_t {
        PARAM_F,
        PARAM_T,
        PARAM_ISO,
        PARAM_COUNT
    };

    // All tables are in full stops, so exposure is calculated on the indices.
    // Index 0 is f/1, 1/8000 s and ISO 100; 1 s is at T_ONE_SECOND.
    const uint8_t F_COUNT = 13;
    const uint8_t T_COUNT = 19;
    const uint8_t T_ONE_SECOND = 13;
    const uint8_t ISO_COUNT = 6;
    const uint8_t TEXT_SIZE = 7;

    // selected is the one of f and T that is set by the dial, the other one is metered
    extern uint8_t indices[PARAM_COUNT];
    extern Param selected;

    uint8_t count(Param param);

    // Returns the length of the text
    uint8_t format(Param param, uint8_t idx, char *buffer);

    void load();

    void saveWhenSettled(uint32_t now);
}

#endif //LIGHTMETER_SETTINGS_HPP
