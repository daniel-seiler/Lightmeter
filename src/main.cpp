#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.hpp"
#include "dial.hpp"
#include "input.hpp"
#include "meter.hpp"
#include "settings.hpp"
#include "ui.hpp"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button 1 switches between setting f and T, the other one follows the light.
// Button 2 holds the measurement. Both buttons held down set the ISO value.
bool isoMode, hold;
bool previousButton1, previousButton2;
// Set while both buttons were down, so releasing them does not count as a press
bool chord;
settings::Param controlled;
uint32_t lastFrame;

void handleButtons();

ui::Model buildModel(meter::Warning warning);

void setup() {
#ifdef LIGHTMETER_DEBUG
    Serial.begin(9600);
#endif
    pinMode(LED_BUILTIN, OUTPUT);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        for (;;) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(200);
        }
    }
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);

    input::begin();
    settings::load();
    controlled = settings::selected;
    dial::select(settings::count(controlled), input::pot(), settings::indices[controlled]);
    meter::update(input::light());
    ui::begin(buildModel(meter::expose()));
}

void loop() {
    uint32_t now = millis();
    input::update(now);
    handleButtons();

    settings::Param target = isoMode ? settings::PARAM_ISO : settings::selected;
    if (target != controlled) {
        controlled = target;
        dial::select(settings::count(controlled), input::pot(), settings::indices[controlled]);
    }
    dial::update(input::pot(), settings::indices[controlled]);
    if (!hold) {
        meter::update(input::light());
    }
    meter::Warning warning = meter::expose();
    settings::saveWhenSettled(now);

    if (ui::update(buildModel(warning), now) && now - lastFrame >= FRAME_MS) {
        lastFrame = now;
        ui::draw(display);
        display.display();
#ifdef LIGHTMETER_DEBUG
        Serial.print(input::pot());
        Serial.print(F(" : "));
        Serial.print(input::light());
        Serial.print(F(" : "));
        Serial.println(millis() - now);
#endif
    }
}

void handleButtons() {
    bool button1 = input::button1();
    bool button2 = input::button2();
    if (button1 && button2) {
        chord = true;
    }
    if (!chord) {
        if (previousButton1 && !button1) {
            settings::selected = settings::selected == settings::PARAM_F ? settings::PARAM_T
                                                                         : settings::PARAM_F;
        }
        if (previousButton2 && !button2) {
            hold = !hold;
        }
    }
    if (!button1 && !button2) {
        chord = false;
    }
    isoMode = button1 && button2;
    previousButton1 = button1;
    previousButton2 = button2;
}

ui::Model buildModel(meter::Warning warning) {
    ui::Model model;
    for (uint8_t i = 0; i < settings::PARAM_COUNT; i++) {
        model.index[i] = settings::indices[i];
    }
    model.selected = settings::selected;
    model.isoMode = isoMode;
    model.hold = hold;
    model.warning = warning;
    model.ev = meter::ev();
    model.pickup = dial::pickupDirection();
    return model;
}
