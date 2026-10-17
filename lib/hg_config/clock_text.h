#pragma once

// Time of day for the clock widget (docs/time-of-day.md). The hub's beacon
// carries the time as seconds since 1970 in the hub's own zone, so a gauge
// needs no zone of its own: hours, minutes and seconds fall straight out.

#include <stddef.h>
#include <stdint.h>

namespace hg {
namespace cfg {

struct ClockParts {
    int hour;    // 0 to 23
    int minute;
    int second;
};

ClockParts clockParts(uint32_t localSeconds);

// "13:05", "13:05:09", "1:05 pm", "1:05:09 pm". Without a known time, dashes
// in the same shape: "--:--", "--:--:--". `cap` of 12 always fits.
void clockText(bool known, uint32_t localSeconds, bool withSeconds, bool hour12, char* out, size_t cap);

// Where the hands point, degrees clockwise from 12 o'clock.
float hourHandDegrees(const ClockParts& p);
float minuteHandDegrees(const ClockParts& p);
float secondHandDegrees(const ClockParts& p);

}  // namespace cfg
}  // namespace hg
