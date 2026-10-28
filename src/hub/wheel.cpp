#include "wheel.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <stdio.h>
#include <string.h>

#include "board_pins.h"
#include "crc32.h"
#include "fg_can.h"
#include "hub_state.h"
#include "swc_output.h"
#include "wheel_controller.h"

using namespace hg;
using namespace hg::proto;

namespace wheel {

namespace {

const char* kPrefs = "wheel";
const char* kCalFile = "/wheel-cal.json";
const uint32_t kCalRetryMs = 3000;
const size_t kMaxCalBytes = 2048;

// The kInput... bit names, in bit order, as the page and the wheel use them.
const char* const kButtonNames[14] = {"horn",       "cruise_on_off", "cruise_set", "cruise_resume", "cruise_accel",
                                      "cruise_decel", "cruise_cancel", "vol_up",   "vol_down",      "mode",
                                      "mute",       "seek_up",       "seek_down",  "phone"};

hg::wheel::Controller controller;
uint32_t version_ = 1;
std::string calibration;  // the hub's copy, as sent to the wheel
uint32_t calibrationCrc = 0;
bool calibrationLoaded = false;
uint32_t calSentMs = 0;
bool calPending = false;  // sent, waiting for the wheel's status to confirm
uint16_t lastMask = 0;

void bump() { version_++; }

void hornOut(bool on, void*) {
    digitalWrite(PIN_HORN, on ? HIGH : LOW);
    bump();
}
void cruiseOut(uint16_t mask, void*) {
    fg_can::setCruise(mask);
    bump();
}
void mediaOut(uint16_t mask, void*) {
    swc_output::set(mask);
    bump();
}

void loadSettings() {
    hg::wheel::Settings s;
    Preferences p;
    if (p.begin(kPrefs, true)) {
        s.hornEnabled = p.getBool("horn_on", false);
        s.cruiseEnabled = p.getBool("cruise_on", false);
        s.mediaEnabled = p.getBool("media_on", false);
        p.end();
    }
    controller.init({hornOut, cruiseOut, mediaOut, nullptr}, s);
}

void saveSettings() {
    const hg::wheel::Settings& s = controller.settings();
    Preferences p;
    if (!p.begin(kPrefs, false)) return;
    p.putBool("horn_on", s.hornEnabled);
    p.putBool("cruise_on", s.cruiseEnabled);
    p.putBool("media_on", s.mediaEnabled);
    p.end();
}

void loadCalibration() {
    calibrationLoaded = true;
    calibration.clear();
    calibrationCrc = 0;
    File f = LittleFS.open(kCalFile, "r");
    if (!f) return;
    while (f.available()) {
        char buf[256];
        const size_t n = f.readBytes(buf, sizeof(buf));
        calibration.append(buf, n);
        if (calibration.size() > kMaxCalBytes) break;
    }
    f.close();
    calibrationCrc = crc32(reinterpret_cast<const uint8_t*>(calibration.data()), calibration.size());
}

int buttonIndex(const char* name) {
    for (int i = 0; i < 14; i++) {
        if (strcmp(name, kButtonNames[i]) == 0) return i;
    }
    return -1;
}

}  // namespace

void begin() {
    pinMode(PIN_HORN, OUTPUT);
    digitalWrite(PIN_HORN, LOW);
    loadSettings();
    fg_can::begin();
    swc_output::begin();
}

void onInput(const HubManager::Gauge& gauge, const Input& input, void*) {
    controller.onInput(gauge.node, input, millis());
    if (input.mask != lastMask || (input.flags & kInputFlagFault)) {
        lastMask = input.mask;
        bump();
    }
}

void onInputRaw(const HubManager::Gauge& gauge, const InputRaw& raw, void*) {
    controller.onRaw(gauge.node, raw, millis());
    bump();
}

void poll(uint32_t nowMs) {
    if (!calibrationLoaded) loadCalibration();
    const bool lostBefore = controller.state().linkLost;
    const uint32_t releasesBefore = controller.state().failSafeReleases;
    controller.poll(nowMs);
    if (controller.state().linkLost != lostBefore || controller.state().failSafeReleases != releasesBefore) bump();
    fg_can::poll(nowMs);
    swc_output::poll(nowMs);

    // The wheel on the roster: gone offline, or holding a calibration that
    // differs from the hub's copy.
    HubManager& m = hub::manager();
    const HubManager::Gauge* w = nullptr;
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = m.gauge(i);
        if (g.used && g.online && HubManager::isWheel(g)) {
            w = &g;
            break;
        }
    }
    if (!w) {
        if (controller.state().seen) {
            controller.reset(nowMs);
            lastMask = 0;
            bump();
        }
        calPending = false;
        return;
    }
    if (!calibration.empty() && w->haveStatus && w->status.configCrc != calibrationCrc) {
        if (calPending && uint32_t(nowMs - calSentMs) < kCalRetryMs) return;
        if (m.pushBusy()) return;
        if (m.sendPayload(w->node, XferKind::WheelCalibration, reinterpret_cast<const uint8_t*>(calibration.data()),
                          uint32_t(calibration.size()), nowMs)) {
            calPending = true;
            calSentMs = nowMs;
        }
    } else if (calPending) {
        calPending = false;
        bump();
    }
}

uint32_t version() { return version_; }

void statusJson(JsonObject out) {
    const hg::wheel::State& s = controller.state();
    HubManager& m = hub::manager();
    const HubManager::Gauge* w = nullptr;
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = m.gauge(i);
        if (g.used && HubManager::isWheel(g)) {
            w = &g;
            break;
        }
    }
#if HG_FG_ON_GAUGE_PORT
    out["present"] = true;  // the test build is about the card's FG bus section
