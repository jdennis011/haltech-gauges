// The steering wheel module (docs/steering-wheel.md): reads the wheel's
// buttons and reports them to the hub over the gauge chain.
#include <Arduino.h>
#include <esp_ota_ops.h>

#include "board_pins.h"
#include "buttons.h"
#include "console.h"
#include "link.h"
#include "version.h"
#include "wifi_fetch.h"

// A build installed over Wi-Fi is confirmed below after ten seconds of
// running with the hub heard, not by the core at start, so a bad one is
// rolled back by the bootloader.
extern "C" bool verifyRollbackLater() { return true; }

namespace {

uint32_t startedMs = 0;
bool confirmed = false;

void led(uint32_t now) {
    bool on;
    if (wheel_link::identifying()) {
        on = (now / 100) % 2 == 0;  // fast blink
    } else if (wheel_link::hubPresent()) {
        on = true;
    } else {
        on = (now / 1000) % 2 == 0;  // slow blink: no hub heard
    }
    digitalWrite(PIN_LED, on ? HIGH : LOW);
}

}  // namespace

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);  // never block on USB CDC when no host is attached
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);

    // BOOT held through power-up for three seconds: start afresh.
    uint32_t held = 0;
    while (digitalRead(PIN_BOOT_BUTTON) == LOW && held < 3000) {
        delay(10);
        held += 10;
    }
    buttons::begin();
    if (held >= 3000) {
        buttons::clearCalibration();
        Serial.println("BOOT held: calibration cleared");
    }
    Serial.printf("Steering wheel module %d.%d.%d\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);

    wifi_fetch::begin();
    wheel_link::begin();
    console::begin();
    startedMs = millis();
}

void loop() {
    const uint32_t now = millis();
    console::poll();
    led(now);

    if (!confirmed && wheel_link::hubPresent() && now - startedMs >= 10000) {
        confirmed = true;
        esp_ota_mark_app_valid_cancel_rollback();
    }
    if (wheel_link::rebootRequested()) {
        delay(200);  // let the acknowledgement go out first
        ESP.restart();
    }
    delay(5);
}
