#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "haltech.h"

// Plays a recording (recorder.h) back through the simulator: the simulator
// takes its values from here instead of its built-in patterns, so gauges on
// the bus and the page see the recorded drive. Follows the simulator's pause
// and speed. poll() runs in the main loop and reads the file outside the hub
// lock; values are swapped in under it, and value() is read under it.
namespace playback {

bool start(const char* name, bool loop, std::string& error);
void stop();
bool active();
// True while a recording is playing, or has finished and its last values are held.
bool value(hg::ChannelId id, float& out);
void poll(uint32_t nowMs);
// active, name, position_ms, duration_ms, loop, ended.
void statusJson(JsonObject out);
const char* currentName();

}  // namespace playback
