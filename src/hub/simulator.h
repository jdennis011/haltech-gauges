#pragma once

#include <stdint.h>

#include "can_ports.h"
#include "channel_store.h"

// Stands in for the ECU on the bench: generates the Haltech broadcast frames
// at their documented rates with plausible moving values.
namespace simulator {

void setEnabled(bool enabled);
bool enabled();

// Generates whatever frames are due. Every frame is decoded into `store`, so
// the web UI sees the data, and sent on the bus when `port` is given. Call
// about once a millisecond.
void poll(uint32_t nowMs, hg::ChannelStore& store, GaugePort* port);

// Frames generated so far, whether or not a gauge was there to receive them.
uint32_t framesGenerated();

}  // namespace simulator
