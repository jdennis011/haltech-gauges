#include "swc_output.h"

#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

#include "board_pins.h"
#include "proto.h"

using namespace hg::proto;

namespace swc_output {

namespace {

const char* kPrefs = "wheel";
const char* kKey = "swc";
// The media buttons, in kInput... bit order from bit 7.
const char* const kNames[7] = {"vol_up", "vol_down", "mode", "mute", "seek_up", "seek_down", "phone"};
const int kPins[kOutputs] = {PIN_SWC_0, PIN_SWC_1, PIN_SWC_2, PIN_SWC_3};

bool enabled = false;
int8_t map_[7] = {0, 1, -1, -1, 2, 3, -1};  // vol up/down and seek up/down on the four outputs
bool on[kOutputs] = {};
uint16_t maskNow = 0;

void apply() {
    bool next[kOutputs] = {};
    if (enabled) {
        for (int b = 0; b < 7; b++) {
            if (map_[b] >= 0 && map_[b] < kOutputs && (maskNow & (1u << (7 + b)))) next[map_[b]] = true;
        }
    }
    for (int i = 0; i < kOutputs; i++) {
        if (next[i] != on[i]) {
            on[i] = next[i];
            digitalWrite(kPins[i], on[i] ? HIGH : LOW);
        }
    }
}

void load() {
    Preferences p;
    if (!p.begin(kPrefs, true)) return;
    const String text = p.getString(kKey, "");
    p.end();
    if (!text.length()) return;
    JsonDocument doc;
    if (deserializeJson(doc, text) != DeserializationError::Ok) return;
    enabled = doc["enabled"] | false;
    for (int b = 0; b < 7; b++) {
        if (!doc["map"][kNames[b]].isNull()) map_[b] = int8_t(doc["map"][kNames[b]].as<int>());
    }
}

void save() {
    JsonDocument doc;
    doc["enabled"] = enabled;
    JsonObject m = doc["map"].to<JsonObject>();
    for (int b = 0; b < 7; b++) m[kNames[b]] = map_[b];
    String text;
    serializeJson(doc, text);
    Preferences p;
    if (!p.begin(kPrefs, false)) return;
    p.putString(kKey, text);
    p.end();
}

}  // namespace

void begin() {
    for (int pin : kPins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
    load();
}

void poll(uint32_t) {}

void set(uint16_t mediaMask) {
    maskNow = uint16_t(mediaMask & kInputMediaMask);
    apply();
}

void statusJson(JsonObject out) {
    out["enabled"] = enabled;
    JsonArray outputs = out["outputs"].to<JsonArray>();
    for (int i = 0; i < kOutputs; i++) {
        JsonObject o = outputs.add<JsonObject>();
        o["index"] = i;
        o["on"] = on[i];
        for (int b = 0; b < 7; b++) {
            if (map_[b] == i) o["button"] = kNames[b];
        }
    }
    JsonObject m = out["map"].to<JsonObject>();
    for (int b = 0; b < 7; b++) m[kNames[b]] = map_[b];
}

bool setSettings(JsonVariantConst in, std::string& error) {
    if (!in["enabled"].isNull()) enabled = in["enabled"].as<bool>();
    JsonObjectConst m = in["map"].as<JsonObjectConst>();
    if (!m.isNull()) {
        for (int b = 0; b < 7; b++) {
            JsonVariantConst v = m[kNames[b]];
            if (v.isNull()) continue;
            const int idx = v.as<int>();
            if (idx < -1 || idx >= kOutputs) {
                error = "an output is -1 (none) or 0 to 3";
                return false;
            }
            map_[b] = int8_t(idx);
        }
    }
    save();
    apply();
    return true;
}

}  // namespace swc_output
