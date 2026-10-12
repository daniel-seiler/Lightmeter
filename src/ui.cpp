#include "ui.hpp"
#include "anim.hpp"
#include "bitmap_generated.hpp"
#include "config.hpp"

using namespace settings;

#define ROLL_DISTANCE 8
#define ROLL_MS 220
#define GEAR_MS 320
#define MARKER_MS 250
#define UNDERLINE_MS 350
#define FOCUS_MS 200
#define ARROW_MS 400
#define GEAR_MAX_TEETH 3

#define VALUE_Y 29
#define T_VALUE_X 12    // left edge
#define F_VALUE_X 110   // right edge
#define ISO_VALUE_Y 2
#define ISO_LABEL_Y 11
#define CENTER_X 64
#define STATUS_Y 55
#define UNDERLINE_Y 39
#define UNDERLINE_WIDTH 24
#define UNDERLINE_LEFT_X 12
#define UNDERLINE_RIGHT_X 85
#define FOCUS_WIDTH 26
#define CHAR_WIDTH 6

namespace {
    enum Side : uint8_t {
        LEFT,   // T
        RIGHT   // f
    };

    enum Align : uint8_t {
        ALIGN_LEFT,
        ALIGN_RIGHT,
        ALIGN_CENTER
    };

    // The first three tweens belong to the parameter with the same number
    enum TweenId : uint8_t {
        ROLL,
        GEAR = ROLL + PARAM_COUNT,
        MARKER = GEAR + 2,
        UNDERLINE = MARKER + 2,
        FOCUS,
        TWEEN_COUNT
    };

    // Column of the dial arcs for each row of their sprites, to place the markers on them
    const uint8_t ARC_LEFT[] PROGMEM = {1, 3, 5, 6, 7, 8, 8, 9, 9, 9, 9, 9, 9, 8, 8, 7, 6, 5, 3, 1};
    const uint8_t ARC_RIGHT[] PROGMEM = {14, 11, 8, 7, 5, 4, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0,
                                         0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 4, 5, 7, 8, 11, 14};
    const uint8_t MARKER_TOP[] = {3, 4};
    const uint8_t MARKER_BOTTOM[] = {16, 27};

    anim::Tween tweens[TWEEN_COUNT];
    ui::Model shown;
    uint8_t previous[PARAM_COUNT];
    int8_t rollDirection[PARAM_COUNT];
    int8_t arrowDirection[2];
    uint16_t arrowOffAt[2];
    bool blink;
    bool dirty;

    Side sideOf(uint8_t param) {
        return param == PARAM_T ? LEFT : RIGHT;
    }

    Param paramOf(Side side) {
        return side == LEFT ? PARAM_T : PARAM_F;
    }

    int16_t markerRow(Side side, uint8_t idx) {
        uint8_t range = MARKER_BOTTOM[side] - MARKER_TOP[side];
        return MARKER_BOTTOM[side] - idx * range / (count(paramOf(side)) - 1);
    }

    int16_t underlineX(uint8_t selected) {
        return selected == PARAM_T ? UNDERLINE_LEFT_X : UNDERLINE_RIGHT_X;
    }

    void flashArrow(Side side, int8_t direction, uint32_t now) {
        arrowDirection[side] = direction;
        arrowOffAt[side] = (uint16_t) now + ARROW_MS;
    }

    void turnGear(Side side, int8_t steps, uint32_t now) {
        anim::Tween &gear = tweens[GEAR + side];
        if (!gear.active) {
            // At rest the gear always sits on frame 0, so the position can start over
            anim::set(gear, 0);
        }
        steps = constrain(steps, -GEAR_MAX_TEETH, GEAR_MAX_TEETH);
        anim::start(gear, gear.to + steps * bitmap::GEAR_LEFT_FRAMES, GEAR_MS, anim::EASE_OUT, now);
    }

    void valueChanged(uint8_t param, int8_t difference, uint32_t now) {
        int8_t direction = difference > 0 ? 1 : -1;
        previous[param] = shown.index[param];
        rollDirection[param] = direction;
        anim::set(tweens[ROLL + param], direction * ROLL_DISTANCE);
        anim::start(tweens[ROLL + param], 0, ROLL_MS, anim::EASE_OUT, now);

        if (param == PARAM_ISO) {
            flashArrow(LEFT, direction, now);
            flashArrow(RIGHT, direction, now);
            return;
        }
        Side side = sideOf(param);
        flashArrow(side, direction, now);
        turnGear(side, difference, now);
        anim::start(tweens[MARKER + side], markerRow(side, shown.index[param] + difference),
                    MARKER_MS, anim::EASE_OUT, now);
    }

