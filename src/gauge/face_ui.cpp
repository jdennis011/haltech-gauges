#include "face_ui.h"

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "can_task.h"
#include "display.h"
#include "face_renderer.h"
#include "face_store.h"
#include "font_manager.h"

using hg::cfg::Font;

namespace face_ui {

namespace {

using display::scaled;

lv_obj_t* screen = nullptr;
lv_obj_t* identify = nullptr;
lv_obj_t* alertBand = nullptr;
lv_obj_t* alertText = nullptr;
lv_obj_t* note = nullptr;  // "NO DATA" and the like, small, at the bottom

uint32_t lastUpdateMs = 0;
uint32_t lastReportMs = 0;
uint32_t startedMs = 0;
bool healthy = false;
uint8_t lastBrightness = 200;
bool screenOff = false;
char alertShown[33] = {};
uint32_t alertColour = 0;

uint16_t fontPx(int designPx) {
    const int px = scaled(designPx);
    return uint16_t(px < 12 ? 12 : px);
}

void rebuild() {
    const hg::cfg::Config* cfg = face_store::current();
    if (!cfg || cfg->faces.empty()) return;
    face_renderer::build(screen, *cfg, face_store::activeFace());
    // The overlays stay above the face.
    lv_obj_move_foreground(note);
    lv_obj_move_foreground(alertBand);
    lv_obj_move_foreground(identify);
}

void showFace(int index) {
    const int count = int(face_store::faceCount());
    if (count <= 1) return;
    index = ((index % count) + count) % count;
    face_store::setActiveFace(uint8_t(index));
    rebuild();
}

void onGesture(lv_event_t*) {
    const lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_LEFT) {
        showFace(int(face_store::activeFace()) + 1);
    } else if (dir == LV_DIR_RIGHT) {
        showFace(int(face_store::activeFace()) - 1);
    } else {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
}

void makeOverlays() {
    note = lv_label_create(screen);
    lv_obj_set_style_text_font(note, fontGet(Font::Sans, fontPx(18)), 0);
    lv_obj_set_style_text_color(note, lv_color_hex(0x8E8E93), 0);
    lv_obj_align(note, LV_ALIGN_BOTTOM_MID, 0, -scaled(26));
    lv_label_set_text(note, "");

    // The hub's alert message: a band across the top in the alert's colour.
    alertBand = lv_obj_create(screen);
    lv_obj_remove_style_all(alertBand);
    lv_obj_set_size(alertBand, display::kSize, scaled(64));
    lv_obj_align(alertBand, LV_ALIGN_TOP_MID, 0, scaled(60));
    lv_obj_set_style_bg_opa(alertBand, LV_OPA_COVER, 0);
    lv_obj_remove_flag(alertBand, LV_OBJ_FLAG_CLICKABLE);
    alertText = lv_label_create(alertBand);
    lv_obj_set_style_text_font(alertText, fontGet(Font::Display, fontPx(28)), 0);
    lv_obj_set_style_text_color(alertText, lv_color_black(), 0);
    lv_obj_center(alertText);
    lv_obj_add_flag(alertBand, LV_OBJ_FLAG_HIDDEN);

    identify = lv_label_create(screen);
    lv_obj_set_style_text_font(identify, fontGet(Font::Display, fontPx(56)), 0);
    lv_obj_set_style_text_color(identify, lv_color_black(), 0);
    lv_obj_set_style_bg_color(identify, lv_color_hex(0xFFCC00), 0);
    lv_obj_set_style_bg_opa(identify, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(identify, scaled(18), 0);
    lv_obj_set_style_radius(identify, scaled(16), 0);
    lv_obj_center(identify);
    lv_obj_add_flag(identify, LV_OBJ_FLAG_HIDDEN);
}

// Black text reads on a light band, white on a dark one.
lv_color_t textOn(uint32_t rgb) {
    const uint32_t r = (rgb >> 16) & 255, g = (rgb >> 8) & 255, b = rgb & 255;
    return (r * 299 + g * 587 + b * 114) / 1000 > 140 ? lv_color_black() : lv_color_white();
}

void updateOverlays(uint32_t now) {
    const uint32_t identifyUntil = can_task::identifyUntilMs();
    const bool showIdentify = identifyUntil != 0 && int32_t(identifyUntil - now) > 0;
    if (showIdentify == lv_obj_has_flag(identify, LV_OBJ_FLAG_HIDDEN)) {
        if (showIdentify) {
            lv_label_set_text_fmt(identify, "%06lX", (unsigned long)can_task::nodeId());
            lv_obj_remove_flag(identify, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(identify, LV_OBJ_FLAG_HIDDEN);
        }
    }

    char text[33];
    uint32_t colour = 0;
    const bool show = can_task::alertMessage(text, colour);
    if (show != !lv_obj_has_flag(alertBand, LV_OBJ_FLAG_HIDDEN) || (show && (strcmp(text, alertShown) != 0 || colour != alertColour))) {
        if (show) {
            strncpy(alertShown, text, sizeof(alertShown) - 1);
            alertColour = colour;
            lv_obj_set_style_bg_color(alertBand, lv_color_hex(colour), 0);
            lv_obj_set_style_text_color(alertText, textOn(colour), 0);
            lv_label_set_text(alertText, text);
            lv_obj_remove_flag(alertBand, LV_OBJ_FLAG_HIDDEN);
        } else {
            alertShown[0] = 0;
            lv_obj_add_flag(alertBand, LV_OBJ_FLAG_HIDDEN);
        }
    }

    const char* status = face_renderer::stale() ? "NO DATA" : (face_renderer::demo() ? "DEMO" : "");
    static std::string lastStatus;
    std::string next = status;
    if (face_store::trying()) next += next.empty() ? "TRIAL" : "  TRIAL";
    if (face_store::fallback()) next += next.empty() ? "BUILT-IN FACE" : "  BUILT-IN FACE";
    if (next != lastStatus) {
        lastStatus = next;
        lv_label_set_text(note, next.c_str());
    }
}

}  // namespace

void begin() {
    screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(screen, onGesture, LV_EVENT_GESTURE, nullptr);
    makeOverlays();
    rebuild();
    startedMs = millis();
}

void update() {
    const uint32_t now = millis();

    if (face_store::takeChange()) rebuild();

    const int face = can_task::takeFaceRequest();
    if (face >= 0) showFace(face);

    const int brightness = can_task::takeBrightnessRequest();
    if (brightness >= 0) {
        lastBrightness = uint8_t(brightness);
        if (!screenOff) display::setBrightness(lastBrightness);
    }
    const int displayOn = can_task::takeDisplayRequest();
    if (displayOn == 0 && !screenOff) {
        screenOff = true;
        display::setBrightness(0);
    } else if (displayOn == 1 && screenOff) {
        screenOff = false;
        display::setBrightness(lastBrightness);
    }

    if (now - lastUpdateMs >= 33) {
        lastUpdateMs = now;
        face_renderer::update(now);
        updateOverlays(now);
        can_task::setDemo(face_renderer::demo());
    }

    // Ten seconds of running clears the restart counter that guards against
    // a config that crashes the renderer.
    if (!healthy && now - startedMs >= 10000) {
        healthy = true;
        face_store::markHealthy();
    }

    if (now - lastReportMs >= 5000) {
        lastReportMs = now;
        Serial.printf("fps %u, face %u/%u, %s, hub %d, can frames %u, internal heap %u (min %u), psram %u\n",
                      unsigned(display::framesPerSecond()), unsigned(face_store::activeFace() + 1),
                      unsigned(face_store::faceCount()),
                      face_renderer::stale() ? "NO DATA" : (face_renderer::demo() ? "DEMO" : "CAN"),
                      int(can_task::hubPresent()), unsigned(can_task::framesReceived()),
                      unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)),
                      unsigned(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL)),
                      unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)));
    }
}

}  // namespace face_ui
