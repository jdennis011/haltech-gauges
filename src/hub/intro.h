#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

// The key-on intro across the gauges (docs/startup-animation.md): the hub's
// settings for it, the count of gauges it expects from last time, the order
// from the planned gauges, and the trigger once they have all announced
// themselves. The sequencing itself is lib/hg_intro.
namespace intro {

// After the manager and the planned gauges are up.
void begin();
// From the main loop; takes the hub lock itself.
void poll(uint32_t nowMs);
// The page's Play: the intro now, on every online gauge. How many were told.
size_t play();

// {enabled, style, stagger, duration, done, expected}
void settingsJson(JsonObject out);
// Any of enabled, style ("sweep", "chase", "fade"), stagger and duration (ms).
bool setSettings(JsonObjectConst in, std::string& error);

// The slot this gauge plays in, from 0, or -1. Call under the hub lock.
int slotOf(const uint8_t mac[6]);

}  // namespace intro
