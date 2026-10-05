#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "hub_manager.h"

// The hub's eight theme colours, kept in /theme-colours.json and shared by
// every config and gauge: a widget coloured "themecolour1" is drawn in whatever
// colour 1 is here. The gauges get them over the bus when they change, when a
// gauge comes online, and every few seconds. docs/config-schema.md has the format.
namespace theme_colours {

// Loads the stored colours (library_store::begin() must have mounted the filesystem).
void begin();

// Sends the colours to the gauges when they have changed and every
// kThemeColourRepeatMs. Call often, under hub::lock().
void poll(uint32_t nowMs);

// HubManager::Handler::online: a gauge that has just appeared gets them at once.
void onOnline(const hg::proto::HubManager::Gauge& gauge, void* ctx);

// The rest take hub::lock() themselves: do not hold it when calling them.

// Replaces the colours with a JSON list of eight {value, name} and stores them.
// Nothing changes if the list is wrong; `error` then says why.
bool set(const uint8_t* data, size_t len, std::string& error);
// Appends {value: "#RRGGBB", name} for each colour.
void json(JsonArray out);
// Changes whenever the colours do.
uint32_t version();

}  // namespace theme_colours
