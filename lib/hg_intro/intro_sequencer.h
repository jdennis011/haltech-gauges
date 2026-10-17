#pragma once

// The key-on intro across the gauges (docs/startup-animation.md). The gauges
// share the hub's 5 V rail and boot with it, dark. The hub waits for the ones
// it expects to announce themselves, then tells each its slot: a delay, a
// duration and a style. CAN delivers that in well under a millisecond, so the
// gauges play one after another without a shared clock. Host-testable; the
// hub adds the settings, the saved count and the order from its planned gauges.

#include <stddef.h>
#include <stdint.h>

#include "hub_manager.h"
#include "proto.h"

namespace hg {
namespace intro {

struct Settings {
    bool enabled = true;
    proto::IntroStyle style = proto::IntroStyle::Sweep;
    uint16_t staggerMs = 250;    // between one gauge's start and the next
    uint16_t durationMs = 1200;  // each gauge's own intro
};

// A gauge the hub expects, by MAC, in display order.
struct Slot {
    uint8_t mac[6];
};

class Sequencer {
public:
    // With some but not all of the expected gauges heard, the intro goes
    // this long after the first hello...
    static constexpr uint32_t kSettleMs = 1500;
    // ...and with none at all, the hub stops waiting this long after start.
    static constexpr uint32_t kGiveUpMs = 8000;
    static constexpr size_t kMaxSlots = proto::HubManager::kMaxGauges;

    // At the hub's start. `expected` is how many gauges should turn up: the
    // planned gauges linked to hardware, or the count seen last time,
    // whichever is more; 0 means wait the settle time after the first hello.
    void begin(uint32_t nowMs, size_t expected);
    void setSettings(const Settings& s) { settings_ = s; }
    const Settings& settings() const { return settings_; }
    // The display order: these first, as given; any other gauge after them
    // in the order it announced itself.
    void setOrder(const Slot* slots, size_t count);

    // Call about ten times a second. Sends the intro once the gauges have
    // settled; true on the run it did. Nothing goes when disabled, and a
    // gauge that announces itself later plays its own.
    bool poll(proto::HubManager& manager, uint32_t nowMs);
    // Plays it now on every online gauge, in order, whatever they have played
    // since they started (the page's Play). How many were told.
    size_t play(proto::HubManager& manager, uint32_t nowMs);

    // The key-on intro has gone, or the wait for it is over.
    bool done() const { return done_; }
    size_t expected() const { return expected_; }
    // The slot a gauge plays in, from 0, or -1 when it is not online.
    int slotOf(const proto::HubManager& manager, const uint8_t mac[6]) const;

private:
    struct Ranked {
        uint32_t node;
        uint32_t rank;
        const uint8_t* mac;
    };
    size_t ranked(const proto::HubManager& manager, Ranked* out) const;
    size_t send(proto::HubManager& manager, bool force);

    Settings settings_;
    Slot order_[kMaxSlots] = {};
    size_t orderCount_ = 0;
    uint32_t startMs_ = 0;
    uint32_t firstHelloMs_ = 0;
    bool anyHello_ = false;
    size_t expected_ = 0;
    bool done_ = false;
};

}  // namespace intro
}  // namespace hg
