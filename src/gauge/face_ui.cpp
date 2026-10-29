#include "face_ui.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "can_task.h"
#include "display.h"
#include "face_renderer.h"
#include "face_store.h"
#include "font_manager.h"
#include "ip_link.h"
#include "proto.h"
#include "quick_menu.h"
#include "splash.h"

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
uint8_t lastBrightness = 200;  // this gauge's own, kept in flash
bool screenOff = false;
bool wakeSwallow = false;  // the touch that woke the screen is not a tap or a swipe
const char* kPrefs = "gauge";
const char* kBrightKey = "bright";

void saveBrightness() {
    Preferences p;
    p.begin(kPrefs, false);
    p.putUChar(kBrightKey, lastBrightness);
    p.end();
}
char alertShown[33] = {};
uint32_t alertColour = 0;

// The key-on intro (docs/startup-animation.md): dark until the hub's intro
// frame, or this gauge's own if no hub speaks up, then the style, then live.
enum class IntroState : uint8_t { Waiting, Delay, Playing, Done };
const uint32_t kSoloAfterBootMs = 3000;  // no beacon at all: play alone
const uint32_t kSoloAfterHubMs = 2000;   // beacons but no intro frame: play alone
IntroState introState = IntroState::Waiting;
bool handover = false;  // the splash was up when the intro started
hg::proto::IntroStyle introStyle = hg::proto::IntroStyle::Sweep;
uint32_t bootMs = 0;
uint32_t introStartMs = 0;
uint32_t introDurationMs = 1200;
// What the hub last asked for, kept in the preferences: a gauge that plays
// alone (no hub yet, or none at all) plays the same intro as last time.
hg::proto::IntroStyle soloStyle = hg::proto::IntroStyle::Sweep;
uint16_t soloDurationMs = 1200;
const char* kIntroStyleKey = "intro_style";
const char* kIntroDurKey = "intro_dur";
bool hubSeen = false;
uint32_t hubSeenMs = 0;

bool introDone() { return introState == IntroState::Done; }

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
    splash::raise();
}

void showFace(int index) {
    const int count = int(face_store::faceCount());
    if (count <= 1) return;
    index = ((index % count) + count) % count;
    face_store::setActiveFace(uint8_t(index));
    rebuild();
}

void wakeScreen() {
    screenOff = false;
    display::setBrightness(lastBrightness);
}

// The menu's handlers: brightness live, screen off, and "all gauges" to the hub.
void menuBrightness(uint8_t level, bool final) {
    lastBrightness = level;
    if (!screenOff) display::setBrightness(level);
    if (final) saveBrightness();
}
void menuScreenOff() {
    screenOff = true;
    display::setBrightness(0);
}
void menuSendAll(uint8_t cmd, uint8_t arg) { can_task::requestAll(hg::proto::Cmd(cmd), arg); }
void menuResetHold() { face_renderer::resetHold(); }

// A touch on a dark screen wakes it and does nothing else.
void onPressed(lv_event_t*) {
    if (!screenOff) return;
    wakeScreen();
    wakeSwallow = true;
    lv_indev_wait_release(lv_indev_active());
}

void onClicked(lv_event_t*) {
    if (wakeSwallow) {
        wakeSwallow = false;
        return;
    }
    if (quick_menu::isOpen()) quick_menu::close();  // a tap outside the panel
}

