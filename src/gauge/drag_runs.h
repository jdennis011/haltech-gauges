#pragma once

#include <stdint.h>

#include "drag_timer.h"

// The drag timers behind the dragtimer widgets (docs/motion.md): one per run
// type the config uses, fed every vehicle speed sample off the bus and the
// accelerometer, running whatever face is shown so a swipe mid-run loses
// nothing. A finished run is kept for the widget, the best of the session
// beside it, and sent to the hub.
namespace drag_runs {

constexpr size_t kRunCount = hg::motion::kRunCount;

// The run types the config's faces use, a bit per hg::motion::Run, with the
// rollout setting of each. Called after a config is built.
void setRuns(uint8_t mask, const bool rollout[kRunCount]);

// From the UI loop. With `demo` (no live data has ever arrived) a made-up
// quarter mile is replayed instead of the bus, and nothing is remembered or sent.
void poll(uint32_t nowMs, bool demo);

// True while some run type is in use (poll() does nothing otherwise).
bool active();

// The timer for a run type, or null if no face uses it.
const hg::motion::DragTimer* timer(hg::motion::Run run);

struct Memory {
    bool haveLast = false;
    hg::motion::Result last;
    bool haveBest = false;
    uint32_t bestMs = 0;  // the quickest complete run
};
Memory memory(hg::motion::Run run);

// Reset peaks: forgets every result and best.
void reset();

}  // namespace drag_runs
