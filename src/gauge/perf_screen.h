#pragma once

// Bring-up step 2: a hard-coded tachometer driven by a synthetic sweep, used
// to measure what the panel and LVGL can do before the real faces exist. It
// shows a needle dial, an outer ring, a large TTF number updated at 20 Hz and
// the measured refresh rate.

namespace perf_screen {

void create();
void update();

}  // namespace perf_screen