    void drawText(Adafruit_SSD1306 &display, const char *text, uint8_t length, int16_t x, int16_t y,
                  Align align) {
        if (align == ALIGN_RIGHT) {
            x -= length * CHAR_WIDTH;
        } else if (align == ALIGN_CENTER) {
            x -= length * CHAR_WIDTH / 2;
        }
        display.setCursor(x, y);
        display.print(text);
    }

    void drawValue(Adafruit_SSD1306 &display, uint8_t param, int16_t x, int16_t y, Align align) {
        char text[TEXT_SIZE];
        const anim::Tween &roll = tweens[ROLL + param];
        y += roll.value;
        uint8_t length = format((Param) param, shown.index[param], text);
        drawText(display, text, length, x, y, align);
        if (roll.active) {
            length = format((Param) param, previous[param], text);
            drawText(display, text, length, x, y - rollDirection[param] * ROLL_DISTANCE, align);
        }
    }

    void drawLabel(Adafruit_SSD1306 &display, const char *text, int16_t x, int16_t y, bool inverted) {
        if (inverted) {
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        }
        display.setCursor(x, y);
        display.print(text);
        display.setTextColor(SSD1306_WHITE);
    }

    void drawStatus(Adafruit_SSD1306 &display) {
        if (shown.warning && blink) {
            drawText(display, shown.warning > 0 ? "OVER" : "UNDR", 4, CENTER_X, STATUS_Y, ALIGN_CENTER);
        } else if (shown.hold) {
            display.fillRect(CENTER_X - 2 * CHAR_WIDTH - 1, STATUS_Y - 1, 4 * CHAR_WIDTH + 1, 9, SSD1306_WHITE);
            drawLabel(display, "HOLD", CENTER_X - 2 * CHAR_WIDTH, STATUS_Y, true);
        } else if (!shown.warning) {
            char text[6] = "EV";
            itoa(shown.ev, text + 2, 10);
            drawText(display, text, strlen(text), CENTER_X, STATUS_Y, ALIGN_CENTER);
        }
    }

    void drawMarker(Adafruit_SSD1306 &display, Side side) {
        uint8_t row = tweens[MARKER + side].value;
        int16_t x = side == LEFT ? bitmap::CIRCLE_LEFT_X + pgm_read_byte(ARC_LEFT + row)
                                 : bitmap::CIRCLE_RIGHT_X + pgm_read_byte(ARC_RIGHT + row);
        int16_t y = (side == LEFT ? bitmap::CIRCLE_LEFT_Y : bitmap::CIRCLE_RIGHT_Y) + row;
        display.fillRect(x - 1, y - 1, 3, 3, SSD1306_WHITE);
    }

    // Arrows are filled briefly after a value moved in their direction, and blink while the
    // dial has to be turned that way to take over the setting
    bool arrowActive(Side side, int8_t direction) {
        if (arrowDirection[side] == direction) {
            return true;
        }
        bool controlled = shown.isoMode || sideOf(shown.selected) == side;
        return controlled && shown.pickup == direction && blink;
    }

    uint8_t gearFrame(Side side) {
        int16_t frame = tweens[GEAR + side].value % bitmap::GEAR_LEFT_FRAMES;
        return frame < 0 ? frame + bitmap::GEAR_LEFT_FRAMES : frame;
    }
}

void ui::begin(const Model &model) {
    shown = model;
    for (uint8_t i = 0; i < TWEEN_COUNT; i++) {
        anim::set(tweens[i], 0);
    }
    anim::set(tweens[MARKER + LEFT], markerRow(LEFT, model.index[PARAM_T]));
    anim::set(tweens[MARKER + RIGHT], markerRow(RIGHT, model.index[PARAM_F]));
    anim::set(tweens[UNDERLINE], underlineX(model.selected));
    anim::set(tweens[FOCUS], model.isoMode ? FOCUS_WIDTH : 0);
    arrowDirection[LEFT] = arrowDirection[RIGHT] = 0;
    dirty = true;
}

