#pragma once

#include "haltech.h"

namespace hg {

// Latest decoded value of every channel, with enough bookkeeping to tell a
// live reading from one that has stopped arriving.
//
// Written by the CAN task and read by the UI task without a lock: each field
// is a single 32-bit (or smaller) store, and a reader seeing a new value with
// the previous timestamp for one frame is harmless.
class ChannelStore {
public:
    // Feeds one received frame in. Returns the number of channels updated.
    int onFrame(const CanFrame& frame, uint32_t nowMs);

    void set(ChannelId id, float value, bool valid, uint32_t nowMs);

    // False if the channel has never arrived, the ECU flagged it as an error,
    // or it has gone stale (no update for several of its broadcast periods).
    bool get(ChannelId id, uint32_t nowMs, float& value) const;

    // How long a channel may go without an update before it counts as stale.
    static uint32_t staleAfterMs(ChannelId id);

    void clear();

private:
    enum : uint8_t { kSeen = 1, kValid = 2 };

    float value_[CH_COUNT] = {};
    uint32_t stamp_[CH_COUNT] = {};
    uint8_t flags_[CH_COUNT] = {};
};

}  // namespace hg
