#include <Arduino.h>

#include "board.h"
#include "can_task.h"
#include "display.h"
#include "perf_screen.h"
#include "touch.h"

// LVGL's software renderer and the TTF rasteriser need more than the default 8 KB.
SET_LOOP_TASK_STACK_SIZE(16 * 1024);

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);  // never block on USB CDC when no host is attached

    board::begin();
    Serial.printf("Gauge firmware on %s, %d px\n", board::name(), display::kSize);

    if (!display::begin()) Serial.println("Display start failed");
    if (!touch::begin()) Serial.println("Touch start failed");

    can_task::begin();
    perf_screen::create();
}

void loop() {
    board::update();
    perf_screen::update();
    display::loop();

    const int brightness = can_task::takeBrightnessRequest();
    if (brightness >= 0) display::setBrightness(uint8_t(brightness));

    static uint32_t lastPowerMs = 0;
    if (millis() - lastPowerMs >= 1000) {
        lastPowerMs = millis();
        can_task::setVbusMillivolts(board::vbusMillivolts());
    }

    if (can_task::rebootRequested()) {
        delay(200);  // let the acknowledgement go out first
        ESP.restart();
    }

    delay(1);
}
