#include "intro.h"

#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

#include "hub_state.h"
#include "intro_sequencer.h"
#include "virtual_gauges.h"

using namespace hg;
using proto::HubManager;
using proto::IntroStyle;

namespace intro {

namespace {

const char* kPrefs = "hub";
const uint32_t kRosterNoteMs = 5000;

hg::intro::Sequencer sequencer;
uint8_t lastGauges = 0;  // how many were online last time, kept in flash
uint32_t lastNoteMs = 0;

const char* styleName(IntroStyle s) {
    switch (s) {
        case IntroStyle::Sweep: return "sweep";
        case IntroStyle::Chase: return "chase";
        case IntroStyle::Fade: return "fade";
    }
    return "sweep";
}

bool styleFrom(const char* name, IntroStyle& out) {
    if (!name) return false;
    if (strcmp(name, "sweep") == 0) out = IntroStyle::Sweep;
    else if (strcmp(name, "chase") == 0) out = IntroStyle::Chase;
    else if (strcmp(name, "fade") == 0) out = IntroStyle::Fade;
    else return false;
    return true;
}

void saveSettings(const hg::intro::Settings& s) {
    Preferences p;
    p.begin(kPrefs, false);
    p.putBool("intro_on", s.enabled);
    p.putUChar("intro_style", uint8_t(s.style));
    p.putUShort("intro_stag", s.staggerMs);
    p.putUShort("intro_dur", s.durationMs);
    p.end();
}

// The planned gauges linked to hardware, in their order: the display order.
size_t linkedOrder(hg::intro::Slot* out, size_t cap) {
    virtual_gauges::Entry planned[virtual_gauges::kMax];
    const size_t count = virtual_gauges::snapshot(planned, virtual_gauges::kMax);
    size_t n = 0;
    for (size_t k = 0; k < count && n < cap; k++) {
        const std::string& mac = planned[k].mac;
        if (mac.rfind("02:00:00:00:00:", 0) == 0) continue;  // synthetic: no hardware yet
        unsigned b[6];
        if (sscanf(mac.c_str(), "%2x:%2x:%2x:%2x:%2x:%2x", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) continue;
        for (int i = 0; i < 6; i++) out[n].mac[i] = uint8_t(b[i]);
        n++;
    }
    return n;
}

void applyOrder() {
    hg::intro::Slot slots[hg::intro::Sequencer::kMaxSlots];
    const size_t n = linkedOrder(slots, hg::intro::Sequencer::kMaxSlots);
    sequencer.setOrder(slots, n);
}

}  // namespace

void begin() {
    hg::intro::Settings s;
    Preferences p;
    p.begin(kPrefs, true);
    s.enabled = p.getBool("intro_on", true);
    const uint8_t style = p.getUChar("intro_style", uint8_t(IntroStyle::Sweep));
    s.style = style <= uint8_t(IntroStyle::Fade) ? IntroStyle(style) : IntroStyle::Sweep;
    s.staggerMs = p.getUShort("intro_stag", 250);
    s.durationMs = p.getUShort("intro_dur", 1200);
    lastGauges = p.getUChar("last_gauges", 0);
    p.end();

    hg::intro::Slot slots[hg::intro::Sequencer::kMaxSlots];
    const size_t linked = linkedOrder(slots, hg::intro::Sequencer::kMaxSlots);
    const size_t expected = linked > lastGauges ? linked : lastGauges;
    hub::Lock lock;
    sequencer.setSettings(s);
    sequencer.setOrder(slots, linked);
    sequencer.begin(millis(), expected);
}

void poll(uint32_t nowMs) {
    bool noteRoster = false;
    uint8_t online = 0;
    {
        hub::Lock lock;
        applyOrder();
        if (sequencer.poll(hub::manager(), nowMs)) Serial.println("Intro sent to the gauges");
        if (uint32_t(nowMs - lastNoteMs) >= kRosterNoteMs) {
            lastNoteMs = nowMs;
            HubManager& m = hub::manager();
            for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
                const HubManager::Gauge& g = m.gauge(i);
                if (g.used && g.online && g.haveHello) online++;
            }
            noteRoster = online != lastGauges;
        }
    }
    // How many gauges there are, for the wait at the next key-on. Written
    // only when it changes.
    if (noteRoster && online > 0) {
        lastGauges = online;
        Preferences p;
        p.begin(kPrefs, false);
        p.putUChar("last_gauges", lastGauges);
        p.end();
    }
}

size_t play() {
    hub::Lock lock;
    applyOrder();
    return sequencer.play(hub::manager(), millis());
}

void settingsJson(JsonObject out) {
    hub::Lock lock;
    const hg::intro::Settings& s = sequencer.settings();
    out["enabled"] = s.enabled;
    out["style"] = styleName(s.style);
    out["stagger"] = s.staggerMs;
    out["duration"] = s.durationMs;
    out["done"] = sequencer.done();
    out["expected"] = sequencer.expected();
}

bool setSettings(JsonObjectConst in, std::string& error) {
    hg::intro::Settings s;
    {
        hub::Lock lock;
        s = sequencer.settings();
    }
    if (!in["enabled"].isNull()) s.enabled = in["enabled"].as<bool>();
    if (!in["style"].isNull() && !styleFrom(in["style"].as<const char*>(), s.style)) {
        error = "'style' must be sweep, chase or fade";
        return false;
    }
    if (!in["stagger"].isNull()) {
        const int v = in["stagger"].as<int>();
        if (v < 0 || v > 5000) {
            error = "'stagger' is milliseconds, 0 to 5000";
            return false;
        }
        s.staggerMs = uint16_t(v);
    }
    if (!in["duration"].isNull()) {
        const int v = in["duration"].as<int>();
        if (v < 200 || v > 10000) {
            error = "'duration' is milliseconds, 200 to 10000";
            return false;
        }
        s.durationMs = uint16_t(v);
    }
    {
        hub::Lock lock;
        sequencer.setSettings(s);
    }
    saveSettings(s);
    return true;
}

int slotOf(const uint8_t mac[6]) { return sequencer.slotOf(hub::manager(), mac); }

}  // namespace intro
