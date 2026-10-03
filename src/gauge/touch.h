#pragma once

namespace touch {

// Starts the board's touch controller and registers it with LVGL as a pointer
// device. Call after board::begin() and display::begin().
bool begin();

}  // namespace touch
