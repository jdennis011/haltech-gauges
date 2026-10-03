#pragma once

#include <stdint.h>

// The parts of a gauge board that differ between the Waveshare 1.75" AMOLED
// and the M5Stack Dial: power-up, the supply reading, and any per-loop
// housekeeping the board's own library needs.
namespace board {

// Brings up buses and board libraries. Call before display::begin().
void begin();

// Call once per loop, before LVGL runs.
void update();

// Supply voltage at the board's 5 V input, or 0 if the board cannot read it.
uint16_t vbusMillivolts();

const char* name();

}  // namespace board
