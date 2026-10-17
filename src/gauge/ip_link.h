#pragma once

#include <stdint.h>

#include "can_frame.h"

// The gauge bus over Wi-Fi (docs/ip-link.md): this gauge joins a network the
// hub is on and exchanges the bus's frames with it in UDP datagrams, as well
// as over CAN. For a gauge on the bench with no transceiver, or one too far
// for the chain. Set up from the serial console; kept in flash.
namespace ip_link {

// After wifi_fetch::begin(). Joins the network if a link is set.
void begin();
bool enabled();
bool connected();
// Saves and applies a link; an empty SSID switches it off.
void set(const char* ssid, const char* password, const char* host);

// From the CAN task: a frame for the hub; a frame from it, if one is waiting;
// and the housekeeping (joining, the keepalive), called every loop.
bool send(const hg::CanFrame& frame);
bool read(hg::CanFrame& out);
void service(uint32_t nowMs);

// "off", "joining HaltechGauges", "linked to 192.168.4.1"...
const char* status();

}  // namespace ip_link
