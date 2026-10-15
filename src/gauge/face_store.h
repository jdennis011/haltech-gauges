#pragma once

#include <stddef.h>
#include <stdint.h>

#include "face_model.h"
#include "proto.h"

// The config the gauge shows: the one the hub stored on it (LittleFS,
// /config.json), a "try" config shown without storing, or the built-in face
// when there is neither. Configs arrive on the CAN task and are parsed there;
// the UI task picks a new one up with takeChange() and rebuilds its screen, so
// the config the UI is drawing from is never replaced under it.
namespace face_store {

// After the filesystem is mounted (image_store::begin()).
void begin();

// XferReceiver::Handler for Config and TryConfig. Image transfers are the
// image store's.
hg::proto::XferStatus accept(hg::proto::XferKind kind, uint32_t size, uint8_t** buffer);
bool complete(hg::proto::XferKind kind, const uint8_t* data, uint32_t size);
void dropped();

// --- UI task

// The config to draw. Never null after begin().
const hg::cfg::Config* current();
// True once after the config to draw has changed; the caller rebuilds.
bool takeChange();
size_t faceCount();
uint8_t activeFace();
// Clamped to the faces there are; kept across a power cycle.
void setActiveFace(uint8_t index);
bool trying();
void endTry();
// Call after the firmware has run healthily for a while: clears the restart
// counter that otherwise makes the next boot skip the stored config.
void markHealthy();
// True when the stored config was skipped after repeated restarts.
bool fallback();

// --- CAN task

// CRC32 of the stored config's bytes, 0 with none: what the hub compares.
uint32_t storedCrc();
// The stored bytes, for Cmd::SendConfig; null with none. Valid until the next
// stored config arrives.
const uint8_t* storedBytes(size_t& size);

}  // namespace face_store