#else
    out["present"] = w != nullptr || s.seen || !calibration.empty();
#endif
    out["online"] = w && w->online;
    if (w) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%06lX", (unsigned long)w->node);
        out["node"] = buf;
        snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", w->mac[0], w->mac[1], w->mac[2], w->mac[3],
                 w->mac[4], w->mac[5]);
        out["mac"] = buf;
        if (w->haveInfo) {
            snprintf(buf, sizeof(buf), "%u.%u.%u", w->info.fwMajor, w->info.fwMinor, w->info.fwPatch);
            out["fw"] = buf;
        }
        out["last_seen_s"] = (millis() - w->lastSeenMs) / 1000;
    }
    out["link_lost"] = s.linkLost;
    out["fail_safe_releases"] = s.failSafeReleases;
    out["seq_gaps"] = s.seqGaps;
    out["mask"] = s.mask;
    out["flags"] = s.flags;
    out["fault"] = (s.flags & kInputFlagFault) != 0;
    out["uncalibrated"] = (s.flags & kInputFlagUncalibrated) != 0;
    out["lines"] = s.lines;
    JsonArray buttons = out["buttons"].to<JsonArray>();
    const uint32_t now = millis();
    for (int i = 0; i < 14; i++) {
        JsonObject b = buttons.add<JsonObject>();
        b["name"] = kButtonNames[i];
        b["down"] = (s.mask & (1u << i)) != 0;
        if (s.changedMs[i]) b["changed_s"] = (now - s.changedMs[i]) / 1000;
    }
    JsonArray raw = out["raw"].to<JsonArray>();
    for (size_t i = 0; i < hg::wheel::kLines; i++) raw.add(s.rawMv[i]);
    if (s.rawMs) out["raw_age_s"] = (now - s.rawMs) / 1000;
    else out["raw_age_s"] = -1;
    JsonObject outputs = out["outputs"].to<JsonObject>();
    outputs["horn"] = s.hornOn;
    outputs["cruise"] = s.cruiseOn;
    outputs["media"] = s.mediaOn;
    JsonObject settings = out["settings"].to<JsonObject>();
    settings["horn"] = controller.settings().hornEnabled;
    settings["cruise"] = controller.settings().cruiseEnabled;
    settings["media"] = controller.settings().mediaEnabled;
    JsonObject cal = out["calibration"].to<JsonObject>();
    cal["stored"] = !calibration.empty();
    char crc[12];
    snprintf(crc, sizeof(crc), "%08lX", (unsigned long)calibrationCrc);
    cal["crc"] = crc;
    cal["in_sync"] = w && w->haveStatus && w->status.configCrc == calibrationCrc && !calibration.empty();
    cal["pending"] = calPending;
    if (!calibration.empty()) cal["json"] = calibration;
    fg_can::statusJson(out["fg"].to<JsonObject>());
    swc_output::statusJson(out["swc"].to<JsonObject>());
}

bool setSettings(JsonVariantConst in, std::string& error) {
    hg::wheel::Settings s = controller.settings();
    if (!in["horn"].isNull()) s.hornEnabled = in["horn"].as<bool>();
    if (!in["cruise"].isNull()) s.cruiseEnabled = in["cruise"].as<bool>();
    if (!in["media"].isNull()) s.mediaEnabled = in["media"].as<bool>();
    (void)error;
    controller.setSettings(s, millis());
    saveSettings();
    bump();
    return true;
}

bool setCalibration(const uint8_t* json, size_t len, std::string& error) {
    if (len == 0 || len > kMaxCalBytes) {
        error = "the calibration is up to 2 KB of JSON";
        return false;
    }
    JsonDocument doc;
    if (deserializeJson(doc, json, len) != DeserializationError::Ok || !doc["lines"].is<JsonArrayConst>()) {
        error = "the calibration is a JSON object with a 'lines' list";
        return false;
    }
    File f = LittleFS.open(kCalFile, "w");
    if (!f) {
        error = "could not write the calibration";
        return false;
    }
    f.write(json, len);
    f.close();
    calibration.assign(reinterpret_cast<const char*>(json), len);
    calibrationCrc = crc32(json, len);
    calibrationLoaded = true;
    calPending = false;  // the next poll sends it if the wheel differs
    bump();
    return true;
}

bool test(const char* what, uint32_t ms, std::string& error) {
    if (ms == 0 || ms > 2000) {
        error = "'ms' is 1 to 2000";
        return false;
    }
    const int i = what ? buttonIndex(what) : -1;
    if (i < 0) {
        error = "'output' is horn, a cruise button or a media button by name";
        return false;
    }
    const uint16_t bit = uint16_t(1u << i);
    bool ok;
    if (bit == kInputHorn) {
        ok = controller.settings().hornEnabled && controller.test(hg::wheel::Output::Horn, 0, ms, millis());
    } else if (bit & kInputCruiseMask) {
        ok = controller.settings().cruiseEnabled && controller.test(hg::wheel::Output::Cruise, bit, ms, millis());
    } else {
        ok = controller.settings().mediaEnabled && controller.test(hg::wheel::Output::Media, bit, ms, millis());
    }
    if (!ok) error = "that output is off, held by the wheel, or already being tested";
    bump();
    return ok;
}

bool requestRaw(uint8_t seconds, std::string& error) {
    HubManager& m = hub::manager();
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = m.gauge(i);
        if (g.used && g.online && HubManager::isWheel(g)) {
            if (m.command(g.node, Cmd::InputRaw, seconds)) return true;
            error = "the bus could not take the command; try again";
            return false;
        }
    }
    error = "no steering wheel module is online";
    return false;
}

}  // namespace wheel
