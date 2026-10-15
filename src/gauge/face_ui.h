#pragma once

#include <stdint.h>

// The gauge's screen: the active face of the current config, swiped left and
// right between faces, with the hub's identify request and alert message
// drawn over it. Replaces perf_screen.
namespace face_ui {

// After display::begin(), touch::begin() and can_task::begin().
void begin();

// Call from loop(): picks up new configs, commands from the hub, and polls
// the values.
void update();

}  // namespace face_ui
