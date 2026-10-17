#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

// Gauge firmware builds kept on the hub (docs/firmware-updates.md, "Gauges"):
// one per gauge board, fetched from the site and served to a gauge over the
// hub's own Wi-Fi when the page asks for a gauge to be updated.
namespace gauge_firmware {

// A gauge board, as its Info message names it (hwRevision) and as the site
// keys its builds.
struct Board {
    uint8_t id;
    const char* key;   // the site's name for the build: /fw/<key>/
    const char* name;  // for the page
};
const Board* boards(size_t& count);
const Board* boardById(uint8_t id);

struct Build {
    std::string version;
    std::string path;  // the file on the hub's filesystem
    uint32_t size = 0;
    std::string sha256;
};

// After the filesystem is mounted.
void begin();
// The build the hub holds for a board, if any.
bool stored(uint8_t boardId, Build& out);
// True if the hub's build for the board is newer than this version.
bool newerThan(uint8_t boardId, uint8_t major, uint8_t minor, uint8_t patch);

// From the cloud task, once a second: a check of the site or a fetch that
// was asked for. One HTTPS session at a time with everything else there.
void service(uint32_t nowMs, bool online);
bool busy();
void requestCheck();
// Downloads the site's build for a board onto the hub. False with why.
bool requestFetch(uint8_t boardId, std::string& error);
bool remove(uint8_t boardId);

// {boards: [{id, key, name, stored: {version, size} | null, latest: {version, size, date, notes} | null}],
//  state, progress, error, checked_s}
void statusJson(JsonObject out);

}  // namespace gauge_firmware
