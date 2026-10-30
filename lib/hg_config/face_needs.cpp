#include "face_needs.h"

#include "haltech.h"

namespace hg {
namespace cfg {

void framesUsed(const Config& cfg, uint64_t* parts, size_t partCount, unsigned partBits) {
    for (size_t p = 0; p < partCount; p++) parts[p] = 0;
    const size_t frames = frameCount();
    for (const Face& face : cfg.faces) {
        for (const Widget& w : face.widgets) {
            if (w.channel < 0 || size_t(w.channel) >= CH_COUNT) continue;
            const size_t index = frameIndexOf(channelDef(ChannelId(w.channel)).canId);
            if (index >= frames) continue;
            const size_t p = index / partBits;
            if (p < partCount) parts[p] |= uint64_t(1) << (index % partBits);
        }
    }
}

}  // namespace cfg
}  // namespace hg
