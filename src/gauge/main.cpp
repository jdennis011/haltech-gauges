#include <Arduino.h>

#include "audio.h"
#include "board.h"
#include "can_task.h"
#include "display.h"
#include "face_ui.h"
#include "serial_console.h"
#include "touch.h"

// LVGL's software renderer and the TTF rasteriser need more than the default 8 KB.
SET_LOOP_TASK_STACK_SIZE(16 * 1024);

// A build installed over Wi-Fi is confirmed by face_store::markHealthy() after
// ten seconds of running, not by the core at start, so a bad one is rolled back.
extern "C" bool verifyRollbackLater() { return true; }

void setup() {
    Serial.setRxBufferSize(8192);  // a config pasted into the console arrives faster than the loop runs
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);  // never block on USB CDC when no host is attached

    board::begin();
    Serial.printf("Gauge firmware on %s, %d px\n", board::name(), display::kSize);

    if (!display::begin()) Serial.println("Display start failed");
    if (!touch::begin()) Serial.println("Touch start failed");
    audio::begin();

    can_task::begin();
    face_ui::begin();
    serial_console::begin();
}

void loop() {
    board::update();
    face_ui::update();
    display::loop();
    serial_console::poll();
    audio::poll();

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
