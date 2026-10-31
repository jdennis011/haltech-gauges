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
    ThemeColours = 7,  // hub to gauges: the hub's theme colours, in sixteen frames
    XferBegin = 8,
    XferData = 9,
    XferEnd = 10,
    XferAck = 11,
    XferAbort = 12,
    XferDataLast = 13,  // a data frame that also asks the receiver to acknowledge
    Asset = 14,  // does the gauge have this image? The hub asks, the gauge answers
    Input = 15,  // a steering wheel module's buttons, node to hub (docs/steering-wheel.md); the last type
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
    Intro = 8,          // the key-on intro: a whole Intro frame, not a two-byte Command
    InputRaw = 9,       // steering wheel: stream raw ladder readings for arg seconds (0 stops)
    PlaySound = 10,     // arg = a Sound: the gauge plays it on its speaker (docs/gauge-audio.md)
    SetVolume = 11,     // arg = speaker volume 0..100; the gauge keeps it
    ResetHold = 12,     // clear the held peaks and minimums on the gauge (docs/gauge-quick-menu.md)
    Needs = 13,         // gauge to hub: the Haltech frames its faces use (a Needs frame, not a Request)
    RunResult = 14,     // gauge to hub: one split of a drag timer run (a RunResult frame, docs/motion.md)
};

// What Cmd::PlaySound asks for, and what an alert rule names.
enum class Sound : uint8_t {
    None = 0,
    Beep = 1,   // one short note
    Chime = 2,  // two rising notes, the identify chime
    Alarm = 3,  // three two-tone pairs
    Riff = 4,   // the guitar riff from the tab (docs/gauge-audio.md), two octaves up
};

enum class XferKind : uint8_t {
    Config = 1,     // store and apply
    TryConfig = 2,  // apply in RAM only: a preview from the page
    Image = 3,      // a JPEG or PNG file to keep; the CRC32 of its bytes is its name
    SwitchConfig = 4,  // apply in RAM only, as TryConfig, but shown as the face proper: an alert's switch
    WifiJoin = 5,      // hub to gauge: its access point, a token and what to fetch over Wi-Fi, as JSON
    WheelCalibration = 6,  // hub to steering wheel module: its button calibration, as JSON
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
    ReadFail = 9,   // sender-side only: the payload could not be read from storage
};

struct Beacon {
    uint8_t protocolVersion = kProtocolVersion;
    uint8_t flags = 0;
    uint16_t uptimeSec = 0;
    // The hub's time of day as seconds since 1970 in its own zone, so a gauge
    // needs no zone (docs/time-of-day.md). Valid with kBeaconFlagTime; 0 otherwise.
    uint32_t localTime = 0;
};
constexpr uint8_t kBeaconFlagSimulator = 0x01;
constexpr uint8_t kBeaconFlagTime = 0x02;

struct Hello {
    uint8_t mac[6] = {};
    uint8_t protocolVersion = kProtocolVersion;
    uint8_t schemaVersion = 0;
};

struct Info {
    uint8_t fwMajor = 0;
    uint8_t fwMinor = 0;
    uint8_t fwPatch = 0;
    uint8_t hwRevision = 0;  // which gauge board: kBoard...
};
// Info::hwRevision: the board, so the hub picks the right firmware build for it.
constexpr uint8_t kBoardWaveshare175 = 1;
constexpr uint8_t kBoardM5Dial = 2;
constexpr uint8_t kBoardWheel = 3;  // the steering wheel module: a node on the bus, not a gauge

struct Status {
    uint32_t configCrc = 0;
    uint8_t activeFace = 0;
    uint8_t faceCount = 0;
    uint16_t vbusMillivolts = 0;  // carried in 25 mV steps
    uint8_t flags = 0;
};
constexpr uint8_t kStatusFlagTrying = 0x01;  // showing a RAM-only "try" config
constexpr uint8_t kStatusFlagDemo = 0x02;
constexpr uint8_t kStatusFlagWifi = 0x04;  // fetching over the hub's Wi-Fi
// Bits 3 to 7 of Status::flags carry the speaker volume as volume / 4 + 1, so
// 0 means no speaker (or firmware before 0.1.5) and 26 means 100.
constexpr uint8_t kStatusVolumeShift = 3;
inline uint8_t statusVolumeField(bool speaker, uint8_t volume) {
    return speaker ? uint8_t((volume > 100 ? 100 : volume) / 4 + 1) : 0;
}
// The volume a status carries, 0..100 in steps of 4, or -1 for no speaker.
inline int statusVolume(uint8_t flags) {
    const uint8_t v = uint8_t(flags >> kStatusVolumeShift);
    return v ? int(v - 1) * 4 : -1;
}

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

