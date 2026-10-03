#include "display.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "board_pins.h"

namespace display {

namespace {

// A quarter of the screen per render pass. Must be an even number of lines.
const int kBufferLines = 116;

Arduino_DataBus* bus = new Arduino_ESP32QSPI(PIN_LCD_CS, PIN_LCD_SCLK, PIN_LCD_D0, PIN_LCD_D1,
                                             PIN_LCD_D2, PIN_LCD_D3);
// The panel's visible area starts 6 columns into the controller's memory.
Arduino_CO5300* gfx = new Arduino_CO5300(bus, PIN_LCD_RST, 0, kSize, kSize, 6, 0, 0, 0);

lv_display_t* disp = nullptr;
uint32_t refreshes = 0;
uint32_t lastFps = 0;
uint32_t fpsWindowStart = 0;

void flushCb(lv_display_t* d, const lv_area_t* area, uint8_t* pixels) {
    gfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t*>(pixels),
                            lv_area_get_width(area), lv_area_get_height(area));
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

bool begin() {
    if (!gfx->begin()) return false;
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
