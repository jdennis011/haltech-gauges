#pragma once

// The wheel module's serial console, on its USB port: raw readings, the
// calibration, and what the link is doing (docs/steering-wheel.md).
namespace console {

void begin();
void poll();

}  // namespace console
