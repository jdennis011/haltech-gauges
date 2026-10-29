#pragma once

#include <stdint.h>

// The boot splash: assets/splash/oagauge.svg, compiled in as splash_art.h and
// played with LVGL from the moment the panel is up until the key-on intro
// starts (docs/startup-animation.md, "Boot splash"). The wordmark's letters
// rise and fade in one after another, the tag line fades in under them, and a
// short arc circles the rim while the gauge waits for the hub.
namespace splash {

// Creates the splash on the active screen and turns the panel on at the saved
// brightness. Call right after display::begin(), before the face is built.
void begin();

// Keeps the splash above a face that was just (re)built.
void raise();

// Advances the animation; the entrance starts at the first call. About 30 Hz.
void update(uint32_t nowMs);

// The intro's hand-over: 0 shows the splash, 1 has the lettering and the arc
// faded into the black. The panel is then dimmed, end() called, and the face
// brought up by the intro's brightness ramp as it always was.
void setFade(float fade);

// Removes the splash. Safe to call when there is none.
void end();

// True from begin() until end().
bool active();

}  // namespace splash
