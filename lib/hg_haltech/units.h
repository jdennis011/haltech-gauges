#pragma once

#include <stddef.h>

#include "haltech.h"

namespace hg {

// A way of showing a channel: displayed = stored * mul + add.
struct DisplayUnit {
    const char* name;    // identifier used in face configs, e.g. "psi_gauge"
    const char* symbol;  // text shown on the gauge
    float mul;
    float add;
};

// Conversions available for a channel unit. The first entry is the native unit.
const DisplayUnit* displayUnits(Unit unit, size_t& count);

// Looks a conversion up by name. A null or empty name gives the native unit;
// an unknown name gives nullptr.
const DisplayUnit* findDisplayUnit(Unit unit, const char* name);

inline float toDisplay(const DisplayUnit& u, float value) { return value * u.mul + u.add; }

}  // namespace hg
