#include <Arduino.h>
#include <M5Dial.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "display.h"

namespace display {

namespace {

// A quarter of the screen per render pass. The StampS3 has no PSRAM, so this
// comes out of internal RAM (240 x 60 x 2 = 28.8 KB).
const int kBufferLines = kSize / 4;

lv_display_t* disp = nullptr;
uint32_t refreshes = 0;
uint32_t lastFps = 0;
uint32_t fpsWindowStart = 0;

void flushCb(lv_display_t* d, const lv_area_t* area, uint8_t* pixels) {
    M5Dial.Display.startWrite();
    // The typed overload tells M5GFX the pixels are little-endian RGB565, as LVGL renders them.
    M5Dial.Display.pushImage(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area),
                             reinterpret_cast<const lgfx::rgb565_t*>(pixels));
    M5Dial.Display.endWrite();
    lv_display_flush_ready(d);
}

void refreshDoneCb(lv_event_t*) { refreshes++; }

uint32_t tickCb() { return millis(); }

}  // namespace

bool begin() {
    M5Dial.Display.setBrightness(200);
    M5Dial.Display.fillScreen(TFT_BLACK);

    lv_init();
    lv_tick_set_cb(tickCb);

    const size_t bufferBytes = size_t(kSize) * kBufferLines * sizeof(uint16_t);
    void* buffer = heap_caps_aligned_alloc(4, bufferBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!buffer) return false;

    disp = lv_display_create(kSize, kSize);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, flushCb);
    lv_display_set_buffers(disp, buffer, nullptr, bufferBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_add_event_cb(disp, refreshDoneCb, LV_EVENT_REFR_READY, nullptr);

    fpsWindowStart = millis();
    return true;
}

void setBrightness(uint8_t level) { M5Dial.Display.setBrightness(level); }

void loop() {
    lv_timer_handler();

    const uint32_t now = millis();
    if (now - fpsWindowStart >= 1000) {
        lastFps = refreshes;
        refreshes = 0;
        fpsWindowStart = now;
    }
}

uint32_t framesPerSecond() { return lastFps; }

}  // namespace display
