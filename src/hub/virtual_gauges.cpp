#include "virtual_gauges.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "library_store.h"

namespace virtual_gauges {

namespace {

const char* kFile = "/virtual.json";

SemaphoreHandle_t mtx = nullptr;
std::vector<Entry> entries;

struct Guard {
    Guard() { xSemaphoreTakeRecursive(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGiveRecursive(mtx); }
};

std::string syntheticMac(uint8_t id) {
    char buf[18];
    snprintf(buf, sizeof(buf), "02:00:00:00:00:%02X", id);
    return buf;
}

std::string upper(const char* s) {
    std::string out(s ? s : "");
    for (char& c : out) c = char(toupper(uint8_t(c)));
    return out;
}

bool validMac(const std::string& mac) {
    if (mac.size() != 17) return false;
    for (size_t i = 0; i < 17; i++) {
        if (i % 3 == 2) {
            if (mac[i] != ':') return false;
        } else if (!isxdigit(uint8_t(mac[i]))) {
            return false;
        }
    }
    return true;
}

std::string cleanLabel(const char* label) {
    std::string out(label ? label : "");
    if (out.size() > kMaxLabel) out.resize(kMaxLabel);
    return out;
}

Entry* find(uint8_t id) {
    for (Entry& e : entries) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

void save() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const Entry& e : entries) {
        JsonObject o = arr.add<JsonObject>();
        o["id"] = e.id;
        o["label"] = e.label;
        o["mac"] = e.mac;
        o["face"] = e.face;
    }
    File f = LittleFS.open(kFile, "w");
    if (!f) return;
    serializeJson(doc, f);
    f.close();
}

void load() {
    File f = LittleFS.open(kFile, "r");
    if (!f) return;
    JsonDocument doc;
    const bool bad = deserializeJson(doc, f) != DeserializationError::Ok;
    f.close();
    if (bad) return;
    for (JsonObject o : doc.as<JsonArray>()) {
        if (entries.size() >= kMax) break;
        Entry e;
        e.id = o["id"] | 0;
        if (e.id == 0 || find(e.id)) continue;
        e.label = cleanLabel(o["label"] | "");
        e.mac = upper(o["mac"] | "");
        if (!validMac(e.mac)) e.mac = syntheticMac(e.id);
        e.face = o["face"] | 0;
        entries.push_back(e);
    }
}

}  // namespace

bool begin() {
    mtx = xSemaphoreCreateRecursiveMutex();
    Guard guard;
    load();
    return true;
}

size_t snapshot(Entry* out, size_t cap) {
    Guard guard;
    size_t n = 0;
    for (const Entry& e : entries) {
        if (n >= cap) break;
        out[n++] = e;
    }
    return n;
}

void listJson(JsonArray out) {
    Guard guard;
    for (const Entry& e : entries) {
        JsonObject o = out.add<JsonObject>();
        o["id"] = e.id;
        o["label"] = e.label;
        o["mac"] = e.mac;
        o["face"] = e.face;
    }
}

bool add(const char* label, uint8_t& idOut) {
    Guard guard;
    if (entries.size() >= kMax) return false;
    uint8_t id = 1;
    while (find(id)) id++;
    Entry e;
    e.id = id;
    e.label = cleanLabel(label);
    if (e.label.empty()) e.label = "Gauge " + std::to_string(id);
    e.mac = syntheticMac(id);
    entries.push_back(e);
    save();
    idOut = id;
    return true;
}

bool setLabel(uint8_t id, const char* label) {
    Guard guard;
    Entry* e = find(id);
    if (!e) return false;
    e->label = cleanLabel(label);
    if (e->label.empty()) e->label = "Gauge " + std::to_string(id);
    save();
    return true;
}

bool setFace(uint8_t id, uint8_t face) {
    Guard guard;
    Entry* e = find(id);
    if (!e) return false;
    e->face = face;
    save();
    return true;
}

bool setMac(uint8_t id, const char* mac) {
    Guard guard;
    Entry* e = find(id);
    if (!e) return false;
    std::string next = upper(mac);
    if (next.empty()) next = syntheticMac(id);
    if (!validMac(next)) return false;
    if (next == e->mac) return true;
    for (const Entry& other : entries) {
        if (other.id != id && other.mac == next) return false;  // already linked elsewhere
    }
    // The assignment follows the gauge: whichever of the two keys has one wins,
    // preferring the new (real) MAC's own assignment if it already has one.
    std::string name;
    if (!library_store::assignmentFor(next.c_str(), name) &&
        library_store::assignmentFor(e->mac.c_str(), name)) {
        library_store::assign(next.c_str(), name.c_str());
    }
    library_store::assign(e->mac.c_str(), "");
    e->mac = next;
    save();
    return true;
}

bool remove(uint8_t id) {
    Guard guard;
    for (size_t i = 0; i < entries.size(); i++) {
        if (entries[i].id != id) continue;
        // A synthetic MAC means nothing once the entry is gone.
        if (entries[i].mac == syntheticMac(id)) library_store::assign(entries[i].mac.c_str(), "");
        entries.erase(entries.begin() + i);
        save();
        return true;
    }
    return false;
}

bool findByMac(const char* mac, Entry& out) {
    const std::string key = upper(mac);
    Guard guard;
    for (const Entry& e : entries) {
        if (e.mac == key) {
            out = e;
            return true;
        }
    }
    return false;
}

}  // namespace virtual_gauges
