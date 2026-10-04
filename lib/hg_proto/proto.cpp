#include "proto.h"

#include <string.h>

namespace hg {
namespace proto {

namespace {

CanFrame makeFrame(Msg msg, Dir dir, uint32_t node, uint8_t len) {
    CanFrame f;
    f.id = makeId(msg, dir, node);
    f.extended = true;
    f.len = len;
    return f;
}

void put24(uint8_t* p, uint32_t v) {
    p[0] = uint8_t(v >> 16);
    p[1] = uint8_t(v >> 8);
    p[2] = uint8_t(v);
}
uint32_t get24(const uint8_t* p) { return (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8) | p[2]; }

void put32(uint8_t* p, uint32_t v) {
    p[0] = uint8_t(v >> 24);
    put24(p + 1, v);
}
uint32_t get32(const uint8_t* p) { return (uint32_t(p[0]) << 24) | get24(p + 1); }

}  // namespace

CanFrame pack(const Beacon& m) {
    CanFrame f = makeFrame(Msg::Beacon, Dir::HubToGauge, kBroadcastNode, 4);
    f.data[0] = m.protocolVersion;
    f.data[1] = m.flags;
    f.data[2] = uint8_t(m.uptimeSec >> 8);
    f.data[3] = uint8_t(m.uptimeSec);
    return f;
}

bool unpack(const CanFrame& f, Beacon& m) {
    if (f.len < 4) return false;
    m.protocolVersion = f.data[0];
    m.flags = f.data[1];
    m.uptimeSec = uint16_t((f.data[2] << 8) | f.data[3]);
    return true;
}

CanFrame pack(const Hello& m, uint32_t node) {
    CanFrame f = makeFrame(Msg::Hello, Dir::GaugeToHub, node, 8);
    memcpy(f.data, m.mac, 6);
    f.data[6] = m.protocolVersion;
    f.data[7] = m.schemaVersion;
    return f;
}

CanFrame packHelloRequest(uint32_t node) { return makeFrame(Msg::Hello, Dir::HubToGauge, node, 0); }

bool unpack(const CanFrame& f, Hello& m) {
    if (f.len < 8) return false;
    memcpy(m.mac, f.data, 6);
    m.protocolVersion = f.data[6];
    m.schemaVersion = f.data[7];
    return true;
}

CanFrame pack(const Info& m, uint32_t node) {
    CanFrame f = makeFrame(Msg::Info, Dir::GaugeToHub, node, 4);
    f.data[0] = m.fwMajor;
    f.data[1] = m.fwMinor;
    f.data[2] = m.fwPatch;
    f.data[3] = m.hwRevision;
    return f;
}

bool unpack(const CanFrame& f, Info& m) {
    if (f.len < 4) return false;
    m.fwMajor = f.data[0];
    m.fwMinor = f.data[1];
    m.fwPatch = f.data[2];
    m.hwRevision = f.data[3];
    return true;
}

CanFrame pack(const Status& m, uint32_t node) {
    CanFrame f = makeFrame(Msg::Status, Dir::GaugeToHub, node, 8);
    put32(f.data, m.configCrc);
    f.data[4] = m.activeFace;
    f.data[5] = m.faceCount;
    const uint32_t steps = m.vbusMillivolts / 25;
    f.data[6] = uint8_t(steps > 255 ? 255 : steps);
    f.data[7] = m.flags;
    return f;
}

bool unpack(const CanFrame& f, Status& m) {
    if (f.len < 8) return false;
    m.configCrc = get32(f.data);
    m.activeFace = f.data[4];
    m.faceCount = f.data[5];
    m.vbusMillivolts = uint16_t(f.data[6] * 25);
    m.flags = f.data[7];
    return true;
}

CanFrame pack(const Command& m, uint32_t node) {
    CanFrame f = makeFrame(Msg::Command, Dir::HubToGauge, node, 2);
    f.data[0] = uint8_t(m.cmd);
    f.data[1] = m.arg;
    return f;
}

CanFrame pack(const Request& m, uint32_t node) {
    CanFrame f = makeFrame(Msg::Command, Dir::GaugeToHub, node, 3);
    f.data[0] = uint8_t(m.cmd);
    f.data[1] = m.arg;
    f.data[2] = m.all ? 1 : 0;
    return f;
}

bool unpack(const CanFrame& f, Request& m) {
    if (f.len < 3) return false;
    m.cmd = Cmd(f.data[0]);
    m.arg = f.data[1];
    m.all = f.data[2] != 0;
    return true;
}

bool unpack(const CanFrame& f, Command& m) {
    if (f.len < 2) return false;
    m.cmd = Cmd(f.data[0]);
    m.arg = f.data[1];
    return true;
}

CanFrame pack(const CommandAck& m, uint32_t node) {
    CanFrame f = makeFrame(Msg::CommandAck, Dir::GaugeToHub, node, 2);
    f.data[0] = uint8_t(m.cmd);
    f.data[1] = m.ok ? 1 : 0;
    return f;
}

bool unpack(const CanFrame& f, CommandAck& m) {
    if (f.len < 2) return false;
    m.cmd = Cmd(f.data[0]);
    m.ok = f.data[1] != 0;
    return true;
}

// Header: seq << 4, flags (bit 0 = show), text length, colour (3 bytes), time
// to live in seconds. Text frames: seq << 4 | index (1..5), then up to 7 bytes.
size_t packAlert(const Alert& m, uint8_t seq, uint32_t node, CanFrame out[kAlertMaxFrames]) {
    size_t length = 0;
    while (m.show && length < kAlertMaxText && m.text[length]) length++;

    out[0] = makeFrame(Msg::Alert, Dir::HubToGauge, node, 7);
    out[0].data[0] = uint8_t(seq << 4);
    out[0].data[1] = m.show ? 1 : 0;
    out[0].data[2] = uint8_t(length);
    put24(out[0].data + 3, m.color);
    out[0].data[6] = m.ttlSeconds;

    size_t count = 1;
    for (size_t pos = 0; pos < length; pos += 7) {
        const size_t n = length - pos < 7 ? length - pos : 7;
        out[count] = makeFrame(Msg::Alert, Dir::HubToGauge, node, uint8_t(1 + n));
        out[count].data[0] = uint8_t((seq << 4) | count);
        memcpy(out[count].data + 1, m.text + pos, n);
        count++;
    }
    return count;
}

bool AlertReceiver::onFrame(const CanFrame& f, uint32_t nowMs) {
    if (f.len < 1) return false;
    const uint8_t seq = f.data[0] >> 4;
    const uint8_t index = f.data[0] & 0x0F;
    if (seq != seq_) {  // a different message: start again
        seq_ = seq;
        haveHeader_ = false;
        chunks_ = 0;
    }

    if (index == 0) {
        if (f.len < 7) return false;
        const uint8_t length = f.data[2] > kAlertMaxText ? uint8_t(kAlertMaxText) : f.data[2];
        if (haveHeader_ && length != length_) chunks_ = 0;
        haveHeader_ = true;
        length_ = length;
        pending_.show = (f.data[1] & 1) != 0;
        pending_.color = get24(f.data + 3);
        pending_.ttlSeconds = f.data[6];
        if (!pending_.show) {
            const bool wasShowing = current_.show;
            current_ = Alert();
            chunks_ = 0;
            return wasShowing;
        }
    } else {
        const size_t pos = size_t(index - 1) * 7;
        if (index > 5 || pos >= kAlertMaxText) return false;
        size_t n = f.len - 1;
        if (pos + n > kAlertMaxText) n = kAlertMaxText - pos;
        memcpy(pending_.text + pos, f.data + 1, n);
        chunks_ |= uint8_t(1u << (index - 1));
    }

    if (!haveHeader_ || !pending_.show) return false;
    const uint8_t needed = uint8_t((1u << ((length_ + 6) / 7)) - 1);
    if ((chunks_ & needed) != needed) return false;
    pending_.text[length_] = 0;
    refreshedMs_ = nowMs;
    const bool changed = !current_.show || current_.color != pending_.color ||
                         strcmp(current_.text, pending_.text) != 0;
    current_ = pending_;
    return changed;
}

bool AlertReceiver::poll(uint32_t nowMs) {
    if (!current_.show || current_.ttlSeconds == 0) return false;
    if (uint32_t(nowMs - refreshedMs_) < uint32_t(current_.ttlSeconds) * 1000) return false;
    current_ = Alert();
    seq_ = 0xFF;
    haveHeader_ = false;
    chunks_ = 0;
    return true;
}

CanFrame pack(const XferBegin& m, Dir dir, uint32_t node) {
    CanFrame f = makeFrame(Msg::XferBegin, dir, node, 6);
    f.data[0] = m.session;
    f.data[1] = uint8_t(m.kind);
    put24(f.data + 2, m.size);
    f.data[5] = m.blockFrames;
    return f;
}

bool unpack(const CanFrame& f, XferBegin& m) {
    if (f.len < 6) return false;
    m.session = f.data[0];
    m.kind = XferKind(f.data[1]);
    m.size = get24(f.data + 2);
    m.blockFrames = f.data[5];
    return true;
}

CanFrame pack(const XferEnd& m, Dir dir, uint32_t node) {
    CanFrame f = makeFrame(Msg::XferEnd, dir, node, 5);
    f.data[0] = m.session;
    put32(f.data + 1, m.crc);
    return f;
}

bool unpack(const CanFrame& f, XferEnd& m) {
    if (f.len < 5) return false;
    m.session = f.data[0];
    m.crc = get32(f.data + 1);
    return true;
}

CanFrame pack(const XferAck& m, Dir dir, uint32_t node) {
    CanFrame f = makeFrame(Msg::XferAck, dir, node, 5);
    f.data[0] = m.session;
    f.data[1] = uint8_t(m.status);
    put24(f.data + 2, m.nextFrame);
    return f;
}

bool unpack(const CanFrame& f, XferAck& m) {
    if (f.len < 5) return false;
    m.session = f.data[0];
    m.status = XferStatus(f.data[1]);
    m.nextFrame = get24(f.data + 2);
    return true;
}

CanFrame pack(const XferAbort& m, Dir dir, uint32_t node) {
    CanFrame f = makeFrame(Msg::XferAbort, dir, node, 1);
    f.data[0] = m.session;
    return f;
}

bool unpack(const CanFrame& f, XferAbort& m) {
    if (f.len < 1) return false;
    m.session = f.data[0];
    return true;
}

CanFrame packXferData(Dir dir, uint32_t node, uint8_t seq, const uint8_t* payload, uint8_t size,
                      bool last) {
    if (size > kXferPayloadPerFrame) size = kXferPayloadPerFrame;
    CanFrame f = makeFrame(last ? Msg::XferDataLast : Msg::XferData, dir, node, uint8_t(size + 1));
    f.data[0] = seq;
    memcpy(f.data + 1, payload, size);
    return f;
}

}  // namespace proto
}  // namespace hg
