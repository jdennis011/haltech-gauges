#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

// Gauges planned in the web UI before the hardware exists. Each gets a
// synthetic MAC (02:00:00:00:00:<id>, a locally administered address no real
// board can have) so configs can be assigned to it like any other gauge.
// Entering the real MAC later carries the label and assignment across.
namespace virtual_gauges {

constexpr size_t kMax = 8;
constexpr size_t kMaxLabel = 24;

struct Entry {
    uint8_t id = 0;
    std::string label;
    std::string mac;  // synthetic until linked to hardware
    uint8_t face = 0;
};

// Loads the list from LittleFS (library_store::begin() must have mounted it).
bool begin();

// Copies the entries into out; returns how many.
size_t snapshot(Entry* out, size_t cap);
void listJson(JsonArray out);

bool add(const char* label, uint8_t& idOut);
bool setLabel(uint8_t id, const char* label);
bool setFace(uint8_t id, uint8_t face);
// Links the gauge to a real MAC; "" goes back to its synthetic one. The
// config assignment moves with it. False if the MAC is malformed or unknown id.
bool setMac(uint8_t id, const char* mac);
bool remove(uint8_t id);

// The planned gauge, if any, whose MAC (real or synthetic) is this one.
bool findByMac(const char* mac, Entry& out);

}  // namespace virtual_gauges
