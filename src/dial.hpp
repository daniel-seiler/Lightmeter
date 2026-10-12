#ifndef LIGHTMETER_DIAL_HPP
#define LIGHTMETER_DIAL_HPP

#include "stdint.h"

// The potentiometer is shared between all settings. After switching to another setting it has
// to be turned to the stored value before it takes over, so the setting does not jump.
namespace dial {
    void select(uint8_t steps, uint16_t pot, uint8_t index);

    // Returns true if index was changed
    bool update(uint16_t pot, uint8_t &index);

    // Direction the pot has to be turned to take over: -1 or +1, 0 once it is in control
    int8_t pickupDirection();
}

#endif //LIGHTMETER_DIAL_HPP
