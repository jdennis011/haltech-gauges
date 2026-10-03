#pragma once

#include <stddef.h>
#include <stdint.h>

#include "can_frame.h"

namespace hg {

// Unit a channel's decoded value is stored in.
enum class Unit : uint8_t {
    Raw,
    Bool,
    Enum,
    Rpm,
    Kpa,     // gauge pressure
    KpaAbs,  // absolute pressure
    Percent,
    Degrees,
    Kmh,
    Millisec,
    Lambda,
    Decibel,
    Mps2,
    CcPerMin,
    Volts,
    Celsius,
    Ppm,
    GramPerM3,
    Litres,
    KmPerLitre,
    LitrePer100Km,
    Seconds,
    Millimetres,
    MmPerSec,
    DegPerSec,
    Cc,
    Metres,
};

enum ChannelId : uint16_t {
#define HG_FRAME(id, hz)
#define HG_CH(name, label, id, byte, bit, width, sgn, scale, offset, unit, zerr) CH_##name,
#include "haltech_channels_x.h"
    CH_COUNT
};

struct ChannelDef {
    const char* name;   // identifier used in face configs, e.g. "oil_pressure"
    const char* label;  // human-readable name for the builder UI
    float scale;
    float offset;
    uint16_t canId;
    uint8_t startBit;  // MSB of the field, counted from bit 7 of byte 0
    uint8_t width;     // bits
    bool isSigned;
    bool zeroIsError;  // raw 0 means the sensor reported an error
    Unit unit;
};

struct FrameDef {
    uint16_t canId;
    uint8_t rateHz;
    uint16_t firstChannel;
    uint16_t channelCount;
};

const ChannelDef& channelDef(ChannelId id);
// Returns the channel with this name, or -1.
int findChannel(const char* name);
const char* unitSymbol(Unit unit);

size_t frameCount();
const FrameDef& frameDef(size_t index);
// Returns nullptr if the ID is not a decoded Haltech frame.
const FrameDef* findFrame(uint32_t canId);

typedef void (*ChannelSink)(ChannelId id, float value, bool valid, void* ctx);

// Decodes every channel carried by a Haltech broadcast frame and reports each
// through sink. Returns the number of channels reported; 0 for extended
// frames and unknown IDs.
int decodeFrame(const CanFrame& frame, ChannelSink sink, void* ctx);

// Writes one channel's value into a frame's data, for the simulator and
// tests. The caller sets up the frame with beginFrame() first. Values outside
// the field's range are clamped.
void beginFrame(const FrameDef& def, CanFrame& frame);
bool encodeChannel(ChannelId id, float value, CanFrame& frame);

}  // namespace hg
