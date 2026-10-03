#include "perf_screen.h"

#include <Arduino.h>
#include <lvgl.h>
#include <math.h>

#include "can_task.h"
#include "display.h"
#include "font_manager.h"

using hg::cfg::Font;

namespace perf_screen {

namespace {

using display::scaled;

const int kMaxRpm = 8000;
const int kNeedleLength = scaled(165);

// Font sizes scale with the panel but never below a readable minimum.
uint16_t fontPx(int designPx) {
    const int px = scaled(designPx);
    return uint16_t(px < 12 ? 12 : px);
}

lv_obj_t* scale = nullptr;
lv_obj_t* needle = nullptr;
lv_obj_t* ring = nullptr;
lv_obj_t* number = nullptr;
lv_obj_t* stats = nullptr;
lv_obj_t* identify = nullptr;

uint32_t lastNeedleMs = 0;
uint32_t lastNumberMs = 0;
uint32_t lastStatsMs = 0;
bool everLive = false;  // CAN data has been seen at least once

}  // namespace

void create() {
    lv_obj_t* screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    ring = lv_arc_create(screen);
    lv_obj_set_size(ring, display::kSize, display::kSize);
    lv_obj_center(ring);
    lv_arc_set_rotation(ring, 135);
    lv_arc_set_bg_angles(ring, 0, 270);
    lv_arc_set_range(ring, 0, 100);
    lv_obj_remove_style(ring, nullptr, LV_PART_KNOB);
    lv_obj_remove_flag(ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(ring, scaled(10), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ring, scaled(10), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ring, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_arc_color(ring, lv_color_hex(0x39FF88), LV_PART_INDICATOR);

    scale = lv_scale_create(screen);
    lv_obj_set_size(scale, scaled(420), scaled(420));
    lv_obj_center(scale);
    lv_obj_set_style_bg_opa(scale, LV_OPA_TRANSP, 0);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(scale, 0, kMaxRpm);
    lv_scale_set_angle_range(scale, 270);
    lv_scale_set_rotation(scale, 135);
    lv_scale_set_total_tick_count(scale, 41);
    lv_scale_set_major_tick_every(scale, 5);
    lv_scale_set_label_show(scale, true);
    static const char* labels[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", nullptr};
    lv_scale_set_text_src(scale, labels);

    lv_obj_set_style_arc_width(scale, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(scale, fontGet(Font::Condensed, fontPx(34)), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(scale, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_line_color(scale, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_line_width(scale, scaled(4), LV_PART_INDICATOR);
    lv_obj_set_style_length(scale, scaled(22), LV_PART_INDICATOR);
    lv_obj_set_style_line_color(scale, lv_color_hex(0x8E8E93), LV_PART_ITEMS);
    lv_obj_set_style_line_width(scale, 2, LV_PART_ITEMS);
    lv_obj_set_style_length(scale, scaled(10), LV_PART_ITEMS);

    needle = lv_line_create(scale);
    lv_obj_set_style_line_width(needle, scaled(6), 0);
    lv_obj_set_style_line_rounded(needle, true, 0);
    lv_obj_set_style_line_color(needle, lv_color_hex(0xFF3B30), 0);

    number = lv_label_create(screen);
    lv_obj_set_style_text_font(number, fontGet(Font::Digital, fontPx(72)), 0);
    lv_obj_set_style_text_color(number, lv_color_white(), 0);
    lv_obj_align(number, LV_ALIGN_CENTER, 0, scaled(95));
    lv_label_set_text(number, "0");

    stats = lv_label_create(screen);
    lv_obj_set_style_text_font(stats, fontGet(Font::Mono, fontPx(20)), 0);
    lv_obj_set_style_text_color(stats, lv_color_hex(0xFFCC00), 0);
    lv_obj_set_style_text_align(stats, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(stats, LV_ALIGN_CENTER, 0, scaled(-70));
    lv_label_set_text(stats, "-- fps");

    // Shown when the hub asks this gauge to identify itself.
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

void update() {
    const uint32_t now = millis();

    // Live RPM and throttle from the bus once any have been seen; before that,
    // a synthetic 0 -> 8000 -> 0 sweep every 4 seconds.
    float rpm = 0;
    float ringPercent = 0;
    const bool live = can_task::channels().get(hg::CH_rpm, now, rpm);
    if (live) {
        everLive = true;
        can_task::channels().get(hg::CH_throttle_position, now, ringPercent);
    } else if (!everLive) {
        const float sweep = 0.5f - 0.5f * cosf(float(now % 4000) * (2.0f * float(M_PI) / 4000.0f));
        rpm = sweep * kMaxRpm;
        ringPercent = sweep * 100.0f;
    }
    const bool stale = everLive && !live;

    if (now - lastNeedleMs >= 16) {
        lastNeedleMs = now;
        lv_scale_set_line_needle_value(scale, needle, kNeedleLength, int(rpm));
        lv_arc_set_value(ring, int(ringPercent));
    }
    if (now - lastNumberMs >= 50) {
        lastNumberMs = now;
        if (stale) {
            lv_label_set_text(number, "---");
        } else {
            lv_label_set_text_fmt(number, "%d", int(rpm));
        }
    }
    if (now - lastStatsMs >= 1000) {
        lastStatsMs = now;
        const uint32_t fps = display::framesPerSecond();
        const char* source = live ? "CAN" : (everLive ? "NO DATA" : "DEMO");
        lv_label_set_text_fmt(stats, "%u fps\n%s%s", unsigned(fps), source,
                              can_task::hubPresent() ? " +HUB" : "");
        Serial.printf("fps %u, source %s, hub %d, can frames %u, internal heap free %u (min %u), "
                      "psram free %u\n",
                      unsigned(fps), source, int(can_task::hubPresent()),
                      unsigned(can_task::framesReceived()),
                      unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)),
                      unsigned(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL)),
                      unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)));
    }

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
}

}  // namespace perf_screen
