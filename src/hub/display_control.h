#pragma once

#include <stdint.h>

#include "hub_manager.h"
#include "proto.h"

// Brightness and screen on/off across the gauges. A gauge's own brightness
// is its own business (it keeps it in flash); what lives here is the
// "all gauges" state the hub re-applies to any gauge that comes online, and
// the handling of a gauge's request to change all of them from its pull-down
// menu. Call everything under hub::lock().
namespace display_control {

void begin();

// Brightness for every gauge, 0..255, or -1 when none has been set. Kept in
// the hub's preferences.
int allBrightness();
void setAllBrightness(int level);

// Every screen off (true) or on. Not kept across a hub restart: at key-on
// the screens come back.
bool displaysOff();
void setDisplaysOff(bool off);

// Sends a command to every online gauge; how many it reached.
size_t sendAll(hg::proto::Cmd cmd, uint8_t arg);

// HubManager::Handler callbacks.
void onOnline(const hg::proto::HubManager::Gauge& gauge, void* ctx);
void onRequest(const hg::proto::HubManager::Gauge& gauge, const hg::proto::Request& request, void* ctx);

}  // namespace display_control
