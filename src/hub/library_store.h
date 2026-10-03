#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "hub_manager.h"

// The hub's library of gauge configs, kept on LittleFS, and the table of
// which config each gauge (by MAC) should be running. Assigned configs are
// held in RAM so the manager can push them without touching flash.
namespace library_store {

// Mounts the filesystem and loads the assigned configs. False if the
// filesystem could not be mounted; everything then reports "not mounted".
bool begin();

bool validName(const char* name);
bool exists(const char* name);
// Where a config is stored on the filesystem, for streaming it out.
std::string filePath(const char* name);
// Validates the JSON with the gauge's own parser and stores it. On failure
// `error` says why, in the parser's words.
bool put(const char* name, const uint8_t* data, size_t size, std::string& error);
bool remove(const char* name);
bool read(const char* name, std::string& out);
// Appends one object per stored config: name, size, crc, title, faces.
void listJson(JsonArray out);

// mac is "AA:BB:CC:DD:EE:FF". An empty name clears the assignment. False if
// the config does not exist.
bool assign(const char* mac, const char* name);
// Copies the assigned config's name for this gauge into out; false if none.
bool assignmentFor(const uint8_t mac[6], char* out, size_t cap);
bool assignmentFor(const char* mac, std::string& out);
void assignmentsJson(JsonObject out);

// HubManager::Handler::desired.
bool desired(const hg::proto::HubManager::Gauge& gauge, hg::proto::HubManager::Desired& out, void*);

// Frees config copies that were replaced or removed. Call only while no
// push is in progress (hold the hub lock and check pushBusy()).
void collectGarbage();

}  // namespace library_store
