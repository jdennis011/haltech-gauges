#include "drag_overlay.h"

#include <Arduino.h>
#include <Preferences.h>

#include <memory>

#include "drag_timer.h"

namespace drag_overlay {

namespace {

// One face per run type, in hg::motion::Run order, like the faces a config
// would hold: the timer with its state line and splits, the speed under it.
const char kJson[] =
    R"({"schema":1,"name":"Drag timer","theme":"white","faces":[)"
    R"({"name":"0-100","widgets":[{"type":"dragtimer","run":"0-100","size":72,"y":233},)"
    R"({"type":"number","channel":"vehicle_speed","y":380,"size":40,"color":"dim"},)"
    R"({"type":"label","text":"PULL UP TO CLOSE","y":436,"size":14,"color":"dim"}]},)"
    R"({"name":"Strip","widgets":[{"type":"dragtimer","run":"strip","rollout":true,"size":56,"y":190},)"
    R"({"type":"number","channel":"vehicle_speed","y":378,"size":36,"color":"dim"},)"
    R"({"type":"label","text":"PULL UP TO CLOSE","y":436,"size":14,"color":"dim"}]},)"
    R"({"name":"80-120","widgets":[{"type":"dragtimer","run":"80-120","size":72,"y":233},)"
    R"({"type":"number","channel":"vehicle_speed","y":380,"size":40,"color":"dim"},)"
    R"({"type":"label","text":"PULL UP TO CLOSE","y":436,"size":14,"color":"dim"}]},)"
    R"({"name":"100-200","widgets":[{"type":"dragtimer","run":"100-200","size":72,"y":233},)"
    R"({"type":"number","channel":"vehicle_speed","y":380,"size":40,"color":"dim"},)"
    R"({"type":"label","text":"PULL UP TO CLOSE","y":436,"size":14,"color":"dim"}]}]})";

const char* kPrefs = "motion";  // shared with the accelerometer's calibration

std::unique_ptr<hg::cfg::Config> parsed;
bool parseTried = false;
bool shown = false;
bool loaded = false;
size_t runIndex = 0;

void load() {
    if (loaded) return;
    loaded = true;
    Preferences p;
    p.begin(kPrefs, true);
    runIndex = p.getUChar("run", 0);
    p.end();
    if (runIndex >= hg::motion::kRunCount) runIndex = 0;
}

void save() {
    Preferences p;
    p.begin(kPrefs, false);
    p.putUChar("run", uint8_t(runIndex));
    p.end();
}

}  // namespace

bool active() { return shown; }

void toggle() {
    if (!shown && !config()) return;  // nothing to show
    shown = !shown;
    if (shown) load();
}

const hg::cfg::Config* config() {
    if (!parseTried) {
        parseTried = true;
        std::unique_ptr<hg::cfg::Config> c(new hg::cfg::Config);
        const hg::cfg::ParseResult r = hg::cfg::parseConfig(reinterpret_cast<const uint8_t*>(kJson), sizeof(kJson) - 1, *c);
        if (r.ok && c->faces.size() == hg::motion::kRunCount) {
            parsed = std::move(c);
        } else {
            Serial.printf("drag timer overlay refused: %s\n", r.ok ? "wrong face count" : r.error.c_str());
        }
    }
    return parsed.get();
}

size_t face() {
    load();
    return runIndex;
}

void step(int delta) {
    load();
    const int n = int(hg::motion::kRunCount);
    runIndex = size_t(((int(runIndex) + delta) % n + n) % n);
    save();
}

}  // namespace drag_overlay
