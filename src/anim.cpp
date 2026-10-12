#include "anim.hpp"
#include <Arduino.h>

namespace {
    // Eased progress 0..255 at 17 evenly spaced points in time
    const uint8_t EASE_OUT_TABLE[] PROGMEM = {0, 45, 84, 118, 147, 172, 193, 210, 223, 234, 242,
                                              247, 251, 253, 254, 255, 255};
    const uint8_t EASE_IN_OUT_TABLE[] PROGMEM = {0, 3, 11, 24, 40, 59, 81, 104, 128, 151, 174,
                                                 196, 215, 231, 244, 252, 255};

    uint8_t ease(anim::Easing easing, uint8_t progress) {
        if (easing == anim::LINEAR) {
            return progress;
        }
        const uint8_t *table = easing == anim::EASE_OUT ? EASE_OUT_TABLE : EASE_IN_OUT_TABLE;
        uint8_t a = pgm_read_byte(table + (progress >> 4));
        uint8_t b = pgm_read_byte(table + (progress >> 4) + 1);
        return a + (((b - a) * (progress & 15)) >> 4);
    }
}

void anim::set(Tween &tween, int16_t value) {
    tween.from = tween.to = tween.value = value;
    tween.active = false;
}

void anim::start(Tween &tween, int16_t to, uint16_t duration, Easing easing, uint32_t now) {
    tween.from = tween.value;
    tween.to = to;
    tween.startedAt = now;
    tween.duration = duration;
    tween.easing = easing;
    tween.active = tween.from != to;
}

bool anim::update(Tween &tween, uint32_t now) {
    if (!tween.active) {
        return false;
    }
    uint16_t elapsed = (uint16_t) now - tween.startedAt;
    if (elapsed >= tween.duration) {
        tween.value = tween.to;
        tween.active = false;
    } else {
        uint8_t progress = ((uint32_t) elapsed << 8) / tween.duration;
        int32_t distance = tween.to - tween.from;
        tween.value = tween.from + distance * ease(tween.easing, progress) / 255;
    }
    return true;
}

bool anim::updateAll(Tween *tweens, uint8_t count, uint32_t now) {
    bool moved = false;
    for (uint8_t i = 0; i < count; i++) {
        moved |= update(tweens[i], now);
    }
    return moved;
}
