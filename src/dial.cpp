#include "dial.hpp"
#include "config.hpp"

namespace {
    uint8_t steps = 1;
    uint8_t potIndex;
    bool attached;
    int8_t direction;

    uint8_t toIndex(uint16_t pot) {
        return (uint32_t) pot * steps / (ANALOG_MAX + 1);
    }

    // The step only changes once the pot is a quarter step past the border, so a pot resting
    // between two steps does not flicker
    void quantize(uint16_t pot) {
        uint16_t low = (uint32_t) potIndex * (ANALOG_MAX + 1) / steps;
        uint16_t high = (uint32_t) (potIndex + 1) * (ANALOG_MAX + 1) / steps;
        uint16_t margin = (high - low) / 4;
        if (pot + margin < low || pot >= high + margin) {
            potIndex = toIndex(pot);
        }
    }

    void updateDirection(uint8_t index) {
        direction = attached ? 0 : (index > potIndex ? 1 : -1);
    }
}

void dial::select(uint8_t steps_, uint16_t pot, uint8_t index) {
    steps = steps_;
    potIndex = toIndex(pot);
    attached = potIndex == index;
    updateDirection(index);
}

bool dial::update(uint16_t pot, uint8_t &index) {
    uint8_t previous = potIndex;
    quantize(pot);
    if (!attached) {
        bool crossed = (previous < index && potIndex > index) || (previous > index && potIndex < index);
        attached = potIndex == index || crossed;
    }
    updateDirection(index);
    if (attached && index != potIndex) {
        index = potIndex;
        return true;
    }
    return false;
}

int8_t dial::pickupDirection() {
    return direction;
}