bool ui::update(const Model &model, uint32_t now) {
    if (memcmp(&model, &shown, sizeof(Model)) != 0) {
        dirty = true;
    }
    for (uint8_t param = 0; param < PARAM_COUNT; param++) {
        int8_t difference = model.index[param] - shown.index[param];
        if (difference != 0) {
            valueChanged(param, difference, now);
        }
    }
    if (model.selected != shown.selected) {
        anim::start(tweens[UNDERLINE], underlineX(model.selected), UNDERLINE_MS, anim::EASE_IN_OUT, now);
    }
    if (model.isoMode != shown.isoMode) {
        anim::start(tweens[FOCUS], model.isoMode ? FOCUS_WIDTH : 0, FOCUS_MS, anim::EASE_OUT, now);
    }
    shown = model;

    if (anim::updateAll(tweens, TWEEN_COUNT, now)) {
        dirty = true;
    }
    for (uint8_t side = 0; side < 2; side++) {
        if (arrowDirection[side] && (int16_t) ((uint16_t) now - arrowOffAt[side]) >= 0) {
            arrowDirection[side] = 0;
            dirty = true;
        }
    }
    bool phase = (now >> 8) & 1;
    if (phase != blink) {
        blink = phase;
        if (shown.pickup || shown.warning) {
            dirty = true;
        }
    }
    return dirty;
}

void ui::draw(Adafruit_SSD1306 &display) {
    display.clearDisplay();

    // Values are drawn first: while rolling they overlap the rows above and below, which are
    // cleared again before anything else is drawn
    drawValue(display, PARAM_T, T_VALUE_X, VALUE_Y, ALIGN_LEFT);
    drawValue(display, PARAM_F, F_VALUE_X, VALUE_Y, ALIGN_RIGHT);
    drawValue(display, PARAM_ISO, CENTER_X, ISO_VALUE_Y, ALIGN_CENTER);
    display.fillRect(0, VALUE_Y - ROLL_DISTANCE - 1, SCREEN_WIDTH, ROLL_DISTANCE, SSD1306_BLACK);
    display.fillRect(0, VALUE_Y + 9, SCREEN_WIDTH, ROLL_DISTANCE, SSD1306_BLACK);
    display.fillRect(CENTER_X - 12, ISO_LABEL_Y - 1, 24, 9, SSD1306_BLACK);

    uint8_t focus = tweens[FOCUS].value;
    display.fillRect(CENTER_X - focus / 2, ISO_VALUE_Y - 1, focus, 9, SSD1306_INVERSE);
    display.drawLine(CENTER_X - 9, ISO_LABEL_Y - 1, CENTER_X + 8, ISO_LABEL_Y - 1, SSD1306_WHITE);
    drawLabel(display, "ISO", CENTER_X - 9, ISO_LABEL_Y, true);

    drawLabel(display, "T", 0, VALUE_Y, false);
    drawLabel(display, "f/", 116, VALUE_Y, false);
    display.drawFastHLine(tweens[UNDERLINE].value, UNDERLINE_Y, UNDERLINE_WIDTH, SSD1306_WHITE);
    drawStatus(display);

    bitmap::draw(display, bitmap::GEAR_RIGHT, bitmap::GEAR_RIGHT_X, bitmap::GEAR_RIGHT_Y, gearFrame(LEFT));
    bitmap::draw(display, bitmap::GEAR_LEFT, bitmap::GEAR_LEFT_X, bitmap::GEAR_LEFT_Y, gearFrame(RIGHT));
    bitmap::draw(display, bitmap::CIRCLE_LEFT, bitmap::CIRCLE_LEFT_X, bitmap::CIRCLE_LEFT_Y);
    bitmap::draw(display, bitmap::CIRCLE_RIGHT, bitmap::CIRCLE_RIGHT_X, bitmap::CIRCLE_RIGHT_Y);
    drawMarker(display, LEFT);
    drawMarker(display, RIGHT);

    bitmap::draw(display, arrowActive(LEFT, 1) ? bitmap::ARROW_ACTIVE_TL : bitmap::ARROW_INACTIVE_TL,
                 bitmap::ARROW_TL_X, bitmap::ARROW_TL_Y);
    bitmap::draw(display, arrowActive(LEFT, -1) ? bitmap::ARROW_ACTIVE_BL : bitmap::ARROW_INACTIVE_BL,
                 bitmap::ARROW_BL_X, bitmap::ARROW_BL_Y);
    bitmap::draw(display, arrowActive(RIGHT, 1) ? bitmap::ARROW_ACTIVE_TR : bitmap::ARROW_INACTIVE_TR,
                 bitmap::ARROW_TR_X, bitmap::ARROW_TR_Y);
    bitmap::draw(display, arrowActive(RIGHT, -1) ? bitmap::ARROW_ACTIVE_BR : bitmap::ARROW_INACTIVE_BR,
                 bitmap::ARROW_BR_X, bitmap::ARROW_BR_Y);
    dirty = false;
}
