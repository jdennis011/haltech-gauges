#include "update_manifest.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <ArduinoJson.h>

namespace hg {
namespace update {

bool parseVersion(const char* text, Version& out) {
    if (!text) return false;
    if (*text == 'v' || *text == 'V') text++;
    int parts[3] = {0, 0, 0};
    int count = 0;
    const char* p = text;
    while (count < 3) {
        if (!isdigit((unsigned char)*p)) return false;
        char* end = nullptr;
        const long v = strtol(p, &end, 10);
        if (v < 0 || v > 9999) return false;
        parts[count++] = int(v);
        p = end;
        if (*p == 0) break;
        if (*p != '.') return false;
        p++;
    }
    if (*p != 0 || count < 2) return false;
    out.major = parts[0];
    out.minor = parts[1];
    out.patch = parts[2];
    return true;
}

int compare(const Version& a, const Version& b) {
    if (a.major != b.major) return a.major < b.major ? -1 : 1;
    if (a.minor != b.minor) return a.minor < b.minor ? -1 : 1;
    if (a.patch != b.patch) return a.patch < b.patch ? -1 : 1;
    return 0;
}

bool newerThan(const char* candidate, const char* current) {
    Version a, b;
    if (!parseVersion(candidate, a) || !parseVersion(current, b)) return false;
    return compare(a, b) > 0;
}

namespace {

bool isSha256Hex(const std::string& s) {
    if (s.size() != 64) return false;
    for (char c : s) {
        if (!isxdigit((unsigned char)c)) return false;
    }
    return true;
}

}  // namespace

bool parseManifest(const uint8_t* data, size_t len, const char* board, Manifest& out, std::string& error) {
    JsonDocument doc;
    if (deserializeJson(doc, data, len) != DeserializationError::Ok || !doc.is<JsonObject>()) {
        error = "the manifest is not JSON";
        return false;
    }
    JsonObjectConst o = doc.as<JsonObjectConst>();
    const char* forBoard = o["board"] | "";
    if (!board || strcmp(forBoard, board) != 0) {
        error = std::string("the manifest is for '") + forBoard + "', this hub is '" + (board ? board : "") + "'";
        return false;
    }
    const char* version = o["version"] | "";
    Version v;
    if (!parseVersion(version, v)) {
        error = std::string("'version' is not a version: '") + version + "'";
        return false;
    }
    if (!o["size"].is<uint32_t>() || o["size"].as<uint32_t>() == 0) {
        error = "'size' is missing";
        return false;
    }
    std::string sha = o["sha256"] | "";
    for (char& c : sha) c = char(tolower((unsigned char)c));
    if (!isSha256Hex(sha)) {
        error = "'sha256' is not 64 hex characters";
        return false;
    }
    out.version = version;
    out.board = forBoard;
    out.size = o["size"].as<uint32_t>();
    out.sha256 = sha;
    out.date = o["date"] | "";
    out.notes = o["notes"] | "";
    return true;
}

}  // namespace update
}  // namespace hg
