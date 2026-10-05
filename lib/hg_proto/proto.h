#pragma once

// Management protocol between the hub and the gauges on the private gauge bus.
//
// Haltech live data uses 11-bit IDs; everything here uses 29-bit extended IDs,
// so the two can never collide:
//
//   bits 28..25  message type
//   bit  24      direction (0 = hub to gauge, 1 = gauge to hub)
//   bits 23..0   gauge node id (kBroadcastNode = every gauge)
//
// A gauge's node id is the low 24 bits of its MAC unless it has re-rolled one
// after seeing a duplicate. Multi-byte fields are big-endian.

#include <stddef.h>
#include <stdint.h>

#include "can_frame.h"

namespace hg {
namespace proto {

constexpr uint8_t kProtocolVersion = 1;
constexpr uint32_t kBroadcastNode = 0xFFFFFF;

// A gauge stays silent unless it heard a beacon this recently.
constexpr uint32_t kBeaconPeriodMs = 500;
constexpr uint32_t kBeaconTimeoutMs = 3000;
constexpr uint32_t kStatusPeriodMs = 1000;
// The hub drops a gauge from its roster after this long without a status.
constexpr uint32_t kGaugeTimeoutMs = 3000;

enum class Msg : uint8_t {
    Beacon = 0,
    Hello = 1,
    Info = 2,
    Status = 3,
    Command = 4,
    CommandAck = 5,
    Alert = 6,  // hub to gauge: a message to show over the face, in up to six frames
    ThemeColours = 7,  // hub to gauges: the hub's theme colours, in four frames
    XferBegin = 8,
    XferData = 9,
    XferEnd = 10,
    XferAck = 11,
    XferAbort = 12,
    XferDataLast = 13,  // a data frame that also asks the receiver to acknowledge
};

enum class Dir : uint8_t { HubToGauge = 0, GaugeToHub = 1 };

constexpr uint32_t makeId(Msg msg, Dir dir, uint32_t node) {
    return (uint32_t(msg) << 25) | (uint32_t(dir) << 24) | (node & 0xFFFFFF);
}
constexpr Msg idMsg(uint32_t id) { return Msg((id >> 25) & 0x0F); }
constexpr Dir idDir(uint32_t id) { return Dir((id >> 24) & 0x01); }
constexpr uint32_t idNode(uint32_t id) { return id & 0xFFFFFF; }

enum class Cmd : uint8_t {
    Identify = 1,       // arg = seconds to show the identify screen
    SetFace = 2,        // arg = face index
    SetBrightness = 3,  // arg = 0..255
    Reboot = 4,
    SendConfig = 5,     // gauge uploads its stored config to the hub
    EndTry = 6,         // drop a RAM-only "try" config and go back to the stored one
    SetDisplay = 7,     // arg = 0 screen off (panel asleep), 1 on
};

enum class XferKind : uint8_t {
    Config = 1,     // store and apply
    TryConfig = 2,  // apply in RAM only
};

enum class XferStatus : uint8_t {
    Ok = 0,         // carry on from nextFrame
    Done = 1,       // payload complete, CRC good, accepted
    Busy = 2,
    TooBig = 3,
    BadKind = 4,
    CrcFail = 5,
    NoSession = 6,
    Rejected = 7,   // payload arrived intact but the receiver refused it
    Timeout = 8,    // sender-side only: the peer stopped answering
};

struct Beacon {
    uint8_t protocolVersion = kProtocolVersion;
    uint8_t flags = 0;
    uint16_t uptimeSec = 0;
};
constexpr uint8_t kBeaconFlagSimulator = 0x01;

struct Hello {
    uint8_t mac[6] = {};
    uint8_t protocolVersion = kProtocolVersion;
    uint8_t schemaVersion = 0;
};

struct Info {
    uint8_t fwMajor = 0;
    uint8_t fwMinor = 0;
    uint8_t fwPatch = 0;
    uint8_t hwRevision = 0;
};

struct Status {
    uint32_t configCrc = 0;
    uint8_t activeFace = 0;
    uint8_t faceCount = 0;
    uint16_t vbusMillivolts = 0;  // carried in 25 mV steps
    uint8_t flags = 0;
};
constexpr uint8_t kStatusFlagTrying = 0x01;  // showing a RAM-only "try" config
constexpr uint8_t kStatusFlagDemo = 0x02;

struct Command {
    Cmd cmd = Cmd::Identify;
    uint8_t arg = 0;
};

struct CommandAck {
    Cmd cmd = Cmd::Identify;
    bool ok = false;
};

// A gauge asking the hub to apply a setting, from its pull-down menu: to
// itself (recorded) or to every gauge. Carried as Msg::Command, gauge to hub.
struct Request {
    Cmd cmd = Cmd::SetBrightness;
    uint8_t arg = 0;
    bool all = false;
};

// A message the hub asks a gauge to show over its face while an alert is in
// force, or the order to take it down. It goes out as a header frame and up
// to five frames of text, and is repeated for as long as it applies: a gauge
// drops the message when the repeats stop, so a lost "clear" cannot leave it up.
constexpr size_t kAlertMaxText = 32;
constexpr size_t kAlertMaxFrames = 6;
constexpr uint32_t kAlertRepeatMs = 1000;

struct Alert {
    bool show = false;          // false takes the message down
    uint32_t color = 0xFF3B30;  // 0xRRGGBB
    uint8_t ttlSeconds = 3;     // how long the message outlives the last repeat; 0 = until cleared
    char text[kAlertMaxText + 1] = {};
};

// The hub's theme colours, the "themecolour1" to "themecolour8" a config can
// use. Four frames: the frame number (0 to 3), then colours 2k and 2k+1 as
// three bytes each. The hub sends them when they change, to a gauge that comes
// online, and every kThemeColourRepeatMs; each frame stands on its own.
constexpr size_t kThemeColourCount = 8;
constexpr size_t kThemeColourFrames = kThemeColourCount / 2;
constexpr uint32_t kThemeColourRepeatMs = 10000;

struct XferBegin {
    uint8_t session = 0;
    XferKind kind = XferKind::Config;
    uint32_t size = 0;        // bytes, up to 2^24 - 1
    uint8_t blockFrames = 0;  // data frames per acknowledged block
};

struct XferEnd {
    uint8_t session = 0;
    uint32_t crc = 0;
};

struct XferAck {
    uint8_t session = 0;
    XferStatus status = XferStatus::Ok;
    uint32_t nextFrame = 0;  // index of the first data frame not yet received
};

struct XferAbort {
    uint8_t session = 0;
};

constexpr uint32_t kXferPayloadPerFrame = 7;
constexpr uint32_t kXferMaxSize = 0xFFFFFF;

CanFrame pack(const Beacon& m);
CanFrame pack(const Hello& m, uint32_t node);
CanFrame packHelloRequest(uint32_t node);
CanFrame pack(const Info& m, uint32_t node);
CanFrame pack(const Status& m, uint32_t node);
CanFrame pack(const Command& m, uint32_t node);
CanFrame pack(const CommandAck& m, uint32_t node);
CanFrame pack(const Request& m, uint32_t node);
// Fills `out` with the frames of one alert and returns how many. `seq` (0..15)
// tells one message from the next.
size_t packAlert(const Alert& m, uint8_t seq, uint32_t node, CanFrame out[kAlertMaxFrames]);
size_t packThemeColours(const uint32_t colours[kThemeColourCount], uint32_t node,
                        CanFrame out[kThemeColourFrames]);
// Writes the two colours one frame carries into `colours`. False if malformed.
bool unpackThemeColours(const CanFrame& f, uint32_t colours[kThemeColourCount]);
CanFrame pack(const XferBegin& m, Dir dir, uint32_t node);
CanFrame pack(const XferEnd& m, Dir dir, uint32_t node);
CanFrame pack(const XferAck& m, Dir dir, uint32_t node);
CanFrame pack(const XferAbort& m, Dir dir, uint32_t node);
// Data frame carrying up to 7 payload bytes; seq is the low byte of the frame
// index. `last` marks the final frame of a block, which the receiver acknowledges.
CanFrame packXferData(Dir dir, uint32_t node, uint8_t seq, const uint8_t* payload, uint8_t size,
                      bool last);

// Each returns false if the frame is too short for the message.
bool unpack(const CanFrame& f, Beacon& m);
bool unpack(const CanFrame& f, Hello& m);
bool unpack(const CanFrame& f, Request& m);
bool unpack(const CanFrame& f, Info& m);
bool unpack(const CanFrame& f, Status& m);
bool unpack(const CanFrame& f, Command& m);
bool unpack(const CanFrame& f, CommandAck& m);
bool unpack(const CanFrame& f, XferBegin& m);
bool unpack(const CanFrame& f, XferEnd& m);
bool unpack(const CanFrame& f, XferAck& m);
bool unpack(const CanFrame& f, XferAbort& m);

// Puts an alert back together on the gauge from its frames.
class AlertReceiver {
public:
    // Takes a Msg::Alert frame. True if what should be on screen changed.
    bool onFrame(const CanFrame& frame, uint32_t nowMs);
    // True if the message has just timed out.
    bool poll(uint32_t nowMs);
    // The message to show; `show` is false when there is none.
    const Alert& current() const { return current_; }

private:
    Alert current_;
    uint32_t refreshedMs_ = 0;

    // The message being received.
    Alert pending_;
    uint8_t seq_ = 0xFF;
    bool haveHeader_ = false;
    uint8_t length_ = 0;
    uint8_t chunks_ = 0;  // bit k-1 is set once text frame k has arrived
};

}  // namespace proto
}  // namespace hg
