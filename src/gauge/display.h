#pragma once

#include <stdint.h>

#ifndef HG_SCREEN_SIZE
#define HG_SCREEN_SIZE 466
#endif

namespace display {

// Panel size in pixels (square, round). Face configs are laid out on a 466 px
// canvas and scaled to this.
constexpr int kSize = HG_SCREEN_SIZE;

// Scales a length from the 466 px design canvas to this panel.
constexpr int scaled(int designPx) { return designPx * kSize / 466; }

// Starts the panel and LVGL. Returns false if the panel did not respond.
bool begin();

// 0 (off) to 255.
void setBrightness(uint8_t level);

// Runs LVGL's timers and rendering. Call continuously from loop().
void loop();

// Screen refreshes completed during the last whole second.
uint32_t framesPerSecond();

}  // namespace display
