#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

// The head unit's wired remote (docs/steering-wheel.md), as a provision:
// four outputs on the carrier, each meant to switch a fitted resistor onto
// the remote line while a media button is held. Which button drives which
// output is a setting; the resistor values depend on the head unit, which
// is not chosen yet. Off until enabled on the page.
namespace swc_output {

constexpr int kOutputs = 4;

void begin();
void poll(uint32_t nowMs);
// The media buttons held (kInputVolUp and the rest): each mapped button
// holds its output.
void set(uint16_t mediaMask);
// {enabled, outputs:[{index, button, on}]}
void statusJson(JsonObject out);
// {enabled, map:{"vol_up":0,"vol_down":1,...}} with -1 or a missing name for unmapped.
bool setSettings(JsonVariantConst in, std::string& error);

}  // namespace swc_output
