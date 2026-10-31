#pragma once

#include <stdint.h>

#include <ArduinoJson.h>

#include "hub_manager.h"

// Drag timer runs the gauges report (docs/motion.md): the latest fifty kept
// for the page's Runs card, each also appended to runs.csv on the SD card.
namespace runs {

constexpr size_t kKeep = 50;

// HubManager handler callback, under hub::lock().
void onRun(const hg::proto::HubManager::Gauge& gauge, const hg::proto::RunReport& report, void* ctx);

// Changes whenever the list does.
uint32_t version();
// Newest first: [{seq, at_s, time, node, mac, run, name, aborted, complete,
// total_ms, splits: [{name, ms, kmh}]}]. Under hub::lock().
void listJson(JsonArray out);
// Forgets the list (not the file on the card). Under hub::lock().
void clear();

}  // namespace runs
