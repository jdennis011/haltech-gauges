#include "clock_text.h"

#include <stdio.h>

namespace hg {
namespace cfg {

ClockParts clockParts(uint32_t localSeconds) {
    const uint32_t day = localSeconds % 86400u;
    ClockParts p;
    p.hour = int(day / 3600u);
    p.minute = int((day / 60u) % 60u);
    p.second = int(day % 60u);
    return p;
}

void clockText(bool known, uint32_t localSeconds, bool withSeconds, bool hour12, char* out, size_t cap) {
    if (!out || cap == 0) return;
    if (!known) {
        snprintf(out, cap, withSeconds ? "--:--:--" : "--:--");
        return;
    }
    const ClockParts p = clockParts(localSeconds);
    if (hour12) {
        int h = p.hour % 12;
        if (h == 0) h = 12;
        const char* half = p.hour < 12 ? "am" : "pm";
        if (withSeconds) {
            snprintf(out, cap, "%d:%02d:%02d %s", h, p.minute, p.second, half);
        } else {
            snprintf(out, cap, "%d:%02d %s", h, p.minute, half);
        }
        return;
    }
    if (withSeconds) {
        snprintf(out, cap, "%02d:%02d:%02d", p.hour, p.minute, p.second);
    } else {
        snprintf(out, cap, "%02d:%02d", p.hour, p.minute);
    }
}

float hourHandDegrees(const ClockParts& p) { return float(p.hour % 12) * 30.0f + float(p.minute) * 0.5f; }
float minuteHandDegrees(const ClockParts& p) { return float(p.minute) * 6.0f + float(p.second) * 0.1f; }
float secondHandDegrees(const ClockParts& p) { return float(p.second) * 6.0f; }

}  // namespace cfg
}  // namespace hg