// Which Haltech frames a gauge's faces use, so the hub forwards only those
// (docs/frame-filter.md). Carried as Msg::Command,
// gauge to hub, with Cmd::Needs in the first byte, in kNeedsParts frames: part,
// the frame table's tag (hg::frameTableTag(); the hub forwards everything to a
// gauge whose tag differs from its own), then 40 bits, bit i of part p standing
// for the frame at position p * 40 + i in the table (hg::frameIndexOf). Sent
// when the config changes, when the hub appears, and every few seconds.
constexpr size_t kNeedsParts = 2;
constexpr unsigned kNeedsPartBits = 40;
struct Needs {
    uint8_t part = 0;
    uint8_t tag = 0;
    uint64_t bits = 0;  // the low kNeedsPartBits
};

// A finished drag timer run (docs/motion.md), gauge to hub as Msg::Command
// with Cmd::RunResult in the first byte, one frame per split reached (a
// single frame with count 0 when none was): the run type (hg::motion::Run),
// the run's sequence number, which split of how many, whether the run was
// abandoned, the split's time from the start and the speed as it was
// reached. The hub puts the frames of one run together as a RunReport.
constexpr size_t kRunSplits = 4;
struct RunResult {
    uint8_t run = 0;
    uint8_t seq = 0;       // 0..15
    uint8_t index = 0;     // 0..kRunSplits-1
    uint8_t count = 0;     // splits reached, 0..kRunSplits
    bool aborted = false;
    uint32_t ms = 0;       // 24 bits
    uint16_t speed10 = 0;  // 0.1 km/h
};
struct RunReport {
    uint8_t run = 0;
    uint8_t seq = 0;
    uint8_t count = 0;
    bool aborted = false;
    uint32_t ms[kRunSplits] = {};
    uint16_t speed10[kRunSplits] = {};
};

