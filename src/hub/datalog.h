#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "channel_store.h"

// The datalog (docs/sd-card.md): every channel's value, ten times a second,
// as CSV on the SD card, a new file at each key-on, for as long as live data
// arrives. Unlike the recorder it needs no start button and has no length
// limit beyond the card.
//
// It runs on a task of its own, which also mounts the card: a card that
// stalls (timeouts on a marginal slot) must never hold up the main loop,
// whose watchdog would restart the hub.
namespace datalog {

constexpr uint32_t kRateHz = 10;

// Starts the task; `store` is read lock-free from it.
void begin(const hg::ChannelStore& store);

// Whether to log at all; kept in the hub's preferences (on).
bool enabled();
void setEnabled(bool on);
bool active();
// Asks the task to try mounting the card now (the page's Mount).
void requestMount();

bool validName(const char* name);
// The file's path on the card for a name like "2026-10-17_09-15-30.csv".
std::string path(const char* name);
bool remove(const char* name);

// {enabled, active, mounting, paused, error, file, rows, bytes, seconds}
void statusJson(JsonObject out);
// One {name, bytes, writing} per log on the card, newest first: the task's
// last reading of the directory, so the web never waits on the card.
void listJson(JsonArray out);

}  // namespace datalog
