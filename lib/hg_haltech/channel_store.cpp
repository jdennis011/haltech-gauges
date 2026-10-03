#include "channel_store.h"

#include <string.h>

namespace hg {

namespace {

struct SinkCtx {
    ChannelStore* store;
    uint32_t nowMs;
};

void storeSink(ChannelId id, float value, bool valid, void* ctx) {
    SinkCtx* c = static_cast<SinkCtx*>(ctx);
    c->store->set(id, value, valid, c->nowMs);
}

}  // namespace

int ChannelStore::onFrame(const CanFrame& frame, uint32_t nowMs) {
    SinkCtx ctx = {this, nowMs};
    return decodeFrame(frame, storeSink, &ctx);
}

void ChannelStore::set(ChannelId id, float value, bool valid, uint32_t nowMs) {
    value_[id] = value;
    stamp_[id] = nowMs;
    flags_[id] = uint8_t(kSeen | (valid ? kValid : 0));
}

uint32_t ChannelStore::staleAfterMs(ChannelId id) {
    // Five missed broadcasts, but never less than half a second: the rates
    // in Haltech's document are nominal, not guaranteed.
    const FrameDef* frame = findFrame(channelDef(id).canId);
    const uint32_t period = frame && frame->rateHz ? 1000u / frame->rateHz : 1000u;
    const uint32_t limit = period * 5;
    return limit < 500 ? 500 : limit;
}

bool ChannelStore::get(ChannelId id, uint32_t nowMs, float& value) const {
    const uint8_t flags = flags_[id];
    if (!(flags & kSeen) || !(flags & kValid)) return false;
    if (uint32_t(nowMs - stamp_[id]) > staleAfterMs(id)) return false;
    value = value_[id];
    return true;
}

void ChannelStore::clear() { memset(flags_, 0, sizeof(flags_)); }

}  // namespace hg
