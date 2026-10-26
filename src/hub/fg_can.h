#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

// The vehicle CAN bus to the Nexus Rebel's CAN 2, speaking Ford FG Falcon
// (docs/steering-wheel.md): the cruise buttons go out as the frame table in
// lib/hg_wheel/fg_cruise, repeated at its rate while enabled. Only the P4
// hub has the third CAN controller for it; the others answer "not on this
// board". The port can also listen only, keep a table of what it hears, and
// probe a candidate id bit by bit while the Haltech's own cruise channels
// are watched, to find the frame the ECU reads.
namespace fg_can {

void begin();  // settings from the preferences; the port starts if enabled
void poll(uint32_t nowMs);
// The cruise buttons held (kInputCruise... bits): sent at once, then repeated.
void setCruise(uint16_t mask);
bool available();  // this board has the port

// {available, enabled, kbps, send_idle, listen, state, tx, rx, errors, table:{...},
//  capture:[{id, count, period_ms, data}], probe:{running, id, bit, hits:[{bit, channel, from, to}]}}
void statusJson(JsonObject out);
// Any of enabled, kbps, send_idle, table (the frame table JSON).
bool setSettings(JsonVariantConst in, std::string& error);
// Listen only on or off (the transceiver's silent pin; nothing is sent or acknowledged).
bool setListen(bool on, std::string& error);
// Starts probing `id` (0 stops): every bit of an idle frame in turn, 250 ms
// each, watching cruise_state and cruise_input_state on the Haltech bus.
bool probe(uint32_t id, bool extended, std::string& error);

}  // namespace fg_can
