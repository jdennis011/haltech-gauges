#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

// Firmware updates for the hub over Wi-Fi (docs/firmware-updates.md): a check
// of the site's manifest for this board, a download into the spare app slot
// with the SHA-256 checked, and a restart. The bootloader comes back to this
// build if the new one does not run. The page can also upload a .bin directly.
namespace firmware_update {

// The chip this build is for, as the site names its builds: "esp32c6", "esp32p4"...
const char* board();

void begin();
// From the main loop: confirms a freshly installed build once it has run a
// while, and restarts the hub after an install.
void poll(uint32_t nowMs);
// From the cloud task, once a second: carries out a check or an install that
// was asked for, and the daily check. One HTTPS session at a time with the sync.
void service(uint32_t nowMs, bool online);
// A download or an upload is writing the spare slot: the sync stays out of the way.
bool busy();

void requestCheck();
// False, with why, if there is nothing to install or something is running.
bool requestInstall(std::string& error);

// A .bin uploaded through the page, streamed in pieces from the web server.
bool uploadBegin(size_t total, std::string& error);
bool uploadWrite(const uint8_t* data, size_t len);
// True if the image is complete and good; the hub then restarts on its own.
bool uploadEnd(std::string& error);
void uploadAbort();

void statusJson(JsonObject out);

// A GET with the certificate bundle; the body goes to `sink` piece by piece.
// Returns the HTTP status, or -1 with `why` when no reply came. For the cloud
// task only, like everything else that opens a TLS session.
typedef bool (*HttpSink)(const uint8_t* data, size_t len, void* ctx);
int fetch(const char* url, HttpSink sink, void* ctx, std::string& why, int64_t& length);

}  // namespace firmware_update
