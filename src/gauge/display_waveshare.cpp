#include "display.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "board_pins.h"

namespace display {

namespace {

// Lines rendered per pass, an even number. 40 lines is 37 KB, which fits in
// internal RAM next to Wi-Fi; a quarter screen (108 KB) did not leave room
// for it. Most frames redraw only a needle's box, so passes stay few.
const int kBufferLines = 40;

Arduino_DataBus* bus = new Arduino_ESP32QSPI(PIN_LCD_CS, PIN_LCD_SCLK, PIN_LCD_D0, PIN_LCD_D1,
                                             PIN_LCD_D2, PIN_LCD_D3);
// The panel's visible area starts 6 columns into the controller's memory.
Arduino_CO5300* gfx = new Arduino_CO5300(bus, PIN_LCD_RST, 0, kSize, kSize, 6, 0, 0, 0);

lv_display_t* disp = nullptr;
uint32_t refreshes = 0;
uint32_t lastFps = 0;
uint32_t fpsWindowStart = 0;
uint32_t flushUs = 0, lvglUs = 0, lastFlushMs = 0, lastLvglMs = 0;

void flushCb(lv_display_t* d, const lv_area_t* area, uint8_t* pixels) {
    const uint32_t t0 = micros();
    gfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t*>(pixels),
                            lv_area_get_width(area), lv_area_get_height(area));
    flushUs += micros() - t0;
    lv_display_flush_ready(d);
}

// The CO5300 only accepts windows that start on even coordinates and have
// even sizes, so every redrawn area is widened to even boundaries.
void roundAreaCb(lv_event_t* e) {
    lv_area_t* area = static_cast<lv_area_t*>(lv_event_get_param(e));
    area->x1 &= ~1;
    area->y1 &= ~1;
    area->x2 |= 1;
    area->y2 |= 1;
}

void refreshDoneCb(lv_event_t*) { refreshes++; }

uint32_t tickCb() { return millis(); }

}  // namespace

// QSPI clock. The CO5300 and the ESP32-S3 both run the bus at 80 MHz; at the
// library's 40 MHz default a full frame took 50 ms to push.
const int32_t kQspiHz = 80000000;

bool begin() {
    if (!gfx->begin(kQspiHz)) return false;
    gfx->fillScreen(0x0000);
    gfx->setBrightness(200);

    lv_init();
    lv_tick_set_cb(tickCb);

    const size_t bufferBytes = size_t(kSize) * kBufferLines * sizeof(uint16_t);
    void* buffer = heap_caps_aligned_alloc(4, bufferBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!buffer) buffer = heap_caps_aligned_alloc(4, bufferBytes, MALLOC_CAP_SPIRAM);
    if (!buffer) return false;

    disp = lv_display_create(kSize, kSize);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, flushCb);
    lv_display_set_buffers(disp, buffer, nullptr, bufferBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_add_event_cb(disp, roundAreaCb, LV_EVENT_INVALIDATE_AREA, nullptr);
    lv_display_add_event_cb(disp, refreshDoneCb, LV_EVENT_REFR_READY, nullptr);

    fpsWindowStart = millis();
    return true;
}

void setBrightness(uint8_t level) { gfx->setBrightness(level); }

void loop() {
    const uint32_t t0 = micros();
    lv_timer_handler();
    lvglUs += micros() - t0;

    const uint32_t now = millis();
    if (now - fpsWindowStart >= 1000) {
        lastFps = refreshes;
        refreshes = 0;
        lastFlushMs = flushUs / 1000;
        lastLvglMs = lvglUs / 1000;
        flushUs = lvglUs = 0;
        fpsWindowStart = now;
    }
}

uint32_t framesPerSecond() { return lastFps; }
uint32_t flushMsPerSecond() { return lastFlushMs; }
uint32_t lvglMsPerSecond() { return lastLvglMs; }

}  // namespace display
