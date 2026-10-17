#pragma once

// Firmware updates: the version scheme and the manifest the site serves for
// each hub board (docs/firmware-updates.md). Host-testable; the hub adds the
// download and the flashing (src/hub/firmware_update.cpp).

#include <stddef.h>
#include <stdint.h>

#include <string>

namespace hg {
namespace update {

struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;
};

// "1.2.3" or "v1.2.3"; a missing patch reads as 0. False if it is not a version.
bool parseVersion(const char* text, Version& out);
// Negative, zero or positive as `a` is older than, the same as, or newer than `b`.
int compare(const Version& a, const Version& b);
// True if `candidate` is a version newer than `current`; false for anything unparsable.
bool newerThan(const char* candidate, const char* current);

struct Manifest {
    std::string version;
    std::string board;   // the chip the build is for: "esp32c6", "esp32p4"
    uint32_t size = 0;   // bytes of firmware.bin
    std::string sha256;  // 64 hex characters, lower case
    std::string date;    // "2026-10-16", for the page
    std::string notes;   // what changed, for the page
};

// Parses the manifest for one board. False, with the reason, if it is not for
// `board`, has no version, size or SHA-256, or is not JSON.
bool parseManifest(const uint8_t* data, size_t len, const char* board, Manifest& out, std::string& error);

}  // namespace update
}  // namespace hg
