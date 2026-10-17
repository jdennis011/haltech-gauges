#include "ip_frames.h"

#include <string.h>

namespace hg {

size_t packIpFrames(const CanFrame* frames, size_t count, uint8_t* out, size_t cap) {
    if (count > kIpMaxFrames || cap < kIpHeaderBytes + count * kIpFrameBytes) return 0;
    out[0] = 'H';
    out[1] = 'G';
    out[2] = kIpBusVersion;
    out[3] = uint8_t(count);
    uint8_t* p = out + kIpHeaderBytes;
    for (size_t i = 0; i < count; i++) {
        const CanFrame& f = frames[i];
        const uint32_t id = (f.id & 0x1FFFFFFFu) | (f.extended ? 0x80000000u : 0u);
        p[0] = uint8_t(id >> 24);
        p[1] = uint8_t(id >> 16);
        p[2] = uint8_t(id >> 8);
        p[3] = uint8_t(id);
        p[4] = f.len > 8 ? 8 : f.len;
        memset(p + 5, 0, 8);
        memcpy(p + 5, f.data, p[4]);
        p += kIpFrameBytes;
    }
    return kIpHeaderBytes + count * kIpFrameBytes;
}

int unpackIpFrames(const uint8_t* data, size_t len, CanFrame* out, size_t cap) {
    if (len < kIpHeaderBytes || data[0] != 'H' || data[1] != 'G' || data[2] != kIpBusVersion) return -1;
    const size_t count = data[3];
    if (count > kIpMaxFrames || len < kIpHeaderBytes + count * kIpFrameBytes) return -1;
    const uint8_t* p = data + kIpHeaderBytes;
    size_t n = 0;
    for (size_t i = 0; i < count && n < cap; i++) {
        CanFrame f = {};
        const uint32_t id = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
        f.extended = (id & 0x80000000u) != 0;
        f.id = id & 0x1FFFFFFFu;
        f.len = p[4] > 8 ? 8 : p[4];
        memcpy(f.data, p + 5, 8);
        out[n++] = f;
        p += kIpFrameBytes;
    }
    return int(n);
}

}  // namespace hg
