#include "input.hpp"
#include "config.hpp"

namespace {
    struct Button {
        uint8_t pin;
        bool pressed, raw;
        uint32_t changedAt;
    };

    // Exponential moving average, kept in 1/16 steps
    struct Analog {
        uint8_t pin;
        uint8_t smoothing;
        int16_t acc;
    };

    Button buttons[] = {{BUTTON1_PIN, false, false, 0},
                        {BUTTON2_PIN, false, false, 0}};
    Analog potInput = {POT_PIN, 1, 0};
    Analog lightInput = {PHOTO_RES_PIN, 3, 0};
    uint32_t sampledAt;

    void updateButton(Button &button, uint32_t now) {
        bool raw = digitalRead(button.pin) == HIGH;
        if (raw != button.raw) {
            button.raw = raw;
            button.changedAt = now;
        } else if (raw != button.pressed && now - button.changedAt >= DEBOUNCE_MS) {
            button.pressed = raw;
        }
    }

    void updateAnalog(Analog &analog) {
        int16_t raw = analogRead(analog.pin) << 4;
        analog.acc += (raw - analog.acc) >> analog.smoothing;
    }

    uint16_t value(const Analog &analog) {
        return (analog.acc + 8) >> 4;
    }
}

void input::begin() {
    for (Button &button: buttons) {
        pinMode(button.pin, INPUT);
    }
    pinMode(POT_PIN, INPUT);
    pinMode(PHOTO_RES_PIN, INPUT);
    potInput.acc = analogRead(POT_PIN) << 4;
    lightInput.acc = analogRead(PHOTO_RES_PIN) << 4;
}

void input::update(uint32_t now) {
    for (Button &button: buttons) {
        updateButton(button, now);
    }
    if (now - sampledAt >= ANALOG_SAMPLE_MS) {
        sampledAt = now;
        updateAnalog(potInput);
        updateAnalog(lightInput);
    }
}

bool input::button1() {
    return buttons[0].pressed;
}

bool input::button2() {
    return buttons[1].pressed;
}

uint16_t input::pot() {
    return value(potInput);
}

uint16_t input::light() {
    return value(lightInput);
}
