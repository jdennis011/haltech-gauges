#pragma once

#include <stddef.h>
#include <stdint.h>

// Fetching over the hub's Wi-Fi (docs/images.md, "Over Wi-Fi"). The hub sends
// a WifiJoin message over CAN: its access point, where to fetch from, a token,
// and what to get: images by CRC, or a firmware build. This joins, downloads,
// stores and reports each item, installs firmware into the spare app slot,
// and leaves the network again. Wi-Fi is otherwise off.
namespace wifi_fetch {

void begin();
// A WifiJoin payload from the hub. Starts the job on its own task; false if
// one is already running or the payload is not understood.
bool start(const uint8_t* data, size_t size);
bool busy();
// What the job is doing, for the serial report.
const char* state();

}  // namespace wifi_fetch
