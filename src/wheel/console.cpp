#include "console.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "buttons.h"
#include "link.h"
#include "version.h"
#include "wifi_fetch.h"

namespace console {

namespace {

std::string line;
bool rawMode = false;
uint32_t lastRawMs = 0;

void help() {
    Serial.println("Steering wheel module commands:");
    Serial.println("  raw                     print each line's millivolts ten times a second (any key stops)");
    Serial.println("  cal <line> <button>     the current reading of line 0.. is that button's centre");
    Serial.println("  cal tol <mV>            tolerance around every centre (default 80)");
    Serial.println("  cal show                the calibration as the hub sees it");
    Serial.println("  cal save                keep the calibration in the preferences");
    Serial.println("  cal clear               forget it");
    Serial.println("  info                    node, hub, buttons down, counters");
    Serial.println("  reboot");
    Serial.print("Button names:");
    for (size_t i = 0; i < buttons::kNames; i++) Serial.printf(" %s", buttons::kButtonNames[i]);
    Serial.println();
}

void info() {
    Serial.printf("wheel %d.%d.%d node %06lX, hub %s, calibration %s (CRC %08lX), buttons %04X%s\n", FW_VERSION_MAJOR,
                  FW_VERSION_MINOR, FW_VERSION_PATCH, (unsigned long)wheel_link::node(), wheel_link::hubPresent() ? "present" : "none",
                  buttons::calibrated() ? "stored" : "none", (unsigned long)buttons::calibrationCrc(),
                  unsigned(buttons::mask()), buttons::fault() ? ", a line reads out of range" : "");
    Serial.printf("frames in %lu, reports out %lu, wifi %s\n", (unsigned long)wheel_link::framesReceived(),
                  (unsigned long)wheel_link::inputsSent(), wifi_fetch::state());
}

void run(const std::string& cmd) {
    std::string error;
    if (cmd == "?" || cmd == "help") {
        help();
    } else if (cmd == "raw") {
        rawMode = true;
        Serial.println("millivolts per line; any key stops");
    } else if (cmd == "cal show") {
        Serial.println(buttons::calibrationJson().c_str());
    } else if (cmd == "cal save") {
        Serial.println(buttons::save() ? "saved" : "could not save");
    } else if (cmd == "cal clear") {
        buttons::clearCalibration();
        Serial.println("calibration cleared");
    } else if (cmd.rfind("cal tol ", 0) == 0) {
        const int mv = atoi(cmd.c_str() + 8);
        if (mv < 10 || mv > 1000) {
            Serial.println("tolerance is 10 to 1000 mV");
        } else {
            buttons::setTolerance(uint16_t(mv));
            Serial.printf("tolerance %d mV; 'cal save' to keep it\n", mv);
        }
    } else if (cmd.rfind("cal ", 0) == 0) {
        const char* rest = cmd.c_str() + 4;
        char* end = nullptr;
        const long lineIndex = strtol(rest, &end, 10);
        while (end && *end == ' ') end++;
        if (!end || end == rest || !*end) {
            Serial.println("cal <line> <button>");
        } else if (!buttons::capture(size_t(lineIndex), end, error)) {
            Serial.println(error.c_str());
        } else {
            Serial.printf("line %ld at %u mV is %s; 'cal save' to keep it\n", lineIndex,
                          unsigned(buttons::rawMv(size_t(lineIndex))), end);
        }
    } else if (cmd == "info") {
        info();
    } else if (cmd == "reboot") {
        wheel_link::requestReboot();
    } else if (!cmd.empty()) {
        Serial.println("? for the commands");
    }
}

}  // namespace

void begin() { line.reserve(128); }

void poll() {
    const uint32_t now = millis();
    if (rawMode && now - lastRawMs >= 100) {
        lastRawMs = now;
        Serial.print("raw");
        for (size_t i = 0; i < buttons::lineCount(); i++) Serial.printf(" %u", unsigned(buttons::rawMv(i)));
        Serial.printf("  mask %04X%s\n", unsigned(buttons::mask()), buttons::fault() ? " fault" : "");
    }
    while (Serial.available()) {
        const char c = char(Serial.read());
        if (rawMode) {
            rawMode = false;
            Serial.println("raw stopped");
            continue;
        }
        if (c == '\r' || c == '\n') {
            run(line);
            line.clear();
        } else if (line.size() < 120) {
            line += c;
        }
    }
}

}  // namespace console
