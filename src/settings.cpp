#include "settings.hpp"
#include "config.hpp"
#include <EEPROM.h>

#define EEPROM_MAGIC 0xA7

namespace {
    // f-number times ten
    const uint16_t F_NUMBER[] PROGMEM = {10, 14, 20, 28, 40, 56, 80, 110, 160, 220, 320, 450, 640};
    // Fractions of a second up to T_ONE_SECOND, whole seconds from there on
    const uint16_t T_VALUE[] PROGMEM = {8000, 4000, 2000, 1000, 500, 250, 125, 60, 30, 15, 8, 4, 2,
                                        1, 2, 4, 8, 15, 30};

    struct Stored {
        uint8_t selected, value, iso;

        bool operator!=(const Stored &other) const {
            return selected != other.selected || value != other.value || iso != other.iso;
        }
    };

    Stored saved, pending;
    uint32_t pendingSince;

    Stored current() {
        return {settings::selected, settings::indices[settings::selected],
                settings::indices[settings::PARAM_ISO]};
    }

    uint8_t formatNumber(uint16_t number, char *buffer) {
        char digits[5];
        uint8_t n = 0;
        do {
            digits[n++] = '0' + number % 10;
            number /= 10;
        } while (number);
        for (uint8_t i = 0; i < n; i++) {
            buffer[i] = digits[n - 1 - i];
        }
        buffer[n] = '\0';
        return n;
    }
}

uint8_t settings::indices[PARAM_COUNT] = {5, 6, 1};
settings::Param settings::selected = PARAM_F;

uint8_t settings::count(Param param) {
    switch (param) {
        case PARAM_F:
            return F_COUNT;
        case PARAM_T:
            return T_COUNT;
        default:
            return ISO_COUNT;
    }
}

uint8_t settings::format(Param param, uint8_t idx, char *buffer) {
    uint8_t n;
    switch (param) {
        case PARAM_F: {
            uint16_t f = pgm_read_word(&F_NUMBER[idx]);
            n = formatNumber(f / 10, buffer);
            if (f < 100 && f % 10) {
                buffer[n++] = '.';
                buffer[n++] = '0' + f % 10;
                buffer[n] = '\0';
            }
            return n;
        }
        case PARAM_T: {
            uint16_t t = pgm_read_word(&T_VALUE[idx]);
            if (idx < T_ONE_SECOND) {
                buffer[0] = '1';
                buffer[1] = '/';
                return 2 + formatNumber(t, buffer + 2);
            }
            n = formatNumber(t, buffer);
            buffer[n++] = 's';
            buffer[n] = '\0';
            return n;
        }
        default:
            return formatNumber(100 << idx, buffer);
    }
}

void settings::load() {
    Stored stored = {EEPROM.read(1), EEPROM.read(2), EEPROM.read(3)};
    if (EEPROM.read(0) == EEPROM_MAGIC && stored.selected <= PARAM_T
        && stored.value < count((Param) stored.selected) && stored.iso < ISO_COUNT) {
        selected = (Param) stored.selected;
        indices[selected] = stored.value;
        indices[PARAM_ISO] = stored.iso;
    }
    saved = pending = current();
}

void settings::saveWhenSettled(uint32_t now) {
    Stored state = current();
    if (state != pending) {
        pending = state;
        pendingSince = now;
    } else if (pending != saved && now - pendingSince >= SAVE_DELAY_MS) {
        EEPROM.update(0, EEPROM_MAGIC);
        EEPROM.update(1, pending.selected);
        EEPROM.update(2, pending.value);
        EEPROM.update(3, pending.iso);
        saved = pending;
    }
}
