#include <math.h>
#include <string.h>

#include "haltech.h"

namespace hg {

void beginFrame(const FrameDef& def, CanFrame& frame) {
    frame.id = def.canId;
    frame.extended = false;
    frame.len = 8;
    memset(frame.data, 0, sizeof(frame.data));
}

bool encodeChannel(ChannelId id, float value, CanFrame& frame) {
    const ChannelDef& d = channelDef(id);
    if (frame.extended || frame.id != d.canId || frame.len != 8) return false;

    double rawF = round((double(value) - double(d.offset)) / double(d.scale));
    const double maxRaw = d.isSigned ? double((int64_t(1) << (d.width - 1)) - 1)
                                     : double((uint64_t(1) << d.width) - 1);
    const double minRaw = d.isSigned ? -double(int64_t(1) << (d.width - 1)) : 0.0;
    if (rawF > maxRaw) rawF = maxRaw;
    if (rawF < minRaw) rawF = minRaw;

    const uint64_t mask = (uint64_t(1) << d.width) - 1;
    const uint64_t raw = uint64_t(int64_t(rawF)) & mask;
    const unsigned shift = 64 - (unsigned(d.startBit) + d.width);

    uint64_t be = 0;
    for (int i = 0; i < 8; i++) be = (be << 8) | frame.data[i];
    be = (be & ~(mask << shift)) | (raw << shift);
    for (int i = 7; i >= 0; i--) {
        frame.data[i] = uint8_t(be & 0xFF);
        be >>= 8;
    }
    return true;
}

}  // namespace hg
