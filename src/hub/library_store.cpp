#include "library_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "crc32.h"
#include "face_model.h"

using namespace hg;
using proto::HubManager;

namespace library_store {

namespace {

const char* kDir = "/cfg";
const char* kTempFile = "/cfg/.tmp.json";
const char* kAssignFile = "/assign.json";
const size_t kMaxName = 32;

struct Entry {
    std::string name;
    std::vector<uint8_t> data;
    uint32_t crc = 0;
    uint8_t schema = cfg::kSchemaVersion;
};

struct Assignment {
    std::string mac;
    std::string name;
};

SemaphoreHandle_t mtx = nullptr;
std::vector<Entry*> cache;    // configs that are assigned to a gauge
std::vector<Entry*> retired;  // replaced copies, freed by collectGarbage()
std::vector<Assignment> assignments;
bool mounted = false;

// Recursive, because the public functions call one another.
struct Guard {
    Guard() { xSemaphoreTakeRecursive(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGiveRecursive(mtx); }
};

std::string pathFor(const char* name) { return std::string(kDir) + "/" + name + ".json"; }

void macString(const uint8_t mac[6], char* out, size_t cap) {
    snprintf(out, cap, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4],
             mac[5]);
}

std::string upper(const char* s) {
    std::string out(s ? s : "");
    for (char& c : out) c = char(toupper(uint8_t(c)));
    return out;
}

bool readFile(const char* path, std::vector<uint8_t>& out) {
    File f = LittleFS.open(path, "r");
    if (!f || f.isDirectory()) return false;
    out.resize(f.size());
    const size_t got = out.empty() ? 0 : f.read(out.data(), out.size());
    f.close();
    return got == out.size();
}

uint8_t schemaOf(const uint8_t* data, size_t size) {
    JsonDocument filter;
    filter["schema"] = true;
    JsonDocument doc;
    if (deserializeJson(doc, reinterpret_cast<const char*>(data), size,
                        DeserializationOption::Filter(filter))) {
        return cfg::kSchemaVersion;
    }
    return uint8_t(doc["schema"] | int(cfg::kSchemaVersion));
}

Entry* findCached(const char* name) {
    for (Entry* e : cache) {
        if (e->name == name) return e;
    }
    return nullptr;
}

void retire(const char* name) {
    for (size_t i = 0; i < cache.size(); i++) {
        if (cache[i]->name == name) {
            retired.push_back(cache[i]);
            cache.erase(cache.begin() + i);
            return;
        }
    }
}

bool loadIntoCache(const char* name) {
    std::vector<uint8_t> data;
    if (!readFile(pathFor(name).c_str(), data)) return false;
    Entry* e = new Entry;
    e->name = name;
    e->crc = crc32(data.data(), data.size());
    e->schema = schemaOf(data.data(), data.size());
    e->data = std::move(data);
    retire(name);
    cache.push_back(e);
    return true;
}

bool isAssigned(const char* name) {
    for (const Assignment& a : assignments) {
        if (a.name == name) return true;
    }
    return false;
}

void saveAssignments() {
    JsonDocument doc;
    JsonObject o = doc.to<JsonObject>();
    for (const Assignment& a : assignments) o[a.mac] = a.name;
    File f = LittleFS.open(kAssignFile, "w");
    if (!f) return;
    serializeJson(doc, f);
    f.close();
}

void loadAssignments() {
    File f = LittleFS.open(kAssignFile, "r");
    if (!f) return;
    JsonDocument doc;
    const bool bad = deserializeJson(doc, f) != DeserializationError::Ok;
    f.close();
    if (bad) return;
    for (JsonPair kv : doc.as<JsonObject>()) {
        const char* name = kv.value().as<const char*>();
        if (name && name[0]) assignments.push_back({upper(kv.key().c_str()), name});
    }
}

}  // namespace

bool begin() {
    mtx = xSemaphoreCreateRecursiveMutex();
    mounted = LittleFS.begin(true);
    if (!mounted) return false;
    if (!LittleFS.exists(kDir)) LittleFS.mkdir(kDir);
    Guard guard;
    loadAssignments();
    for (const Assignment& a : assignments) loadIntoCache(a.name.c_str());
    return true;
}

bool validName(const char* name) {
    if (!name) return false;
    const size_t len = strlen(name);
    if (len == 0 || len > kMaxName) return false;
    for (size_t i = 0; i < len; i++) {
        const char c = name[i];
        if (!isalnum(uint8_t(c)) && c != '-' && c != '_') return false;
    }
    return true;
}

bool exists(const char* name) {
    return mounted && validName(name) && LittleFS.exists(pathFor(name).c_str());
}

std::string filePath(const char* name) { return pathFor(name); }

bool put(const char* name, const uint8_t* data, size_t size, std::string& error) {
    if (!validName(name)) {
        error = "name must be 1-32 characters: letters, digits, - and _";
        return false;
    }
    if (!mounted) {
        error = "storage not mounted";
        return false;
    }
    cfg::Config parsed;
    const cfg::ParseResult result = cfg::parseConfig(data, size, parsed);
    if (!result.ok) {
        error = result.error;
        return false;
    }

    File f = LittleFS.open(kTempFile, "w");
    if (!f) {
        error = "cannot write to storage";
        return false;
    }
    const size_t wrote = f.write(data, size);
    f.close();
    if (wrote != size) {
        LittleFS.remove(kTempFile);
        error = "storage full";
        return false;
    }
    const std::string path = pathFor(name);
    LittleFS.remove(path.c_str());
    if (!LittleFS.rename(kTempFile, path.c_str())) {
        error = "could not replace the stored file";
        return false;
    }

    Guard guard;
    if (isAssigned(name)) loadIntoCache(name);  // the old copy is retired, not freed
    return true;
}

bool remove(const char* name) {
    if (!exists(name)) return false;
    LittleFS.remove(pathFor(name).c_str());
    Guard guard;
    retire(name);
    bool changed = false;
    for (size_t i = assignments.size(); i-- > 0;) {
        if (assignments[i].name == name) {
            assignments.erase(assignments.begin() + i);
            changed = true;
        }
    }
    if (changed) saveAssignments();
    return true;
}

bool read(const char* name, std::string& out) {
    if (!exists(name)) return false;
    std::vector<uint8_t> data;
    if (!readFile(pathFor(name).c_str(), data)) return false;
    out.assign(data.begin(), data.end());
    return true;
}

void listJson(JsonArray out) {
    if (!mounted) return;
    File dir = LittleFS.open(kDir);
    if (!dir) return;
    JsonDocument filter;
    filter["name"] = true;
    filter["faces"][0]["name"] = true;
    for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
        if (f.isDirectory()) continue;
        String base = f.name();
        const int slash = base.lastIndexOf('/');
        if (slash >= 0) base = base.substring(slash + 1);
        if (base.startsWith(".") || !base.endsWith(".json")) continue;
        base.remove(base.length() - 5);

        std::vector<uint8_t> data(f.size());
        const size_t got = data.empty() ? 0 : f.read(data.data(), data.size());
        if (got != data.size()) continue;

        JsonObject o = out.add<JsonObject>();
        o["name"] = base;
        o["size"] = data.size();
        char crc[12];
        snprintf(crc, sizeof(crc), "%08lX", (unsigned long)crc32(data.data(), data.size()));
        o["crc"] = crc;
        JsonDocument doc;
        if (!deserializeJson(doc, reinterpret_cast<const char*>(data.data()), data.size(),
                             DeserializationOption::Filter(filter))) {
            o["title"] = doc["name"] | "";
            o["faces"] = doc["faces"].size();
        }
    }
}

