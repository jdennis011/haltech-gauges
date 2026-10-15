// Updates the ESP32-C6 Wi-Fi module on a Waveshare ESP32-P4-WIFI6 to the
// ESP-Hosted slave firmware the Arduino core was built against, over the SDIO
// link, with the firmware file embedded in this image (platformio.ini,
// env hub_p4_c6update). The board ships with an older slave firmware that
// does not answer the core's version request, which leaves Wi-Fi unusable
// (electrical-spec.md 5.4). Flash this once, watch the serial output, then
// flash the hub again.
//
// The core's ESP_HostedOTA library does the same thing but fetches the file
// over HTTPS, which needs the Wi-Fi that is not working yet.

#include <Arduino.h>
#include <WiFi.h>
#include <esp32-hal-hosted.h>

// objcopy names the embedded file after its path from the project directory.
extern const uint8_t slaveStart[] asm("_binary_tools_hosted_esp32c6_v2_12_13_bin_start");
extern const uint8_t slaveEnd[] asm("_binary_tools_hosted_esp32c6_v2_12_13_bin_end");

namespace {

const size_t kChunk = 4096;

void report() {
    uint32_t major, minor, patch;
    hostedGetHostVersion(&major, &minor, &patch);
    Serial.printf("Host expects slave firmware %lu.%lu.%lu\n", (unsigned long)major, (unsigned long)minor,
                  (unsigned long)patch);
    hostedGetSlaveVersion(&major, &minor, &patch);
    Serial.printf("Slave reports %lu.%lu.%lu (0.0.0 = it did not answer)\n", (unsigned long)major,
                  (unsigned long)minor, (unsigned long)patch);
}

bool update() {
    const size_t size = size_t(slaveEnd - slaveStart);
    Serial.printf("Embedded slave firmware: %u bytes\n", unsigned(size));
    if (!hostedBeginUpdate()) {
        Serial.println("ota begin failed: the slave does not take updates over SDIO. Use the 4-pin header.");
        return false;
    }
    for (size_t pos = 0; pos < size; pos += kChunk) {
        const size_t n = size - pos < kChunk ? size - pos : kChunk;
        if (!hostedWriteUpdate(const_cast<uint8_t*>(slaveStart + pos), uint32_t(n))) {
            Serial.printf("ota write failed at %u\n", unsigned(pos));
            return false;
        }
        if ((pos / kChunk) % 32 == 0) Serial.printf("  %u%%\n", unsigned(pos * 100 / size));
    }
    if (!hostedEndUpdate()) {
        Serial.println("ota end failed: the slave refused the image");
        return false;
    }
    Serial.println("Written; activating");
    if (!hostedActivateUpdate()) Serial.println("activate reported failure (older slaves do; it may still have taken)");
    return true;
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("\nESP32-P4-WIFI6: ESP32-C6 slave firmware update");
    // Brings the SDIO link and ESP-Hosted up.
    WiFi.mode(WIFI_STA);
    delay(500);
    if (!hostedIsInitialized()) {
        Serial.println("ESP-Hosted did not initialise: no SDIO link to the C6");
        return;
    }
    report();
    hostedHasUpdate();  // logs the comparison
    const bool ok = update();
    Serial.println(ok ? "Update done. Power-cycle the board, then flash the hub firmware." : "Update failed.");
    if (ok) {
        delay(3000);
        Serial.println("Restarting to re-check the slave version");
        ESP.restart();
    }
}

void loop() {
    static uint32_t last = 0;
    if (millis() - last > 5000) {
        last = millis();
        report();
    }
    delay(10);
}
