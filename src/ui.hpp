#ifndef LIGHTMETER_UI_HPP
#define LIGHTMETER_UI_HPP

#include <Adafruit_SSD1306.h>
#include "settings.hpp"

namespace ui {
    struct Model {
        uint8_t index[settings::PARAM_COUNT];
        uint8_t selected;   // PARAM_F or PARAM_T
        bool isoMode;
        bool hold;
        int8_t warning;     // meter::Warning
        int8_t ev;
        int8_t pickup;      // dial::pickupDirection()
    };

    void begin(const Model &model);

    // Returns true if the screen has to be redrawn
    bool update(const Model &model, uint32_t now);

    void draw(Adafruit_SSD1306 &display);
}

#endif //LIGHTMETER_UI_HPP
