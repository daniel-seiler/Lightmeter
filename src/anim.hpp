#ifndef LIGHTMETER_ANIM_HPP
#define LIGHTMETER_ANIM_HPP

#include "stdint.h"

namespace anim {
    enum Easing : uint8_t {
        LINEAR,
        EASE_OUT,
        EASE_IN_OUT
    };

    struct Tween {
        int16_t from, to, value;
        uint16_t startedAt, duration;
        Easing easing;
        bool active;
    };

    void set(Tween &tween, int16_t value);

    void start(Tween &tween, int16_t to, uint16_t duration, Easing easing, uint32_t now);

    bool update(Tween &tween, uint32_t now);

    bool updateAll(Tween *tweens, uint8_t count, uint32_t now);
}

#endif //LIGHTMETER_ANIM_HPP
