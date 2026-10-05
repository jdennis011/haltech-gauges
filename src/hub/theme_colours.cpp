#include "theme_colours.h"

#include <Arduino.h>
#include <LittleFS.h>

#include <atomic>
#include <vector>

#include "face_model.h"
#include "hub_state.h"

using namespace hg;
using proto::HubManager;

namespace theme_colours {

namespace {

const char* kFile = "/theme-colours.json";

static_assert(proto::kThemeColourCount == cfg::kMaxThemeColours, "the bus carries every slot");

cfg::ThemeColours colours;
std::atomic<uint32_t> changes{0};
uint32_t sentVersion = 0xFFFFFFFF;  // nothing sent yet
uint32_t lastSentMs = 0;
uint32_t lastTryMs = 0;
const uint32_t kRetryMs = 100;

void values(uint32_t out[proto::kThemeColourCount]) {
    for (size_t i = 0; i < proto::kThemeColourCount; i++) out[i] = colours.value[i];
}

void write(JsonArray out) {
    for (size_t i = 0; i < colours.count; i++) {
        JsonObject o = out.add<JsonObject>();
        char hex[8];
        cfg::formatColor(colours.value[i], hex);
        o["value"] = hex;
        o["name"] = colours.name[i];
    }
}

}  // namespace

void begin() {
    File f = LittleFS.open(kFile, "r");
    if (!f) return;  // the defaults
    std::vector<uint8_t> text(f.size());
    const size_t got = f.read(text.data(), text.size());
    f.close();
    const cfg::ParseResult result = cfg::parseThemeColours(text.data(), got, colours);
    if (!result.ok) Serial.printf("Stored theme colours ignored: %s\n", result.error.c_str());
}

void poll(uint32_t nowMs) {
    const uint32_t version = changes;
    const bool due = version != sentVersion || uint32_t(nowMs - lastSentMs) >= proto::kThemeColourRepeatMs;
    if (!due || uint32_t(nowMs - lastTryMs) < kRetryMs) return;
    lastTryMs = nowMs;
    HubManager& m = hub::manager();
    uint32_t v[proto::kThemeColourCount];
    values(v);
    // With no gauge listening there is nobody to tell: the next one to come
    // online gets them from onOnline(). A busy bus is tried again shortly.
    if (!m.anyOnline() || m.themeColours(proto::kBroadcastNode, v)) {
        sentVersion = version;
        lastSentMs = nowMs;
    }
}

void onOnline(const HubManager::Gauge& gauge, void*) {
    uint32_t v[proto::kThemeColourCount];
    values(v);
    hub::manager().themeColours(gauge.node, v);
}

bool set(const uint8_t* data, size_t len, std::string& error) {
    cfg::ThemeColours next;
    const cfg::ParseResult result = cfg::parseThemeColours(data, len, next);
    if (!result.ok) {
        error = result.error;
        return false;
    }
    JsonDocument doc;
    {
        hub::Lock lock;
        colours = next;
        changes++;
        write(doc.to<JsonArray>());
    }
    File f = LittleFS.open(kFile, "w");
    if (!f) {
        error = "could not write to the hub's storage";
        return false;
    }
    serializeJson(doc, f);
    f.close();
    return true;
}

void json(JsonArray out) {
    hub::Lock lock;
    write(out);
}

uint32_t version() { return changes; }

}  // namespace theme_colours
