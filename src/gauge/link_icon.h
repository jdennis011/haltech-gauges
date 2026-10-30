#pragma once

#include <lvgl.h>

#include "can_task.h"

// A small icon for the link the live data is arriving on (docs/gauge-quick-menu.md):
// a Wi-Fi fan, or a plug for the CAN chain. Drawn with LVGL's own shapes, so
// there is nothing to rasterise. The quick menu shows it all the time; the
// face shows it for a few seconds whenever the link changes.
namespace link_icon {

struct Icon {
    lv_obj_t* root = nullptr;  // sizePx square, transparent, not clickable
    lv_obj_t* wifi = nullptr;
    lv_obj_t* can = nullptr;
    can_task::LiveLink shown = can_task::LiveLink::None;
};

Icon create(lv_obj_t* parent, int sizePx, lv_color_t color);
// Shows the shape for `link`; None hides both (the root stays where it is).
void set(Icon& icon, can_task::LiveLink link);
// "CAN", "Wi-Fi" or "no data".
const char* name(can_task::LiveLink link);

}  // namespace link_icon
