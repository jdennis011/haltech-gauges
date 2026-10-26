#pragma once

#include <stddef.h>
#include <stdint.h>

#include "proto.h"

// What the hub does with a steering wheel module's buttons
// (docs/steering-wheel.md). Host-testable: the outputs are callbacks and
// time is passed in. One wheel at a time: the first node heard is the wheel
// until reset().
//
// The horn is the safety case. It follows the wheel's report only while the
// reports keep coming: an output held with nothing heard for `failSafeMs`
// is released, whatever the reason for the silence.
namespace hg {
namespace wheel {

constexpr size_t kLines = 6;  // ladder lines a wheel can report raw

struct Outputs {
    // Each is called on change only. Any may be null.
    void (*horn)(bool on, void* ctx);
    void (*cruise)(uint16_t mask, void* ctx);  // the kInputCruise* bits held
    void (*media)(uint16_t mask, void* ctx);   // the volume, seek, mode, mute and phone bits held
    void* ctx;
};

struct Settings {
    bool hornEnabled = false;
    bool cruiseEnabled = false;
    bool mediaEnabled = false;
    uint32_t failSafeMs = 100;   // an output held with no word from the wheel this long is released
    uint32_t linkLostMs = 1000;  // nothing from the wheel this long: "link lost" for the page
};

enum class Output : uint8_t { Horn = 0, Cruise = 1, Media = 2 };

struct State {
    bool seen = false;
    uint32_t node = 0;
    uint32_t lastInputMs = 0;
    uint16_t mask = 0;            // the buttons as last reported
    uint32_t changedMs[16] = {};  // when each button last went down or up
    uint8_t flags = 0;            // the last report's kInputFlag* bits
    uint8_t lines = 0;
    uint16_t rawMv[kLines] = {};
    uint32_t rawMs = 0;
    uint32_t seqGaps = 0;
    bool linkLost = false;
    uint32_t failSafeReleases = 0;
    // What the outputs are doing right now: the wheel's holds and the page's test pulses together.
    bool hornOn = false;
    uint16_t cruiseOn = 0;
    uint16_t mediaOn = 0;
};

class Controller {
public:
    void init(const Outputs& outputs, const Settings& settings);
    // An output just disabled goes off at once.
    void setSettings(const Settings& settings, uint32_t nowMs);
    const Settings& settings() const { return settings_; }

    void onInput(uint32_t node, const proto::Input& in, uint32_t nowMs);
    void onRaw(uint32_t node, const proto::InputRaw& raw, uint32_t nowMs);
    // Call often, every millisecond or so: the fail-safe and the test pulses live here.
    void poll(uint32_t nowMs);
    // A timed pulse of one output, from the page. For Cruise and Media `mask`
    // says which buttons; for Horn it is ignored. Refused while the wheel
    // holds that output or a pulse of it is already running.
    bool test(Output which, uint16_t mask, uint32_t ms, uint32_t nowMs);
    // The wheel went away (offline on the roster): everything off, forget it.
    void reset(uint32_t nowMs);

    const State& state() const { return state_; }

private:
    void apply();
    uint16_t wheelMask() const;  // what the wheel holds, as the settings let it through

    Outputs outputs_ = {};
    Settings settings_;
    State state_;
    uint8_t lastSeq_ = 0;
    uint16_t held_ = 0;  // the wheel's mask as applied to the outputs
    struct Pulse {
        bool active = false;
        uint16_t mask = 0;
        uint32_t untilMs = 0;
    } pulse_[3];
};

}  // namespace wheel
}  // namespace hg
