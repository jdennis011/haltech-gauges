#include "quick_menu.h"

#include <Arduino.h>

#include "display.h"
#include "font_manager.h"
#include "proto.h"

using display::scaled;
using hg::cfg::Font;

namespace quick_menu {

namespace {

const uint32_t kAutoCloseMs = 5000;
const uint8_t kMinBrightness = 10;

Handlers handlers = {};
lv_obj_t* panel = nullptr;
lv_obj_t* slider = nullptr;
lv_obj_t* thisButton = nullptr;
lv_obj_t* allButton = nullptr;
bool forAll = false;
uint32_t activityMs = 0;
bool openNow = false;

uint16_t fontPx(int designPx) {
    const int px = scaled(designPx);
    return uint16_t(px < 12 ? 12 : px);
}

void touched() { activityMs = millis(); }

lv_obj_t* makeButton(lv_obj_t* parent, const char* text, lv_event_cb_t cb) {
    lv_obj_t* b = lv_button_create(parent);
    lv_obj_set_style_bg_color(b, lv_color_hex(0x3A3A3C), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(0x0A84FF), LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(b, scaled(12), 0);
    lv_obj_set_style_pad_hor(b, scaled(18), 0);
    lv_obj_set_style_pad_ver(b, scaled(10), 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_t* l = lv_label_create(b);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, fontGet(Font::Sans, fontPx(20)), 0);
    lv_obj_set_style_text_color(l, lv_color_white(), 0);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, nullptr);
    return b;
}

void setScope(bool all) {
    forAll = all;
    if (all) {
        lv_obj_add_state(allButton, LV_STATE_CHECKED);
        lv_obj_remove_state(thisButton, LV_STATE_CHECKED);
    } else {
        lv_obj_add_state(thisButton, LV_STATE_CHECKED);
        lv_obj_remove_state(allButton, LV_STATE_CHECKED);
    }
}

void onSlider(lv_event_t* e) {
    touched();
    const lv_event_code_t code = lv_event_get_code(e);
    const uint8_t level = uint8_t(lv_slider_get_value(slider));
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (handlers.brightness) handlers.brightness(level, false);
    } else if (code == LV_EVENT_RELEASED) {
        if (handlers.brightness) handlers.brightness(level, true);
        if (forAll && handlers.sendAll) handlers.sendAll(uint8_t(hg::proto::Cmd::SetBrightness), level);
    }
}

void onScreenOff(lv_event_t*) {
    touched();
    if (forAll && handlers.sendAll) handlers.sendAll(uint8_t(hg::proto::Cmd::SetDisplay), 0);
    close();
    if (handlers.screenOff) handlers.screenOff();
}

void onThis(lv_event_t*) {
    touched();
    setScope(false);
}

void onAll(lv_event_t*) {
    touched();
    setScope(true);
}

void onPanelTouched(lv_event_t*) { touched(); }

}  // namespace

void begin(lv_obj_t* screen, const Handlers& h, uint8_t brightness) {
    handlers = h;
    const int w = display::kSize * 78 / 100, hgt = display::kSize * 44 / 100, pad = scaled(16);
    panel = lv_obj_create(screen);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, w, hgt);
    // The screen is round: a panel this wide only fits whole in the middle
    // band (its corners were off the glass when it hung from the top), so
    // it sits a little above centre.
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, -scaled(20));
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
    lv_obj_set_style_radius(panel, scaled(20), 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x48484A), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_pad_all(panel, pad, 0);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);  // a tap on it is not a tap outside
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_event_cb(panel, onPanelTouched, LV_EVENT_PRESSED, nullptr);

    lv_obj_t* title = lv_label_create(panel);
    lv_label_set_text(title, "Brightness");
    lv_obj_set_style_text_font(title, fontGet(Font::Sans, fontPx(20)), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x8E8E93), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    slider = lv_slider_create(panel);
    lv_obj_set_size(slider, w - 2 * pad, scaled(30));
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 0, scaled(46));
    // A drag on the slider is the slider's alone: it must not reach the
    // screen as a swipe and change the face underneath.
    lv_obj_remove_flag(slider, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(slider, LV_OBJ_FLAG_PRESS_LOCK);
    lv_slider_set_range(slider, kMinBrightness, 255);
    lv_slider_set_value(slider, brightness < kMinBrightness ? kMinBrightness : brightness, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x0A84FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, scaled(16), LV_PART_KNOB);  // a 62 px knob
    lv_obj_add_event_cb(slider, onSlider, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(slider, onSlider, LV_EVENT_RELEASED, nullptr);

    lv_obj_t* off = makeButton(panel, "Screen off", onScreenOff);
    lv_obj_align(off, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t* scope = lv_label_create(panel);
    lv_label_set_text(scope, "Apply to");
    lv_obj_set_style_text_font(scope, fontGet(Font::Sans, fontPx(16)), 0);
    lv_obj_set_style_text_color(scope, lv_color_hex(0x8E8E93), 0);
    lv_obj_align(scope, LV_ALIGN_BOTTOM_RIGHT, -scaled(150), -scaled(14));
    allButton = makeButton(panel, "all", onAll);
    lv_obj_align(allButton, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    thisButton = makeButton(panel, "this", onThis);
    lv_obj_align_to(thisButton, allButton, LV_ALIGN_OUT_LEFT_MID, -scaled(8), 0);
    setScope(false);

    lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
}

bool onGesture(lv_dir_t dir) {
    if (!panel) return false;
    if (openNow) {
        if (dir == LV_DIR_TOP) close();
        return true;  // no face change under an open menu
    }
    if (dir == LV_DIR_BOTTOM) {
        open();
        return true;
    }
    return false;
}

void open() {
    if (!panel) return;
    openNow = true;
    touched();
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(panel);
}

void close() {
    if (!panel) return;
    openNow = false;
    lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
}

bool isOpen() { return openNow; }

void setBrightness(uint8_t level) {
    if (slider) lv_slider_set_value(slider, level < kMinBrightness ? kMinBrightness : level, LV_ANIM_OFF);
}

void poll(uint32_t nowMs) {
    if (openNow && uint32_t(nowMs - activityMs) >= kAutoCloseMs) close();
}

}  // namespace quick_menu
