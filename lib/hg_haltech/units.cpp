#include "units.h"

#include <string.h>

namespace hg {

namespace {

#define DEG "\xC2\xB0"

const float kPsiPerKpa = 0.1450377f;

const DisplayUnit kPressure[] = {
    {"kPa", "kPa", 1.0f, 0.0f},
    {"psi", "psi", kPsiPerKpa, 0.0f},
    {"bar", "bar", 0.01f, 0.0f},
};

// Absolute pressures can also be shown relative to a standard atmosphere,
// which is what a boost gauge reading manifold pressure wants.
const DisplayUnit kPressureAbs[] = {
    {"kPa", "kPa", 1.0f, 0.0f},
    {"psi", "psi", kPsiPerKpa, 0.0f},
    {"bar", "bar", 0.01f, 0.0f},
    {"kPa_gauge", "kPa", 1.0f, -101.3f},
    {"psi_gauge", "psi", kPsiPerKpa, -101.3f * kPsiPerKpa},
    {"bar_gauge", "bar", 0.01f, -1.013f},
};

const DisplayUnit kTemperature[] = {
    {"C", DEG "C", 1.0f, 0.0f},
    {"F", DEG "F", 1.8f, 32.0f},
};

const DisplayUnit kSpeed[] = {
    {"km/h", "km/h", 1.0f, 0.0f},
    {"mph", "mph", 0.6213712f, 0.0f},
};

const DisplayUnit kLambda[] = {
    {"lambda", "\xCE\xBB", 1.0f, 0.0f},
    {"afr", "AFR", 14.7f, 0.0f},
};

const DisplayUnit kAccel[] = {
    {"m/s2", "m/s\xC2\xB2", 1.0f, 0.0f},
    {"g", "g", 0.1019716f, 0.0f},
};

const DisplayUnit kVolume[] = {
    {"L", "L", 1.0f, 0.0f},
    {"gal", "gal", 0.2641721f, 0.0f},
};

constexpr size_t kUnitCount = size_t(Unit::Metres) + 1;

struct NativeUnits {
    DisplayUnit units[kUnitCount];
    NativeUnits() {
        for (size_t i = 0; i < kUnitCount; i++) {
            units[i] = {"", unitSymbol(Unit(i)), 1.0f, 0.0f};
        }
    }
};

template <size_t N>
const DisplayUnit* table(const DisplayUnit (&t)[N], size_t& count) {
    count = N;
    return t;
}

}  // namespace

const DisplayUnit* displayUnits(Unit unit, size_t& count) {
    switch (unit) {
        case Unit::Kpa: return table(kPressure, count);
        case Unit::KpaAbs: return table(kPressureAbs, count);
        case Unit::Celsius: return table(kTemperature, count);
        case Unit::Kmh: return table(kSpeed, count);
        case Unit::Lambda: return table(kLambda, count);
        case Unit::Mps2: return table(kAccel, count);
        case Unit::Litres: return table(kVolume, count);
        default: break;
    }
    static const NativeUnits native;
    count = 1;
    return &native.units[size_t(unit)];
}

const DisplayUnit* findDisplayUnit(Unit unit, const char* name) {
    size_t count = 0;
    const DisplayUnit* units = displayUnits(unit, count);
    if (!name || !name[0]) return &units[0];
    for (size_t i = 0; i < count; i++) {
        if (strcmp(units[i].name, name) == 0) return &units[i];
    }
    return nullptr;
}

}  // namespace hg
