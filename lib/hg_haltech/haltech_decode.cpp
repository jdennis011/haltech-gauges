#include <string.h>

#include "haltech.h"

namespace hg {

namespace {

const ChannelDef kChannels[] = {
#define HG_FRAME(id, hz)
#define HG_CH(name, label, id, byte, bit, width, sgn, scale, offset, unit, zerr) \
    {#name, label, scale, offset, id, uint8_t((byte) * 8 + (7 - (bit))), width, sgn, zerr, Unit::unit},
#include "haltech_channels_x.h"
};

struct FrameRate {
    uint16_t canId;
    uint8_t rateHz;
};

const FrameRate kFrameRates[] = {
#define HG_FRAME(id, hz) {id, hz},
#define HG_CH(name, label, id, byte, bit, width, sgn, scale, offset, unit, zerr)
#include "haltech_channels_x.h"
};

constexpr size_t kFrameCount = sizeof(kFrameRates) / sizeof(kFrameRates[0]);

struct FrameIndex {
    FrameDef frames[kFrameCount];

    FrameIndex() {
        size_t ch = 0;
        for (size_t i = 0; i < kFrameCount; i++) {
            FrameDef& f = frames[i];
            f.canId = kFrameRates[i].canId;
            f.rateHz = kFrameRates[i].rateHz;
            f.firstChannel = uint16_t(ch);
            while (ch < CH_COUNT && kChannels[ch].canId == f.canId) ch++;
            f.channelCount = uint16_t(ch - f.firstChannel);
        }
    }
};

const FrameIndex& frameIndex() {
    static const FrameIndex index;
    return index;
}

bool extractRaw(const ChannelDef& d, const CanFrame& frame, int64_t& out) {
    const unsigned endBit = unsigned(d.startBit) + d.width;
    if (endBit > unsigned(frame.len) * 8) return false;

    uint64_t be = 0;
    for (int i = 0; i < 8; i++) be = (be << 8) | frame.data[i];

    uint64_t raw = (be >> (64 - endBit)) & ((uint64_t(1) << d.width) - 1);
    if (d.isSigned && ((raw >> (d.width - 1)) & 1)) {
        out = int64_t(raw) - (int64_t(1) << d.width);
    } else {
        out = int64_t(raw);
    }
    return true;
}

}  // namespace

const ChannelDef& channelDef(ChannelId id) { return kChannels[id]; }

int findChannel(const char* name) {
    if (!name) return -1;
    for (int i = 0; i < CH_COUNT; i++) {
        if (strcmp(kChannels[i].name, name) == 0) return i;
    }
    return -1;
}

const char* unitSymbol(Unit unit) {
    switch (unit) {
        case Unit::Raw:
        case Unit::Bool:
        case Unit::Enum: return "";
        case Unit::Rpm: return "rpm";
        case Unit::Kpa:
        case Unit::KpaAbs: return "kPa";
        case Unit::Percent: return "%";
        case Unit::Degrees: return "\xC2\xB0";
        case Unit::Kmh: return "km/h";
        case Unit::Millisec: return "ms";
        case Unit::Lambda: return "\xCE\xBB";
        case Unit::Decibel: return "dB";
        case Unit::Mps2: return "m/s\xC2\xB2";
        case Unit::CcPerMin: return "cc/min";
        case Unit::Volts: return "V";
        case Unit::Celsius: return "\xC2\xB0" "C";
        case Unit::Ppm: return "ppm";
        case Unit::GramPerM3: return "g/m\xC2\xB3";
        case Unit::Litres: return "L";
        case Unit::KmPerLitre: return "km/L";
        case Unit::LitrePer100Km: return "L/100km";
        case Unit::Seconds: return "s";
        case Unit::Millimetres: return "mm";
        case Unit::MmPerSec: return "mm/s";
        case Unit::DegPerSec: return "\xC2\xB0/s";
        case Unit::Cc: return "cc";
        case Unit::Metres: return "m";
    }
    return "";
}

size_t frameCount() { return kFrameCount; }

const FrameDef& frameDef(size_t index) { return frameIndex().frames[index]; }

const FrameDef* findFrame(uint32_t canId) {
    const FrameDef* frames = frameIndex().frames;
    size_t lo = 0, hi = kFrameCount;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (frames[mid].canId < canId) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return (lo < kFrameCount && frames[lo].canId == canId) ? &frames[lo] : nullptr;
}

int decodeFrame(const CanFrame& frame, ChannelSink sink, void* ctx) {
    if (frame.extended) return 0;
    const FrameDef* def = findFrame(frame.id);
    if (!def) return 0;

    int reported = 0;
    for (uint16_t i = 0; i < def->channelCount; i++) {
        const ChannelId id = ChannelId(def->firstChannel + i);
        const ChannelDef& d = kChannels[id];
        int64_t raw;
        if (!extractRaw(d, frame, raw)) continue;
        const bool valid = !(d.zeroIsError && raw == 0);
        const float value = float(double(raw) * double(d.scale) + double(d.offset));
        sink(id, value, valid, ctx);
        reported++;
    }
    return reported;
}

}  // namespace hg
