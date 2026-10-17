#pragma once

#include <lvgl.h>
#include <stdint.h>

// The pull-down menu (docs/gauge-quick-menu.md): swipe down from the top of
// any face and a panel slides over it with a brightness slider, a screen-off
// button, and whether the change is for this gauge or for all of them (which
// goes to the hub). Swipe up, tap outside it, or wait five seconds to close it.
namespace quick_menu {

struct Handlers {
    // The slider moved: apply this brightness to the panel now.
    void (*brightness)(uint8_t level, bool final);
    // The screen-off button.
    void (*screenOff)();
    // "All gauges": ask the hub to apply the same to every gauge.
    void (*sendAll)(uint8_t cmd, uint8_t arg);
};

// Builds the panel, hidden, on the screen.
void begin(lv_obj_t* screen, const Handlers& handlers, uint8_t brightness);
// A swipe on the screen: down opens, up closes. True if it was for the menu.
bool onGesture(lv_dir_t dir);
void open();
void close();
bool isOpen();
// The brightness changed elsewhere (the hub): keep the slider in step.
void setBrightness(uint8_t level);
// Closes it after a while without a touch.
void poll(uint32_t nowMs);

}  // namespace quick_menu
