#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "can_ports.h"
#include "channel_store.h"
#include "haltech.h"

// Stands in for the ECU on the bench: generates the Haltech broadcast frames
// at their documented rates with plausible moving values. The values follow a
// clock of their own that can be paused or run slower and faster, and any
// channel can be held at a value or swept between limits of your choosing.
// All of this is touched under hub::lock().
namespace simulator {

// Restores on, paused, speed and the overrides from the hub's preferences;
// every setter below saves them, so they survive a restart.
void load();

void setEnabled(bool enabled);
bool enabled();

// Freezes the values; frames keep going out so gauges keep showing them.
void setPaused(bool paused);
bool paused();

// How fast the values move: 1 is the built-in pace, 0.1 is ten times slower.
void setSpeed(float speed);
float speed();

// Per-channel overrides, in the channel's stored unit (kPa, degrees C, km/h).
// A hold keeps the channel at `value`; otherwise it sweeps min..max.
constexpr size_t kMaxOverrides = 16;
bool setOverride(hg::ChannelId id, bool hold, float value, float min, float max);
bool clearOverride(hg::ChannelId id);
void clearOverrides();
size_t overrideCount();
// Appends {channel, hold} or {channel, min, max} per override.
void overridesJson(JsonArray out);
// Applies {paused, speed, clear, overrides}: the overrides as the API's map
// {"rpm": {"hold": 3000}, "gear": null} or as a list [{channel, hold|min,max}]
// (a preset, an export). False with a reason, part way applied.
bool applyJson(JsonVariantConst in, std::string& error);

// Generates whatever frames are due. Every frame is decoded into `store`, so
// the web UI sees the data, and sent on the bus when `port` is given and
// `wanted` (a frame table index) says yes, or is null. Call about once a millisecond.
void poll(uint32_t nowMs, hg::ChannelStore& store, GaugePort* port, bool (*wanted)(size_t frameIndex) = nullptr);

// Frames generated so far, whether or not a gauge was there to receive them.
uint32_t framesGenerated();

}  // namespace simulator
