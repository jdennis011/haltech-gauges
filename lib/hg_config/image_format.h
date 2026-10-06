#pragma once

// What kind of image a file is and how big, read from its header, so the hub
// can turn away a file the gauge could not draw before it crosses the bus.
// The gauge decodes JPEG with LVGL's TJpgDec, which takes baseline JPEG only,
// and PNG with lodepng, which takes any PNG.

#include <stddef.h>
#include <stdint.h>

#include <string>

namespace hg {
namespace cfg {

enum class ImageType : uint8_t { Unknown, Jpeg, Png };

struct ImageInfo {
    ImageType type = ImageType::Unknown;
    uint16_t width = 0;
    uint16_t height = 0;
};

// Copies `len` bytes from `offset` of the file into `out`. False past the end
// or on a read error.
typedef bool (*ImageRead)(uint32_t offset, uint8_t* out, uint32_t len, void* ctx);

// Reads the header of a `size`-byte file. On failure `error` says why, in
// words for the person who uploaded it.
bool probeImage(ImageRead read, void* ctx, uint32_t size, ImageInfo& out, std::string& error);
// The same for a file in memory.
bool probeImage(const uint8_t* data, size_t size, ImageInfo& out, std::string& error);

const char* imageTypeName(ImageType type);  // "jpeg", "png" or ""

}  // namespace cfg
}  // namespace hg
