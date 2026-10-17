#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>

#include "can_frame.h"

// The gauge bus over Wi-Fi (docs/ip-link.md): gauges that join a network the
// hub is on and exchange the bus's frames with it in UDP datagrams, as if they
// were on the chain. Every frame the hub puts on the gauge bus goes to them
// too, and what they send joins the frames from the bus.
namespace ip_bus {

// After the Wi-Fi is up.
void begin();
// A frame from a linked gauge, if one is waiting. From the gauge bus task.
bool read(hg::CanFrame& out);
// Queues a frame for every linked gauge; false if none is linked. From any task.
bool send(const hg::CanFrame& frame);
// Sends what is queued, a few frames to a datagram. Call often from the gauge bus task.
void flush(uint32_t nowMs);
// Gauges heard from in the last ten seconds.
size_t peers();
// Whether this node id was last heard over Wi-Fi, for the page.
bool isIpNode(uint32_t node);
// {port, peers: [{ip, node, age_s}], rx_frames, tx_packets, dropped}
void statusJson(JsonObject out);

}  // namespace ip_bus
