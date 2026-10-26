#pragma once

#include <stddef.h>

#include <ArduinoJson.h>
#include <string>

// Named sets of the simulator's speed, paused state and overrides, kept on
// the hub's filesystem (docs/web-ui.md, "Simulator"): a bench scenario saved
// under a name and brought back with one click, and carried in Export all.
namespace sim_presets {

constexpr size_t kMax = 16;

bool validName(const char* name);
// [{name, speed, paused, overrides:[{channel, hold} or {channel, min, max}]}]
void listJson(JsonArray out);
// Saves `body` ({speed, paused, overrides} with overrides as a list or as the
// POST /api/simulator map) under `name`, or the settings in force when the
// body has neither speed nor overrides. Replaces a preset of the same name.
bool save(const char* name, JsonVariantConst body, std::string& error);
// Applies a preset to the simulator (clears the overrides first).
bool apply(const char* name, std::string& error);
bool remove(const char* name);

}  // namespace sim_presets
