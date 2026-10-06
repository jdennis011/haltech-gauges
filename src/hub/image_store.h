#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "xfer.h"

// The hub's library of images for the gauges: JPEG and PNG files on LittleFS,
// each up to 466 x 466 pixels, used by name in configs. The gauges get the ones
// their configs use over the bus; see docs/images.md.
namespace image_store {

constexpr uint32_t kMaxBytes = 256 * 1024;  // one image
constexpr size_t kMaxCount = 32;
// Left free on the filesystem for configs and recordings.
constexpr uint32_t kReserveBytes = 64 * 1024;

struct Info {
    uint32_t crc = 0;
    uint32_t size = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    bool png = false;
};

// After the filesystem is mounted.
void begin();

// Where an upload in progress is written.
const char* uploadPath();
// Space for an upload of `size` bytes: not too big, and room left on the filesystem.
bool roomFor(uint32_t size, std::string& error);
// Checks the uploaded file is an image a gauge can draw and stores it under
// `name`, replacing one of the same name. Removes the upload either way.
bool accept(const char* name, std::string& error);
bool remove(const char* name);

bool lookup(const char* name, Info& out);
// The file's path and type, for serving it to the page. False if there is none.
bool file(const char* name, std::string& path, const char*& contentType);
// One {name, size, crc, width, height, type} per image.
void listJson(JsonArray out);
// Changes on every add or remove, so the configs that use images are rebuilt.
uint32_t version();

// HubManager::Handler::openImage / closeImage: the image is read from flash as it goes out.
bool openForSend(uint32_t crc, hg::proto::XferSource& source, uint32_t& size, void*);
void closeSend(void*);

}  // namespace image_store
