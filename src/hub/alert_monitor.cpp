#include "alert_monitor.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <math.h>

#include <atomic>
#include <vector>

#include "alert_dispatch.h"
#include "hub_state.h"

using namespace hg;
using alerts::Rule;

namespace alert_monitor {

namespace {

const char* kFile = "/alerts.json";
const uint32_t kCheckMs = 100;

alerts::Engine engine;
alerts::Dispatcher dispatcher;
std::atomic<uint32_t> changes{0};
std::atomic<uint32_t> ruleTotal{0};
uint32_t lastCheckMs = 0;

}  // namespace

void begin() {
    File f = LittleFS.open(kFile, "r");
    if (!f) return;
    std::vector<uint8_t> text(f.size());
    const size_t got = f.read(text.data(), text.size());
    f.close();
    std::vector<Rule> rules;
    const alerts::ParseResult result = alerts::parseRules(text.data(), got, rules);
    if (!result.ok) {
        Serial.printf("Stored alerts ignored: %s\n", result.error.c_str());
        return;
    }
    ruleTotal = uint32_t(rules.size());
    engine.setRules(std::move(rules));
}

void poll(uint32_t nowMs, const ChannelStore& store) {
    if (uint32_t(nowMs - lastCheckMs) < kCheckMs) return;
    lastCheckMs = nowMs;
    if (engine.evaluate(store, nowMs)) changes++;
    dispatcher.run(engine, hub::manager(), nowMs);
}

bool setRules(const uint8_t* data, size_t len, std::string& error) {
    std::vector<Rule> rules;
    const alerts::ParseResult result = alerts::parseRules(data, len, rules);
    if (!result.ok) {
        error = result.error;
        return false;
    }
    JsonDocument doc;
    JsonArray list = doc.to<JsonArray>();
    for (const Rule& r : rules) alerts::toJson(r, list.add<JsonObject>());
    {
        hub::Lock lock;
        ruleTotal = uint32_t(rules.size());
        engine.setRules(std::move(rules));
        dispatcher.rulesChanged();
        changes++;
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

void rulesJson(JsonArray out) {
    hub::Lock lock;
    for (const Rule& r : engine.rules()) alerts::toJson(r, out.add<JsonObject>());
}

void statesJson(JsonArray out) {
    hub::Lock lock;
    for (size_t i = 0; i < engine.rules().size(); i++) {
        const alerts::State& s = engine.state(i);
        JsonObject o = out.add<JsonObject>();
        o["active"] = s.active;
        o["count"] = s.count;
        if (s.haveValue) {
            o["value"] = roundf(s.value * 1000) / 1000;
        } else {
            o["value"] = nullptr;
        }
    }
}

bool test(size_t index, uint32_t seconds) {
    if (seconds < 1) seconds = 1;
    if (seconds > 60) seconds = 60;
    hub::Lock lock;
    if (!engine.test(index, millis(), seconds * 1000)) return false;
    changes++;
    return true;
}

size_t ruleCount() { return ruleTotal; }
uint32_t version() { return changes; }

}  // namespace alert_monitor
