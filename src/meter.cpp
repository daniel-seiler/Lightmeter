#include "meter.hpp"
#include "config.hpp"
#include "settings.hpp"

// The photoresistor sits in a voltage divider with a 10k resistor:
//   R_ldr = 10k * (ANALOG_MAX - raw) / raw
//   lux   = 500 / R_ldr[kOhm] = 50 * raw / (ANALOG_MAX - raw)
//   EV    = log2(lux / 2.5)   = log2(20 * raw) - log2(ANALOG_MAX - raw)

namespace {
    int8_t evStops;
    bool hasReading;

    // log2 in 1/16 steps, linearly interpolated between powers of two
    int16_t log2Q4(uint16_t x) {
        uint8_t exponent = 0;
        while ((x >> exponent) > 1) {
            exponent++;
        }
        uint16_t fraction = ((uint32_t) (x - (1u << exponent)) << 4) >> exponent;
        return exponent * 16 + fraction;
    }

    int16_t evQ4(uint16_t raw) {
        raw = constrain(raw, 1, ANALOG_MAX - 1);
        return log2Q4(20 * raw) - log2Q4(ANALOG_MAX - raw) + EV_CALIBRATION;
    }
}

void meter::update(uint16_t raw) {
    int16_t ev = evQ4(raw);
    int16_t distance = ev - evStops * 16;
    // Hysteresis: the reading has to pass the midpoint between two stops by 3/16 of a stop
    if (!hasReading || distance > 11 || distance < -11) {
        evStops = (ev + 8) >> 4;
        hasReading = true;
    }
}

int8_t meter::ev() {
    return evStops;
}

meter::Warning meter::expose() {
    using namespace settings;
    // APEX: Av + Tv = Ev + Sv, with Av = f index, Tv = T_ONE_SECOND - T index, Sv = ISO index
    int8_t sum = evStops + indices[PARAM_ISO];
    Param computed = selected == PARAM_F ? PARAM_T : PARAM_F;
    int8_t idx = computed == PARAM_T ? T_ONE_SECOND - (sum - indices[PARAM_F])
                                     : sum - (T_ONE_SECOND - indices[PARAM_T]);
    int8_t last = count(computed) - 1;
    Warning warning = OK;
    if (idx < 0 || idx > last) {
        // Running out of short times or small apertures means too much light, and vice versa
        bool tooBright = computed == PARAM_T ? idx < 0 : idx > last;
        warning = tooBright ? OVER : UNDER;
        idx = constrain(idx, 0, last);
    }
    indices[computed] = idx;
    return warning;
}
