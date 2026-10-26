#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "hub_manager.h"

// The steering wheel module on the hub (docs/steering-wheel.md): its button
// reports become the horn relay, the cruise frames on the vehicle CAN and
// the head unit's wired remote, with the fail-safe in lib/hg_wheel. Settings
// live in the hub's preferences; the wheel's calibration is kept here too
// and pushed to a wheel whose copy differs.
namespace wheel {

// First thing in setup(): the horn output is made safe before anything else runs.
void begin();
// HubManager handler callbacks.
void onInput(const hg::proto::HubManager::Gauge& gauge, const hg::proto::Input& input, void* ctx);
void onInputRaw(const hg::proto::HubManager::Gauge& gauge, const hg::proto::InputRaw& raw, void* ctx);
// From the gauge-bus task, about once a millisecond, under hub::lock().
void poll(uint32_t nowMs);

// Changes whenever the page's view of the wheel should refresh.
uint32_t version();
// {present, online, node, mac, fw, link_lost, fail_safe_releases, seq_gaps, mask,
//  buttons:[{name, down, changed_s}], raw:[mV...], flags, outputs:{horn, cruise, media},
//  settings:{horn, cruise, media}, calibration:{stored, crc, in_sync, json}, fg:{...}}
void statusJson(JsonObject out);
// The Outputs toggles: {horn, cruise, media} as booleans; any may be missing.
bool setSettings(JsonVariantConst in, std::string& error);
// The wheel's calibration table (the page's or a file's); stored and pushed.
bool setCalibration(const uint8_t* json, size_t len, std::string& error);
// A timed test of one output: "horn", a cruise button or a media button by
// name (the kInput... names such as "cruise_set", "vol_up"), for `ms`.
bool test(const char* what, uint32_t ms, std::string& error);
// Asks the wheel to stream raw readings for `seconds` (0 stops).
bool requestRaw(uint8_t seconds, std::string& error);

}  // namespace wheel