// The key-on intro (docs/startup-animation.md), hub to one gauge. Carried as
// Msg::Command with Cmd::Intro, so no message type is spent: cmd, style and
// flags, delay ms, duration ms, slot index, slot count. A gauge acknowledges
// it like any command.
enum class IntroStyle : uint8_t { Sweep = 0, Chase = 1, Fade = 2 };
constexpr uint8_t kIntroFlagForce = 0x10;  // play even if played since starting (the page's Play)
struct Intro {
    IntroStyle style = IntroStyle::Sweep;
    bool force = false;
    uint16_t delayMs = 0;        // wait this long after the frame arrives
    uint16_t durationMs = 1200;  // this gauge's own intro
    uint8_t index = 0;           // this gauge's slot, from 0
    uint8_t count = 1;           // gauges in the intro
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

// The hub's theme colours, the "themecolour1" to "themecolour32" a config can
// use. All 32 slots go out, the ones the hub has not filled as white, so a
// gauge always matches the hub. Sixteen frames: the frame number (0 to 15),
// then colours 2k and 2k+1 as three bytes each. The hub sends them when they
// change, to a gauge that comes online, and every kThemeColourRepeatMs; each
// frame stands on its own.
constexpr size_t kThemeColourCount = 32;
constexpr size_t kThemeColourFrames = kThemeColourCount / 2;
constexpr uint32_t kThemeColourRepeatMs = 10000;

// Images a gauge keeps are named by the CRC32 of their bytes, so the same
// picture is never sent twice and a changed one is a new name. Before
// sending one, the hub asks the gauge whether it already has it.
//   hub to gauge:  op 1 (query), kind, CRC32
//   gauge to hub:  op 2 (reply), kind, CRC32, 1 = have it / 0 = not
enum class AssetOp : uint8_t { Query = 1, Reply = 2 };
constexpr uint32_t kMaxImageBytes = 256 * 1024;

struct AssetQuery {
    XferKind kind = XferKind::Image;
    uint32_t crc = 0;
};

struct AssetReply {
    XferKind kind = XferKind::Image;
    uint32_t crc = 0;
    bool have = false;
};

// A steering wheel module's buttons (docs/steering-wheel.md), node to hub, as
// Msg::Input. Byte 0 carries the op in its high nibble and the layout version
// in the low one. `mask` is a level: a bit is set while that button is down
// (after the module's own debounce). Sent at once on a change, every
// kInputHoldPeriodMs while any button is down, and every kInputHeartbeatMs
// as a heartbeat when none is, so the hub can tell silence from idleness.
constexpr uint8_t kInputLayout = 1;
enum class InputOp : uint8_t { State = 0, Raw = 1, Raw2 = 2 };
constexpr uint16_t kInputHorn = 1u << 0;
constexpr uint16_t kInputCruiseOnOff = 1u << 1;
constexpr uint16_t kInputCruiseSet = 1u << 2;
constexpr uint16_t kInputCruiseResume = 1u << 3;
constexpr uint16_t kInputCruiseAccel = 1u << 4;
constexpr uint16_t kInputCruiseDecel = 1u << 5;
constexpr uint16_t kInputCruiseCancel = 1u << 6;
constexpr uint16_t kInputVolUp = 1u << 7;
constexpr uint16_t kInputVolDown = 1u << 8;
constexpr uint16_t kInputMode = 1u << 9;
constexpr uint16_t kInputMute = 1u << 10;
constexpr uint16_t kInputSeekUp = 1u << 11;
constexpr uint16_t kInputSeekDown = 1u << 12;
constexpr uint16_t kInputPhone = 1u << 13;
constexpr uint16_t kInputCruiseMask = kInputCruiseOnOff | kInputCruiseSet | kInputCruiseResume | kInputCruiseAccel |
                                      kInputCruiseDecel | kInputCruiseCancel;
constexpr uint16_t kInputMediaMask = kInputVolUp | kInputVolDown | kInputMode | kInputMute | kInputSeekUp |
                                     kInputSeekDown | kInputPhone;
constexpr uint8_t kInputFlagHeartbeat = 0x01;     // nothing changed since the last report
constexpr uint8_t kInputFlagFault = 0x02;         // a ladder line reads out of range (shorted or open)
constexpr uint8_t kInputFlagUncalibrated = 0x04;  // no calibration stored: the mask is always 0
constexpr uint8_t kInputFlagRaw = 0x08;           // raw readings are being streamed too
constexpr uint32_t kInputHoldPeriodMs = 20;
constexpr uint32_t kInputHeartbeatMs = 200;
struct Input {
    uint8_t seq = 0;    // +1 per frame sent, wraps
    uint16_t mask = 0;  // kInput... bits
    uint8_t flags = 0;  // kInputFlag... bits
    uint8_t lines = 0;  // ladder lines the module reads
};
// Raw ladder readings for calibration (Cmd::InputRaw asks for them): three
// lines' millivolts per frame, lines 0-2 or 3-5.
struct InputRaw {
    uint8_t seq = 0;
    uint8_t first = 0;  // 0 or 3: the first line carried
    uint16_t mv[3] = {};
};

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
CanFrame pack(const Intro& m, uint32_t node);
CanFrame pack(const Request& m, uint32_t node);
CanFrame pack(const Needs& m, uint32_t node);
CanFrame pack(const RunResult& m, uint32_t node);
// Fills `out` with the frames of one alert and returns how many. `seq` (0..15)
// tells one message from the next.
size_t packAlert(const Alert& m, uint8_t seq, uint32_t node, CanFrame out[kAlertMaxFrames]);
size_t packThemeColours(const uint32_t colours[kThemeColourCount], uint32_t node,
                        CanFrame out[kThemeColourFrames]);
// Writes the two colours one frame carries into `colours`. False if malformed.
bool unpackThemeColours(const CanFrame& f, uint32_t colours[kThemeColourCount]);
CanFrame pack(const AssetQuery& m, uint32_t node);
CanFrame pack(const AssetReply& m, uint32_t node);
CanFrame pack(const Input& m, uint32_t node);
CanFrame pack(const InputRaw& m, uint32_t node);
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
// False for a frame that is not a Needs frame (a plain Request, say).
bool unpack(const CanFrame& f, Needs& m);
// False for a frame that is not a RunResult frame.
bool unpack(const CanFrame& f, RunResult& m);
bool unpack(const CanFrame& f, Info& m);
bool unpack(const CanFrame& f, Status& m);
bool unpack(const CanFrame& f, Command& m);
bool unpack(const CanFrame& f, CommandAck& m);
bool unpack(const CanFrame& f, Intro& m);
bool unpack(const CanFrame& f, AssetQuery& m);
bool unpack(const CanFrame& f, AssetReply& m);
bool unpack(const CanFrame& f, Input& m);
bool unpack(const CanFrame& f, InputRaw& m);
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
