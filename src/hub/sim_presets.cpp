#include "sim_presets.h"

#include <LittleFS.h>
#include <ctype.h>
#include <string.h>

#include "haltech.h"
#include "hub_state.h"
#include "simulator.h"

namespace sim_presets {

namespace {

const char* kFile = "/sim-presets.json";

void load(JsonDocument& doc) {
    File f = LittleFS.open(kFile, "r");
    if (!f || deserializeJson(doc, f) != DeserializationError::Ok || !doc.is<JsonObject>()) doc.to<JsonObject>();
    if (f) f.close();
}

bool store(JsonDocument& doc) {
    File f = LittleFS.open(kFile, "w");
    if (!f) return false;
    serializeJson(doc, f);
    f.close();
    return true;
}

JsonArray presets(JsonDocument& doc) {
    JsonArray a = doc["presets"].as<JsonArray>();
    if (a.isNull()) a = doc["presets"].to<JsonArray>();
    return a;
}

int indexOf(JsonArray a, const char* name) {
    int i = 0;
    for (JsonObject p : a) {
        if (strcmp(p["name"] | "", name) == 0) return i;
        i++;
    }
    return -1;
}

// An override as {channel, hold} or {channel, min, max}, from either form,
// checked here so a bad file is refused at import rather than at Load.
bool addOverride(JsonArray out, const char* channel, JsonVariantConst v, std::string& error) {
    if (!channel || !*channel || v.isNull()) return true;
    if (hg::findChannel(channel) < 0) {
        error = std::string("unknown channel '") + channel + "'";
        return false;
    }
    const bool hold = !v["hold"].isNull();
    if (!hold && (v["min"].isNull() || v["max"].isNull())) {
        error = std::string("the override for '") + channel + "' needs hold, or min and max";
        return false;
    }
    if (out.size() >= 16) {
        error = "at most 16 overrides";
        return false;
    }
    JsonObject o = out.add<JsonObject>();
    o["channel"] = channel;
    if (hold) {
        o["hold"] = v["hold"].as<float>();
    } else {
        o["min"] = v["min"].as<float>();
        o["max"] = v["max"].as<float>();
    }
    return true;
}

}  // namespace

bool validName(const char* name) {
    if (!name || !*name || strlen(name) > 24) return false;
    for (const char* c = name; *c; c++) {
        if (!isalnum((unsigned char)*c) && *c != ' ' && *c != '-' && *c != '_' && *c != '.') return false;
    }
    return true;
}

void listJson(JsonArray out) {
    JsonDocument doc;
    load(doc);
    for (JsonObjectConst p : presets(doc)) out.add(p);
}

bool save(const char* name, JsonVariantConst body, std::string& error) {
    if (!validName(name)) {
        error = "a preset name is 1 to 24 letters, digits, spaces, dots, - or _";
        return false;
    }
    JsonDocument doc;
    load(doc);
    JsonArray a = presets(doc);
    const int i = indexOf(a, name);
    if (i < 0 && a.size() >= kMax) {
        error = "at most 16 presets";
        return false;
    }
    JsonObject p = i >= 0 ? a[i].as<JsonObject>() : a.add<JsonObject>();
    p.clear();
    p["name"] = name;
    JsonArray overrides = p["overrides"].to<JsonArray>();
    const bool fromBody = body.is<JsonObjectConst>() && (!body["speed"].isNull() || !body["overrides"].isNull());
    if (fromBody) {
        p["speed"] = body["speed"] | 1.0f;
        p["paused"] = body["paused"] | false;
        JsonVariantConst ov = body["overrides"];
        if (ov.is<JsonArrayConst>()) {
            for (JsonObjectConst o : ov.as<JsonArrayConst>()) {
                if (!addOverride(overrides, o["channel"] | "", o, error)) return false;
            }
        } else if (ov.is<JsonObjectConst>()) {
            for (JsonPairConst kv : ov.as<JsonObjectConst>()) {
                if (!addOverride(overrides, kv.key().c_str(), kv.value(), error)) return false;
            }
        }
    } else {
        hub::Lock lock;
        p["speed"] = simulator::speed();
        p["paused"] = simulator::paused();
        simulator::overridesJson(overrides);
    }
    if (!store(doc)) {
        error = "could not write the presets file";
        return false;
    }
    return true;
}

bool apply(const char* name, std::string& error) {
    JsonDocument doc;
    load(doc);
    JsonArray a = presets(doc);
    const int i = indexOf(a, name);
    if (i < 0) {
        error = "no such preset";
        return false;
    }
    JsonObject p = a[i];
    JsonDocument cmd;
    cmd["speed"] = p["speed"] | 1.0f;
    cmd["paused"] = p["paused"] | false;
    cmd["clear"] = true;
    cmd["overrides"] = p["overrides"];
    hub::Lock lock;
    return simulator::applyJson(cmd.as<JsonVariantConst>(), error);
}

bool remove(const char* name) {
    JsonDocument doc;
    load(doc);
    JsonArray a = presets(doc);
    const int i = indexOf(a, name);
    if (i < 0) return false;
    a.remove(size_t(i));
    return store(doc);
}

}  // namespace sim_presets
