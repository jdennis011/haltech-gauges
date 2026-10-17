#pragma once

// CAN frames carried in UDP datagrams, for a gauge linked to the hub over
// Wi-Fi instead of the bus (docs/ip-link.md): the bench with no transceiver,
// or a gauge too far for the chain. A datagram is a four-byte header, 'H' 'G'
// version count, then up to 64 frames of 13 bytes: id (big-endian, bit 31 set
// for an extended id), length, eight data bytes. An empty datagram is a
// keepalive.

#include <stddef.h>
#include <stdint.h>

#include "can_frame.h"

namespace hg {

constexpr uint16_t kIpBusPort = 4510;
constexpr uint8_t kIpBusVersion = 1;
constexpr size_t kIpHeaderBytes = 4;
constexpr size_t kIpFrameBytes = 13;
constexpr size_t kIpMaxFrames = 64;
constexpr size_t kIpMaxPacket = kIpHeaderBytes + kIpMaxFrames * kIpFrameBytes;

// Writes a datagram of `count` frames; returns its length, or 0 if it would not fit.
size_t packIpFrames(const CanFrame* frames, size_t count, uint8_t* out, size_t cap);
// Reads a datagram's frames into `out` (up to `cap`); returns how many, or -1
// if it is not one of ours.
int unpackIpFrames(const uint8_t* data, size_t len, CanFrame* out, size_t cap);

}  // namespace hg