void onGesture(lv_event_t*) {
    const lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (wakeSwallow) {
        wakeSwallow = false;
        lv_indev_wait_release(lv_indev_active());
        return;
    }
    if (quick_menu::onGesture(dir)) {
        lv_indev_wait_release(lv_indev_active());
        return;
    }
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

    // The hub's alert message, drawn as the page draws it: a dark band across
    // the middle, a line of the alert's colour above and below, the message
    // in that colour in the condensed face.
    alertBand = lv_obj_create(screen);
    lv_obj_remove_style_all(alertBand);
    lv_obj_set_size(alertBand, display::kSize, scaled(120));
    lv_obj_align(alertBand, LV_ALIGN_TOP_MID, 0, scaled(173));
    lv_obj_set_style_bg_color(alertBand, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(alertBand, lv_opa_t(217), 0);  // 85%
    lv_obj_set_style_border_width(alertBand, scaled(5), 0);
    lv_obj_set_style_border_side(alertBand, lv_border_side_t(LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM), 0);
    lv_obj_set_style_border_opa(alertBand, LV_OPA_COVER, 0);
    lv_obj_remove_flag(alertBand, LV_OBJ_FLAG_CLICKABLE);
    alertText = lv_label_create(alertBand);
    lv_obj_set_style_text_font(alertText, fontGet(Font::Condensed, fontPx(56)), 0);
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

void updateOverlays(uint32_t now) {
    const uint32_t identifyUntil = can_task::identifyUntilMs();
    const bool showIdentify = identifyUntil != 0 && int32_t(identifyUntil - now) > 0;
    if (showIdentify == lv_obj_has_flag(identify, LV_OBJ_FLAG_HIDDEN)) {
        if (showIdentify) {
            lv_label_set_text_fmt(identify, "%06lX", (unsigned long)can_task::nodeId());
            lv_obj_remove_flag(identify, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(identify);
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
            lv_obj_set_style_border_color(alertBand, lv_color_hex(colour), 0);
            lv_obj_set_style_text_color(alertText, lv_color_hex(colour), 0);
            // A long message shrinks to fit the band, as on the page (410 of 466 px).
            uint16_t px = fontPx(56);
            const lv_font_t* f = fontGet(Font::Condensed, px);
            lv_point_t size;
            lv_text_get_size(&size, text, f, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
            const int32_t maxWidth = scaled(410);
            if (size.x > maxWidth) {
                px = uint16_t(int32_t(px) * maxWidth / size.x);
                if (px < 18) px = 18;
                f = fontGet(Font::Condensed, px);
            }
            lv_obj_set_style_text_font(alertText, f, 0);
            lv_label_set_text(alertText, text);
            lv_obj_remove_flag(alertBand, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(alertBand);  // above whatever the face built since
        } else {
            alertShown[0] = 0;
            lv_obj_add_flag(alertBand, LV_OBJ_FLAG_HIDDEN);
        }
    }

    const char* status = face_renderer::stale() ? "NO DATA" : (face_renderer::demo() ? "DEMO" : "");
    static std::string lastStatus;
    std::string next = status;
    if (face_store::trial()) next += next.empty() ? "TRIAL" : "  TRIAL";
    if (face_store::fallback()) next += next.empty() ? "BUILT-IN FACE" : "  BUILT-IN FACE";
    if (next != lastStatus) {
        lastStatus = next;
        lv_label_set_text(note, next.c_str());
    }
}

const char* introStyleName(hg::proto::IntroStyle s) {
    return s == hg::proto::IntroStyle::Chase ? "chase" : s == hg::proto::IntroStyle::Fade ? "fade" : "sweep";
}

void startIntro(const hg::proto::Intro& i, uint32_t now, bool fromHub) {
    introStyle = i.style;
    introDurationMs = i.durationMs < 200 ? 200 : i.durationMs;
    introStartMs = now + i.delayMs;
    introState = IntroState::Delay;
    Serial.printf("intro: %s, %lu ms%s\n", introStyleName(introStyle), (unsigned long)introDurationMs,
                  fromHub ? "" : " (alone)");
    if (fromHub && (i.style != soloStyle || uint16_t(introDurationMs) != soloDurationMs)) {
        soloStyle = i.style;
        soloDurationMs = uint16_t(introDurationMs);
        Preferences p;
        p.begin(kPrefs, false);
        p.putUChar(kIntroStyleKey, uint8_t(soloStyle));
        p.putUShort(kIntroDurKey, soloDurationMs);
        p.end();
    }
}

void runIntro(uint32_t now) {
    hg::proto::Intro frame;
    if (can_task::takeIntro(frame)) {
        // Once per start; the page's Play forces another.
        if (introState != IntroState::Done || frame.force) startIntro(frame, now, true);
    }
    if (!hubSeen && can_task::hubPresent()) {
        hubSeen = true;
        hubSeenMs = now;
    }
    switch (introState) {
        case IntroState::Waiting:
            if ((hubSeen && now - hubSeenMs >= kSoloAfterHubMs) || (!hubSeen && now - bootMs >= kSoloAfterBootMs)) {
                hg::proto::Intro solo;
                solo.style = soloStyle;
                solo.durationMs = soloDurationMs;
                startIntro(solo, now, false);
            }
            break;
        case IntroState::Delay:
            if (int32_t(now - introStartMs) >= 0) {
                introState = IntroState::Playing;
                handover = splash::active();
                if (introStyle != hg::proto::IntroStyle::Fade) face_renderer::setIntro(introStyle, 0);
            }
            break;
        case IntroState::Playing: {
            const float phase = float(now - introStartMs) / float(introDurationMs);
            if (phase >= 1) {
                introState = IntroState::Done;
                face_renderer::endIntro();
                splash::end();
                if (!screenOff) display::setBrightness(lastBrightness);
                break;
            }
            if (introStyle != hg::proto::IntroStyle::Fade) face_renderer::setIntro(introStyle, phase);
            // The sweep styles light up over the first quarter; fade takes the whole duration.
            float ramp = introStyle == hg::proto::IntroStyle::Fade ? phase : (phase < 0.25f ? phase / 0.25f : 1.0f);
            if (handover) {
                // The splash fades into its black over the first half of that window,
                // then the panel goes dark for the one redraw that removes it and the
                // face comes up over the second half as it always has.
                if (ramp < 0.5f) {
                    splash::setFade(ramp * 2);
                    break;
                }
                if (splash::active()) {
                    display::setBrightness(0);
                    splash::end();
                }
                ramp = (ramp - 0.5f) * 2;
            }
            if (!screenOff) display::setBrightness(uint8_t(lastBrightness * ramp));
            break;
        }
        case IntroState::Done:
            break;
    }
}

}  // namespace

void begin() {
    screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(screen, onGesture, LV_EVENT_GESTURE, nullptr);
    lv_obj_add_event_cb(screen, onPressed, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(screen, onClicked, LV_EVENT_CLICKED, nullptr);
    {
        Preferences p;
        p.begin(kPrefs, true);
        lastBrightness = p.getUChar(kBrightKey, 200);
        const uint8_t style = p.getUChar(kIntroStyleKey, uint8_t(hg::proto::IntroStyle::Sweep));
        if (style <= uint8_t(hg::proto::IntroStyle::Fade)) soloStyle = hg::proto::IntroStyle(style);
        soloDurationMs = p.getUShort(kIntroDurKey, 1200);
        if (soloDurationMs < 200) soloDurationMs = 200;
        p.end();
    }
    makeOverlays();
    quick_menu::begin(screen, {menuBrightness, menuScreenOff, menuSendAll, menuResetHold}, lastBrightness);
    // Dark until the intro (the hub's, or this gauge's own if no hub speaks up),
    // unless the splash is showing; rebuild() keeps it above the face.
    if (!splash::active()) display::setBrightness(0);
    rebuild();
    startedMs = millis();
    bootMs = startedMs;
}

void update() {
    const uint32_t now = millis();

    if (face_store::takeChange()) {
        face_renderer::resetHold();  // a new config starts with no peaks
        rebuild();
    }

    const int face = can_task::takeFaceRequest();
    if (face >= 0) showFace(face);

    // Brightness and screen commands before the intro are kept for after it.
    const int brightness = can_task::takeBrightnessRequest();
    if (brightness >= 0) {
        lastBrightness = uint8_t(brightness);
        if (!screenOff && (introDone() || splash::active())) display::setBrightness(lastBrightness);
        quick_menu::setBrightness(lastBrightness);
        saveBrightness();
    }
    if (can_task::takeHoldReset()) face_renderer::resetHold();
    const int displayOn = can_task::takeDisplayRequest();
    if (displayOn == 0 && !screenOff) {
        screenOff = true;
        display::setBrightness(0);
        quick_menu::close();
    } else if (displayOn == 1 && screenOff) {
        screenOff = false;
        if (introDone() || splash::active()) display::setBrightness(lastBrightness);
    }

    if (now - lastUpdateMs >= 33) {
        lastUpdateMs = now;
        runIntro(now);
        splash::update(now);
        // A dark screen is not drawn; the overlays keep their state for when it wakes.
        if (!screenOff) face_renderer::update(now);
        quick_menu::poll(now);
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
        Serial.printf("fps %u (flush %u ms/s, lvgl %u ms/s), face %u/%u, %s, hub %d, frames %u, link %s, internal heap %u (min %u), psram %u\n",
                      unsigned(display::framesPerSecond()), unsigned(display::flushMsPerSecond()),
                      unsigned(display::lvglMsPerSecond()), unsigned(face_store::activeFace() + 1),
                      unsigned(face_store::faceCount()),
                      face_renderer::stale() ? "NO DATA" : (face_renderer::demo() ? "DEMO" : "LIVE"),
                      int(can_task::hubPresent()), unsigned(can_task::framesReceived()), ip_link::status(),
                      unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)),
                      unsigned(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL)),
                      unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)));
    }
}

}  // namespace face_ui
