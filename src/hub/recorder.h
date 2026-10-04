#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "channel_store.h"

// Records the live channel values to a file on the hub, ten times a second,
// writing only the values that changed, so a minute of driving is a hundred
// or two kilobytes. The file is JSON, one row per line, and plays back on the
// hub (playback.h) or in the page. Reads the channel store lock-free; call
// poll() from the main loop.
namespace recorder {

constexpr uint32_t kRateHz = 10;
constexpr uint32_t kMaxSeconds = 180;

void begin();  // after the filesystem is mounted

bool validName(const char* name);
bool exists(const char* name);
std::string filePath(const char* name);

bool start(const char* name, uint32_t seconds, std::string& error);
void stop();
bool active();
void poll(uint32_t nowMs, const hg::ChannelStore& store);

// active, name, elapsed_ms, seconds, rows, bytes.
void statusJson(JsonObject out, uint32_t nowMs);
// One {name, bytes, duration_ms} per recording on the filesystem.
void listJson(JsonArray out);
bool remove(const char* name);
// Length of a stored recording, from its trailer; 0 if it was cut short.
uint32_t durationOf(const char* path);
// Checks an uploaded file is a recording and moves it into place.
bool accept(const char* tempPath, const char* name, std::string& error);

}  // namespace recorder
