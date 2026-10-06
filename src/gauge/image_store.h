#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

// Images the hub sends, kept on the gauge's LittleFS as /img/<CRC32>.jpg or
// .png: the CRC of a file's bytes is its name, as the hub's config map uses.
// Files are written by a task of their own, so the CAN task is never held up
// by flash; an image counts as stored from the moment it is handed over.
namespace image_store {

// Mounts the filesystem (formatting it if it will not mount) and lists the images.
bool begin();

bool has(uint32_t crc);
// Room for an image of `size` bytes, after the ones still being written.
bool roomFor(uint32_t size);
// Takes ownership of `data` (from malloc or heap_caps_malloc) and writes it in
// the background. False if the writer is backed up; `data` is freed either way.
bool submit(uint8_t* data, uint32_t size);
// The file of a stored image, e.g. "/img/1A2B3C4D.jpg"; empty if there is none.
std::string path(uint32_t crc);
// Deletes the images not in the list. Called on storing a config, with the
// CRCs from its "images" map, so the gauge keeps what its config uses.
void keepOnly(const uint32_t* crcs, size_t count);

}  // namespace image_store
