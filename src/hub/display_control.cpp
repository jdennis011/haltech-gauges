#include "display_control.h"

#include <Preferences.h>

#include "hub_state.h"

using namespace hg;
using proto::Cmd;
using proto::HubManager;

namespace display_control {

namespace {

int brightnessAll = -1;
bool screensOff = false;

void saveBrightness() {
    Preferences prefs;
    prefs.begin("hub", false);
    prefs.putInt("all_bright", brightnessAll);
    prefs.end();
}

}  // namespace

void begin() {
    Preferences prefs;
    prefs.begin("hub", true);
    brightnessAll = prefs.getInt("all_bright", -1);
    prefs.end();
}

int allBrightness() { return brightnessAll; }

void setAllBrightness(int level) {
    if (level < 0) level = -1;
    if (level > 255) level = 255;
    brightnessAll = level;
    saveBrightness();
    if (level >= 0) sendAll(Cmd::SetBrightness, uint8_t(level));
}

bool displaysOff() { return screensOff; }

void setDisplaysOff(bool off) {
    screensOff = off;
    sendAll(Cmd::SetDisplay, off ? 0 : 1);
}

size_t sendAll(Cmd cmd, uint8_t arg) {
    HubManager& m = hub::manager();
    size_t sent = 0;
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = m.gauge(i);
        if (g.used && g.online && m.command(g.node, cmd, arg)) sent++;
    }
    return sent;
}

// A gauge that has just appeared gets the state everyone else is in.
void onOnline(const HubManager::Gauge& gauge, void*) {
    HubManager& m = hub::manager();
    if (brightnessAll >= 0) m.command(gauge.node, Cmd::SetBrightness, uint8_t(brightnessAll));
    if (screensOff) m.command(gauge.node, Cmd::SetDisplay, 0);
}

// From a gauge's pull-down menu. "This gauge" changes were already applied on
// the gauge itself; "all gauges" ones are applied to everyone and remembered.
void onRequest(const HubManager::Gauge&, const proto::Request& request, void*) {
    if (!request.all) return;
    switch (request.cmd) {
        case Cmd::SetBrightness: setAllBrightness(request.arg); break;
        case Cmd::SetDisplay: setDisplaysOff(request.arg == 0); break;
        default: sendAll(request.cmd, request.arg); break;
    }
}

}  // namespace display_control
