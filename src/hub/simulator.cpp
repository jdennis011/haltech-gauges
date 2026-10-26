#include "simulator.h"

#include <Preferences.h>
#include <math.h>
#include <string.h>

#include "playback.h"

using namespace hg;

namespace simulator {

namespace {

const size_t kMaxFrames = 96;

bool active = false;
uint32_t nextDueMs[kMaxFrames];
bool scheduled = false;
uint32_t generated = 0;

// The values' own clock: it stops while paused and runs at `rate`.
bool isPaused = false;
float rate = 1.0f;
double simClock = 0;
uint32_t lastRealMs = 0;

struct Override {
    ChannelId id;
    bool hold;
    float value;
    float min;
    float max;
};
Override overrides[kMaxOverrides];
size_t overrideTotal = 0;

// The settings in the hub's preferences (namespace "sim"): on, paused, speed,
// and the overrides as one blob of {id, hold, value, min, max} records.
const char* kPrefs = "sim";
bool loaded = false;

void save() {
    if (!loaded) return;  // nothing to keep before the stored settings are in
    Preferences p;
    if (!p.begin(kPrefs, false)) return;
    p.putBool("on", active);
    p.putBool("paused", isPaused);
    p.putFloat("speed", rate);
    struct Record {
        uint16_t id;
        uint8_t hold;
        float value, min, max;
    } records[kMaxOverrides];
    for (size_t i = 0; i < overrideTotal; i++) {
        records[i] = {uint16_t(overrides[i].id), uint8_t(overrides[i].hold ? 1 : 0), overrides[i].value, overrides[i].min,
                      overrides[i].max};
    }
    p.putBytes("ov", records, overrideTotal * sizeof(Record));
    p.end();
}

// 0..1..0 over `periodMs`.
float wave(uint32_t nowMs, uint32_t periodMs) {
    return 0.5f - 0.5f * cosf(float(nowMs % periodMs) * (2.0f * float(M_PI) / float(periodMs)));
}

float builtInValue(ChannelId id, uint32_t nowMs) {
    const float load = wave(nowMs, 8000);  // one "pull" every 8 seconds
    const float rpm = 800.0f + 6200.0f * load;
    const float speed = rpm / 60.0f;

    switch (id) {
        case CH_rpm: return rpm;
        case CH_manifold_pressure: return 30.0f + 220.0f * load;
        case CH_throttle_position:
        case CH_accelerator_pedal:
        case CH_engine_demand: return 100.0f * load;
        case CH_oil_pressure: return 180.0f + rpm / 20.0f;
        case CH_fuel_pressure: return 300.0f + 50.0f * load;
        case CH_coolant_pressure: return 90.0f;
        case CH_ignition_angle: return 12.0f + rpm / 400.0f;
        case CH_inj_stage1_duty: return 5.0f + 75.0f * load;
        case CH_wideband_1:
        case CH_wideband_overall:
        case CH_wideband_bank1: return 1.0f - 0.2f * load;
        case CH_target_lambda: return 1.0f - 0.18f * load;
        case CH_vehicle_speed:
        case CH_wheel_speed_fl:
        case CH_wheel_speed_fr:
        case CH_wheel_speed_rl:
        case CH_wheel_speed_rr: return speed;
        case CH_gear: return float(1 + int(speed / 25.0f));
        case CH_gear_selector: return -4.0f;  // drive
        case CH_battery_voltage: return 13.6f + 0.4f * wave(nowMs, 5000);
        case CH_barometric_pressure: return 101.3f;
        case CH_target_boost: return 101.3f + 140.0f * load;
        case CH_coolant_temp: return 86.0f + 8.0f * wave(nowMs, 60000);
        case CH_oil_temp: return 92.0f + 12.0f * wave(nowMs, 90000);
        case CH_air_temp: return 28.0f + 15.0f * load;
        case CH_fuel_level: return 38.0f;
        case CH_fuel_level_percent_0: return 63.0f;
        case CH_fuel_composition: return 10.0f;
        case CH_engine_state: return 3.0f;  // running
        case CH_ignition_switch: return 1.0f;
        case CH_engine_limiting_active: return rpm > 6900.0f ? 1.0f : 0.0f;
        // Headlights switch every 20 s so night dimming can be seen.
        case CH_head_light:
        case CH_park_light: return (nowMs / 20000) % 2 ? 1.0f : 0.0f;
        // The check engine light comes on for 5 s in every 30.
        case CH_check_engine_light: return (nowMs % 30000) < 5000 ? 1.0f : 0.0f;
        default: break;
    }

    // Anything not modelled gets a harmless resting value for its unit.
    switch (channelDef(id).unit) {
        case Unit::Celsius: return 25.0f;
        case Unit::Lambda: return 1.0f;
        case Unit::Volts: return 12.0f;
        case Unit::KpaAbs: return 101.3f;
        default: return 0.0f;
    }
}

float valueFor(ChannelId id, uint32_t nowMs) {
    for (size_t i = 0; i < overrideTotal; i++) {
        if (overrides[i].id != id) continue;
        if (overrides[i].hold) return overrides[i].value;
        return overrides[i].min + (overrides[i].max - overrides[i].min) * wave(nowMs, 8000);
    }
    float played;
    if (playback::value(id, played)) return played;
    return builtInValue(id, nowMs);
}

int findOverride(ChannelId id) {
    for (size_t i = 0; i < overrideTotal; i++) {
        if (overrides[i].id == id) return int(i);
    }
    return -1;
}

}  // namespace

void load() {
    Preferences p;
    if (p.begin(kPrefs, true)) {
        active = p.getBool("on", false);
        isPaused = p.getBool("paused", false);
        const float s = p.getFloat("speed", 1.0f);
        rate = !(s > 0) ? 1.0f : (s < 0.05f ? 0.05f : (s > 10.0f ? 10.0f : s));
        struct Record {
            uint16_t id;
            uint8_t hold;
            float value, min, max;
        } records[kMaxOverrides];
        const size_t bytes = p.getBytes("ov", records, sizeof(records));
        overrideTotal = 0;
        for (size_t i = 0; i < bytes / sizeof(Record) && i < kMaxOverrides; i++) {
            if (records[i].id >= CH_COUNT) continue;
            overrides[overrideTotal++] = {ChannelId(records[i].id), records[i].hold != 0, records[i].value, records[i].min,
                                          records[i].max};
        }
        p.end();
    }
    loaded = true;
    scheduled = false;
}

void setEnabled(bool enabled) {
    active = enabled;
    scheduled = false;
    save();
}

bool enabled() { return active; }

void setPaused(bool paused) {
    isPaused = paused;
    save();
}
bool paused() { return isPaused; }

void setSpeed(float speed) {
    if (!(speed > 0)) speed = 1.0f;
    rate = speed < 0.05f ? 0.05f : (speed > 10.0f ? 10.0f : speed);
    save();
}
float speed() { return rate; }

bool setOverride(ChannelId id, bool hold, float value, float min, float max) {
    int i = findOverride(id);
    if (i < 0) {
        if (overrideTotal >= kMaxOverrides) return false;
        i = int(overrideTotal++);
    }
    overrides[i] = {id, hold, value, min, max};
    save();
    return true;
}

bool clearOverride(ChannelId id) {
    const int i = findOverride(id);
    if (i < 0) return false;
    overrides[i] = overrides[--overrideTotal];
    save();
    return true;
}

void clearOverrides() {
    overrideTotal = 0;
    save();
}

size_t overrideCount() { return overrideTotal; }

void overridesJson(JsonArray out) {
    for (size_t i = 0; i < overrideTotal; i++) {
        JsonObject o = out.add<JsonObject>();
        o["channel"] = channelDef(overrides[i].id).name;
        if (overrides[i].hold) {
            o["hold"] = overrides[i].value;
        } else {
            o["min"] = overrides[i].min;
            o["max"] = overrides[i].max;
        }
    }
}

namespace {

bool applyOverride(const char* name, JsonVariantConst v, std::string& error) {
    const int id = findChannel(name ? name : "");
    if (id < 0) {
        error = std::string("unknown channel '") + (name ? name : "") + "' in overrides";
        return false;
    }
    bool ok = true;
    if (v.isNull()) {
        clearOverride(ChannelId(id));
    } else if (v["hold"].is<float>()) {
        ok = setOverride(ChannelId(id), true, v["hold"].as<float>(), 0, 0);
    } else if (v["min"].is<float>() && v["max"].is<float>()) {
        ok = setOverride(ChannelId(id), false, 0, v["min"].as<float>(), v["max"].as<float>());
    } else {
        error = "an override is {hold: value} or {min, max}, or null to remove it";
        return false;
    }
    if (!ok) {
        error = "at most 16 overrides";
        return false;
    }
    return true;
}

}  // namespace

bool applyJson(JsonVariantConst in, std::string& error) {
    if (in["paused"].is<bool>()) setPaused(in["paused"].as<bool>());
    if (in["speed"].is<float>()) setSpeed(in["speed"].as<float>());
    if (in["clear"] | false) clearOverrides();
    JsonVariantConst ov = in["overrides"];
    if (ov.is<JsonObjectConst>()) {
        for (JsonPairConst kv : ov.as<JsonObjectConst>()) {
            if (!applyOverride(kv.key().c_str(), kv.value(), error)) return false;
        }
    } else if (ov.is<JsonArrayConst>()) {
        for (JsonObjectConst o : ov.as<JsonArrayConst>()) {
            if (!applyOverride(o["channel"] | "", o, error)) return false;
        }
    }
    return true;
}

uint32_t framesGenerated() { return generated; }

void poll(uint32_t nowMs, hg::ChannelStore& store, GaugePort* port) {
    if (!active) return;
    const size_t count = frameCount() < kMaxFrames ? frameCount() : kMaxFrames;

    if (!scheduled) {
        // Spread the first transmissions out so frames do not all fall due together.
        for (size_t i = 0; i < count; i++) nextDueMs[i] = nowMs + uint32_t(i % 20);
        scheduled = true;
        lastRealMs = nowMs;
    }
    if (!isPaused) simClock += double(nowMs - lastRealMs) * rate;
    lastRealMs = nowMs;
    const uint32_t simMs = uint32_t(simClock);

    for (size_t i = 0; i < count; i++) {
        if (int32_t(nowMs - nextDueMs[i]) < 0) continue;
        const FrameDef& def = frameDef(i);

        hg::CanFrame frame;
        beginFrame(def, frame);
        for (uint16_t c = 0; c < def.channelCount; c++) {
            const ChannelId id = ChannelId(def.firstChannel + c);
            encodeChannel(id, valueFor(id, simMs), frame);
        }
        store.onFrame(frame, nowMs);
        if (port) port->write(frame);
        generated++;
        nextDueMs[i] = nowMs + 1000u / def.rateHz;
    }
}

}  // namespace simulator
