#include "fg_can.h"

#include <Arduino.h>
#include <Preferences.h>
#include <stdio.h>
#include <string.h>

#include "can_ports.h"
#include "fg_cruise.h"
#include "haltech.h"
#include "hub_state.h"

using namespace hg::wheel;

namespace fg_can {

namespace {

const char* kPrefs = "wheel";
const char* kKey = "fg";
const size_t kCaptureIds = 64;
const size_t kProbeHits = 32;
const uint32_t kProbeStepMs = 250;

struct Settings {
    bool enabled = false;
    uint16_t kbps = 500;
    bool sendIdle = true;
    FgCruiseFrame table;
};
Settings settings;
bool listen = false;
bool portUp = false;
uint16_t cruiseNow = 0;
uint8_t counter = 0;
uint32_t lastSendMs = 0;
uint32_t txCount = 0, rxCount = 0, txFailed = 0;
std::string lastError;

struct Seen {
    bool used = false;
    uint32_t id = 0;
    bool extended = false;
    uint8_t len = 0;
    uint8_t data[8] = {};
    uint32_t count = 0;
    uint32_t lastMs = 0;
    uint32_t periodMs = 0;
};
Seen seen[kCaptureIds];

struct Probe {
    bool running = false;
    uint32_t id = 0;
    bool extended = false;
    int bit = -1;  // the bit being held, 0..63
    uint32_t stepMs = 0;
    float baseState = 0, baseInput = 0;
    bool haveBase = false;
    struct Hit {
        uint8_t bit;
        const char* channel;
        float from, to;
    } hits[kProbeHits];
    size_t hitCount = 0;
};
Probe probe_;

#if HG_HAS_FG_PORT
FgPort port;
#endif

TwaiSpeed speedOf(uint16_t kbps) {
    switch (kbps) {
        case 125: return TWAI_SPEED_125KBPS;
        case 250: return TWAI_SPEED_250KBPS;
        case 1000: return TWAI_SPEED_1000KBPS;
        default: return TWAI_SPEED_500KBPS;
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
    settings.enabled = doc["enabled"] | false;
    settings.kbps = uint16_t(doc["kbps"] | 500);
    settings.sendIdle = doc["send_idle"] | true;
    std::string error;
    FgCruiseFrame t;
    if (fgFromJson(doc["table"], t, error)) settings.table = t;
}

void save() {
    JsonDocument doc;
    doc["enabled"] = settings.enabled;
    doc["kbps"] = settings.kbps;
    doc["send_idle"] = settings.sendIdle;
    fgToJson(settings.table, doc["table"].to<JsonObject>());
    String text;
    serializeJson(doc, text);
    Preferences p;
    if (!p.begin(kPrefs, false)) return;
    p.putString(kKey, text);
    p.end();
}

bool startPort() {
#if HG_HAS_FG_PORT
    if (portUp) return true;
    if (!port.begin(speedOf(settings.kbps))) {
        lastError = "the vehicle CAN port did not start";
        return false;
    }
    port.setListenOnly(listen);
    portUp = true;
    return true;
#else
    lastError = "not on this board";
    return false;
#endif
}

void send(uint16_t mask) {
#if HG_HAS_FG_PORT
    if (!portUp || listen || !fgValid(settings.table)) return;
    hg::CanFrame f;
    if (!fgEncode(settings.table, mask, counter++, f)) return;
    if (port.write(f)) txCount++; else txFailed++;
#else
    (void)mask;
#endif
}

void noteSeen(const hg::CanFrame& f, uint32_t nowMs) {
    rxCount++;
    Seen* slot = nullptr;
    for (Seen& s : seen) {
        if (s.used && s.id == f.id && s.extended == f.extended) {
            slot = &s;
            break;
        }
    }
    if (!slot) {
        for (Seen& s : seen) {
            if (!s.used) {
                slot = &s;
                break;
            }
        }
    }
    if (!slot) return;  // the table is full: the first 64 ids are kept
    if (slot->used && slot->lastMs) slot->periodMs = nowMs - slot->lastMs;
    slot->used = true;
    slot->id = f.id;
    slot->extended = f.extended;
    slot->len = f.len;
    memcpy(slot->data, f.data, 8);
    slot->count++;
    slot->lastMs = nowMs;
}

bool haltechCruise(uint32_t nowMs, float& state, float& input) {
    const hg::ChannelStore& live = hub::live();
    return live.get(hg::CH_cruise_state, nowMs, state) && live.get(hg::CH_cruise_input_state, nowMs, input);
}

void stepProbe(uint32_t nowMs) {
    if (!probe_.running) return;
    float state = 0, input = 0;
    const bool have = haltechCruise(nowMs, state, input);
    if (probe_.bit >= 0 && have && probe_.haveBase) {
        const bool changed = state != probe_.baseState || input != probe_.baseInput;
        if (changed && probe_.hitCount < kProbeHits) {
            const bool stateChanged = state != probe_.baseState;
            probe_.hits[probe_.hitCount++] = {uint8_t(probe_.bit), stateChanged ? "cruise_state" : "cruise_input_state",
                                              stateChanged ? probe_.baseState : probe_.baseInput, stateChanged ? state : input};
            probe_.bit = 63;  // move on: the next step starts the following bit
            probe_.stepMs = 0;
        }
    }
    if (uint32_t(nowMs - probe_.stepMs) < kProbeStepMs) return;
    probe_.stepMs = nowMs;
    if (!probe_.haveBase) {
        if (!have) return;  // no Haltech data: wait for it
        probe_.baseState = state;
        probe_.baseInput = input;
        probe_.haveBase = true;
        probe_.bit = -1;
    }
    probe_.bit++;
    if (probe_.bit >= 64) {
        probe_.running = false;
        return;
    }
#if HG_HAS_FG_PORT
    if (!portUp || listen) return;
    hg::CanFrame f;
    f.id = probe_.id;
    f.extended = probe_.extended;
    f.len = 8;
    memset(f.data, 0, 8);
    f.data[probe_.bit / 8] = uint8_t(1u << (probe_.bit % 8));
    for (int k = 0; k < 5; k++) {  // held for the step at about 20 Hz
        if (port.write(f)) txCount++; else txFailed++;
    }
#endif
}

}  // namespace

void begin() {
    load();
#if HG_HAS_FG_PORT
    if (settings.enabled) startPort();
#endif
}

void poll(uint32_t nowMs) {
#if HG_HAS_FG_PORT
    if (portUp) {
        hg::CanFrame f;
        while (port.read(f, 0)) noteSeen(f, nowMs);
        static uint32_t lastServiceMs = 0;
        if (nowMs - lastServiceMs >= 500) {
            lastServiceMs = nowMs;
            port.service();
        }
        if (settings.enabled && !listen && fgValid(settings.table) && (cruiseNow || settings.sendIdle)) {
            const uint32_t period = 1000 / (settings.table.rateHz ? settings.table.rateHz : 20);
            if (uint32_t(nowMs - lastSendMs) >= period) {
                lastSendMs = nowMs;
                send(cruiseNow);
            }
        }
        stepProbe(nowMs);
    }
#else
    (void)nowMs;
#endif
}

void setCruise(uint16_t mask) {
    mask &= hg::proto::kInputCruiseMask;
    if (mask == cruiseNow) return;
    cruiseNow = mask;
    if (settings.enabled) {
        send(cruiseNow);
        lastSendMs = millis();
    }
}

bool available() {
#if HG_HAS_FG_PORT
    return true;
#else
    return false;
#endif
}

void statusJson(JsonObject out) {
    out["available"] = available();
    out["enabled"] = settings.enabled;
    out["kbps"] = settings.kbps;
    out["send_idle"] = settings.sendIdle;
    out["listen"] = listen;
    out["up"] = portUp;
    out["tx"] = txCount;
    out["tx_failed"] = txFailed;
    out["rx"] = rxCount;
    out["cruise"] = cruiseNow;
    out["error"] = lastError;
#if HG_HAS_FG_PORT
    if (portUp) {
        const CanPortStats s = port.stats();
        out["bus_errors"] = s.busErrors;
        out["state"] = s.state;
    }
#endif
    out["table_valid"] = fgValid(settings.table);
    fgToJson(settings.table, out["table"].to<JsonObject>());
    JsonArray capture = out["capture"].to<JsonArray>();
    const uint32_t now = millis();
    for (const Seen& s : seen) {
        if (!s.used) continue;
        JsonObject o = capture.add<JsonObject>();
        char buf[32];
        snprintf(buf, sizeof(buf), s.extended ? "0x%08lX" : "0x%03lX", (unsigned long)s.id);
        o["id"] = buf;
        o["count"] = s.count;
        o["period_ms"] = s.periodMs;
        o["age_s"] = (now - s.lastMs) / 1000;
        size_t at = 0;
        char data[8 * 3];
        for (int i = 0; i < s.len && i < 8; i++) at += size_t(snprintf(data + at, sizeof(data) - at, i ? " %02X" : "%02X", s.data[i]));
        o["data"] = data;
    }
    JsonObject pr = out["probe"].to<JsonObject>();
    pr["running"] = probe_.running;
    if (probe_.id) {
        char buf[16];
        snprintf(buf, sizeof(buf), "0x%lX", (unsigned long)probe_.id);
        pr["id"] = buf;
    }
    pr["bit"] = probe_.bit;
    JsonArray hits = pr["hits"].to<JsonArray>();
    for (size_t i = 0; i < probe_.hitCount; i++) {
        JsonObject h = hits.add<JsonObject>();
        h["bit"] = probe_.hits[i].bit;
        h["byte"] = probe_.hits[i].bit / 8;
        h["mask"] = 1u << (probe_.hits[i].bit % 8);
        h["channel"] = probe_.hits[i].channel;
        h["from"] = probe_.hits[i].from;
        h["to"] = probe_.hits[i].to;
    }
}

bool setSettings(JsonVariantConst in, std::string& error) {
    Settings next = settings;
    if (!in["enabled"].isNull()) next.enabled = in["enabled"].as<bool>();
    if (!in["kbps"].isNull()) {
        const int k = in["kbps"].as<int>();
        if (k != 125 && k != 250 && k != 500 && k != 1000) {
            error = "'kbps' is 125, 250, 500 or 1000";
            return false;
        }
        next.kbps = uint16_t(k);
    }
    if (!in["send_idle"].isNull()) next.sendIdle = in["send_idle"].as<bool>();
    if (!in["table"].isNull()) {
        FgCruiseFrame t;
        if (!fgFromJson(in["table"], t, error)) return false;
        next.table = t;
    }
    if (next.enabled && !available()) {
        error = "this board has no vehicle CAN port";
        return false;
    }
    const bool speedChanged = next.kbps != settings.kbps;
    settings = next;
    save();
#if HG_HAS_FG_PORT
    if (settings.enabled && (!portUp || speedChanged)) {
        if (portUp && speedChanged) port.begin(speedOf(settings.kbps));  // the library re-begins in place
        else if (!startPort()) {
            error = lastError;
            return false;
        }
    }
#else
    (void)speedChanged;
#endif
    return true;
}

bool setListen(bool on, std::string& error) {
    if (!available()) {
        error = "this board has no vehicle CAN port";
        return false;
    }
    listen = on;
#if HG_HAS_FG_PORT
    if (!portUp && !startPort()) {
        error = lastError;
        return false;
    }
    port.setListenOnly(on);
#endif
    return true;
}

bool probe(uint32_t id, bool extended, std::string& error) {
    if (!available()) {
        error = "this board has no vehicle CAN port";
        return false;
    }
    if (id == 0) {
        probe_.running = false;
        return true;
    }
    if (listen) {
        error = "listen only is on; nothing can be sent";
        return false;
    }
#if HG_HAS_FG_PORT
    if (!portUp && !startPort()) {
        error = lastError;
        return false;
    }
#endif
    probe_ = Probe();
    probe_.running = true;
    probe_.id = id;
    probe_.extended = extended;
    probe_.stepMs = millis();
    return true;
}

}  // namespace fg_can
