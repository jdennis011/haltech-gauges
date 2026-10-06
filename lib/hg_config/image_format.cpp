#include "image_format.h"

#include <stdio.h>
#include <string.h>

#include "face_model.h"

namespace hg {
namespace cfg {

namespace {

uint16_t be16(const uint8_t* p) { return uint16_t((p[0] << 8) | p[1]); }
uint32_t be32(const uint8_t* p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3]; }

struct Memory {
    const uint8_t* data;
    size_t size;
};

bool readMemory(uint32_t offset, uint8_t* out, uint32_t len, void* ctx) {
    const Memory* m = static_cast<const Memory*>(ctx);
    if (offset > m->size || len > m->size - offset) return false;
    memcpy(out, m->data + offset, len);
    return true;
}

// Walks the JPEG's segments to its frame header. Only SOF0, baseline, is
// something TJpgDec decodes; the others are named so the message helps.
bool probeJpeg(ImageRead read, void* ctx, uint32_t size, ImageInfo& out, std::string& error) {
    uint32_t pos = 2;
    while (pos + 4 <= size) {
        uint8_t m[2];
        if (!read(pos, m, 2, ctx)) break;
        if (m[0] != 0xFF) break;
        const uint8_t marker = m[1];
        if (marker == 0xFF) {  // fill byte
            pos++;
            continue;
        }
        if (marker == 0xD8 || marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7)) {  // no length
            pos += 2;
            continue;
        }
        if (marker == 0xD9 || marker == 0xDA) break;  // end, or image data before any frame header
        uint8_t len[2];
        if (!read(pos + 2, len, 2, ctx)) break;
        const uint16_t segment = be16(len);
        if (segment < 2) break;

        const bool frame = marker >= 0xC0 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC;
        if (frame) {
            if (marker == 0xC2 || marker == 0xC6 || marker == 0xCA || marker == 0xCE) {
                error = "progressive JPEG: save it as a baseline (standard) JPEG";
                return false;
            }
            if (marker != 0xC0) {
                error = "this kind of JPEG (lossless or arithmetic-coded) is not supported: save it as a baseline JPEG";
                return false;
            }
            uint8_t sof[6];
            if (segment < 8 || !read(pos + 4, sof, 6, ctx)) break;
            if (sof[0] != 8) {
                error = "JPEG with 12-bit samples is not supported";
                return false;
            }
            if (sof[5] != 1 && sof[5] != 3) {
                error = "JPEG in CMYK is not supported: save it as RGB";
                return false;
            }
            out.type = ImageType::Jpeg;
            out.height = be16(sof + 1);
            out.width = be16(sof + 3);
            return true;
        }
        pos += 2u + segment;
    }
    error = "the JPEG is damaged or cut short";
    return false;
}

bool probePng(ImageRead read, void* ctx, uint32_t size, ImageInfo& out, std::string& error) {
    uint8_t ihdr[16];  // length, "IHDR", width, height
    if (size < 33 || !read(8, ihdr, sizeof(ihdr), ctx) || memcmp(ihdr + 4, "IHDR", 4) != 0) {
        error = "the PNG is damaged or cut short";
        return false;
    }
    const uint32_t w = be32(ihdr + 8), h = be32(ihdr + 12);
    out.type = ImageType::Png;
    out.width = uint16_t(w > 0xFFFF ? 0xFFFF : w);
    out.height = uint16_t(h > 0xFFFF ? 0xFFFF : h);
    return true;
}

}  // namespace

bool probeImage(ImageRead read, void* ctx, uint32_t size, ImageInfo& out, std::string& error) {
    out = ImageInfo();
    uint8_t magic[8];
    if (size < 8 || !read(0, magic, sizeof(magic), ctx)) {
        error = "the file is too short to be an image";
        return false;
    }
    bool ok;
    if (magic[0] == 0xFF && magic[1] == 0xD8 && magic[2] == 0xFF) {
        ok = probeJpeg(read, ctx, size, out, error);
    } else if (memcmp(magic, "\x89PNG\r\n\x1a\n", 8) == 0) {
        ok = probePng(read, ctx, size, out, error);
    } else {
        error = "not a JPEG or PNG file";
        return false;
    }
    if (!ok) return false;
    if (out.width == 0 || out.height == 0) {
        error = "the image has no pixels";
        return false;
    }
    if (out.width > kScreenSize || out.height > kScreenSize) {
        char buf[96];
        snprintf(buf, sizeof(buf), "the image is %u x %u: at most %d x %d pixels, the size of the screen",
                 unsigned(out.width), unsigned(out.height), kScreenSize, kScreenSize);
        error = buf;
        return false;
    }
    return true;
}

bool probeImage(const uint8_t* data, size_t size, ImageInfo& out, std::string& error) {
    Memory m = {data, size};
    return probeImage(readMemory, &m, uint32_t(size), out, error);
}

const char* imageTypeName(ImageType type) {
    switch (type) {
        case ImageType::Jpeg: return "jpeg";
        case ImageType::Png: return "png";
        default: return "";
    }
}

}  // namespace cfg
}  // namespace hg
