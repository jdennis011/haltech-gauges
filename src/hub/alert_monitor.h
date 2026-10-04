#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "alert_rules.h"
#include "channel_store.h"

// Alerts on the hub: the rules kept in /alerts.json, checked against the live
// data ten times a second and carried out on the gauges - a message over the
// face, a switch to another face, or both. docs/alerts.md has the details.
namespace alert_monitor {

constexpr size_t kMaxRules = hg::alerts::kMaxRules;

// Loads the stored rules (library_store::begin() must have mounted the filesystem).
void begin();

// Checks the rules and sends the gauges whatever they need. Call often,
// under hub::lock().
void poll(uint32_t nowMs, const hg::ChannelStore& store);

// The rest take hub::lock() themselves: do not hold it when calling them.

// Replaces the rules with a JSON list and stores them. Nothing changes if any
// rule is wrong; `error` then says which and why.
bool setRules(const uint8_t* data, size_t len, std::string& error);
void rulesJson(JsonArray out);
// One {active, count, value} per rule, in order.
void statesJson(JsonArray out);
// Puts a rule in force for `seconds` whatever its condition says.
bool test(size_t index, uint32_t seconds);
size_t ruleCount();
// Changes whenever an alert fires or ends, or the rules are replaced.
uint32_t version();

}  // namespace alert_monitor