bool assign(const char* mac, const char* name) {
    if (!mac || !mac[0]) return false;
    const bool clearing = !name || !name[0];
    if (!clearing && !exists(name)) return false;
    const std::string key = upper(mac);

    Guard guard;
    bool found = false;
    for (size_t i = 0; i < assignments.size() && !found; i++) {
        if (assignments[i].mac != key) continue;
        found = true;
        if (clearing) {
            assignments.erase(assignments.begin() + i);
        } else {
            assignments[i].name = name;
        }
    }
    if (!found && !clearing) assignments.push_back({key, name});
    if (!clearing && !findCached(name)) loadIntoCache(name);
    saveAssignments();
    return true;
}

bool assignmentFor(const uint8_t mac[6], char* out, size_t cap) {
    char key[18];
    macString(mac, key, sizeof(key));
    Guard guard;
    for (const Assignment& a : assignments) {
        if (a.mac == key) {
            snprintf(out, cap, "%s", a.name.c_str());
            return true;
        }
    }
    return false;
}

bool assignmentFor(const char* mac, std::string& out) {
    const std::string key = upper(mac);
    Guard guard;
    for (const Assignment& a : assignments) {
        if (a.mac == key) {
            out = a.name;
            return true;
        }
    }
    return false;
}

void assignmentsJson(JsonObject out) {
    Guard guard;
    for (const Assignment& a : assignments) out[a.mac] = a.name;
}

bool desired(const HubManager::Gauge& gauge, HubManager::Desired& out, void*) {
    if (!gauge.haveHello) return false;
    char name[kMaxName + 1];
    if (!assignmentFor(gauge.mac, name, sizeof(name))) return false;
    Guard guard;
    const Entry* e = findCached(name);
    if (!e) return false;
    out.data = e->data.data();
    out.size = uint32_t(e->data.size());
    out.crc = e->crc;
    out.schema = e->schema;
    return true;
}

void collectGarbage() {
    Guard guard;
    for (Entry* e : retired) delete e;
    retired.clear();
}

}  // namespace library_store
