#pragma once

#include <stddef.h>
#include <stdint.h>

// A firmware update over the hub's Wi-Fi (docs/firmware-updates.md,
// "Gauges"): the hub sends a WifiJoin message over the bus with its access
// point, a token and the build's size and SHA-256; this joins, streams the
// image into the spare app slot, checks it, leaves and asks for a restart.
// The gauge's module without its image fetching. Wi-Fi is otherwise off.
namespace wifi_fetch {

void begin();
bool start(const uint8_t* data, size_t size);
bool busy();
const char* state();

}  // namespace wifi_fetch
