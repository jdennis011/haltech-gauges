#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "hub_manager.h"

// Large transfers to the gauges over the hub's own Wi-Fi (docs/images.md,
// "Over Wi-Fi"). The hub tells a gauge, over CAN, its access point, where to
// fetch from and a one-time token; the gauge joins, downloads, reports what it
// got, and leaves. Big images go this way when the gauge can; gauge firmware
// always does. CAN stays the fallback for images.
namespace wifi_xfer {

void begin();

// HubManager::Handler::wifiJoin: offers a gauge its missing images over Wi-Fi.
bool offerImages(const hg::proto::HubManager::Gauge& gauge, const uint32_t* crcs, size_t count,
                 const uint8_t** data, uint32_t* size, uint32_t* waitMs, void* ctx);

// Sends one gauge the hub's stored firmware build for its board. Takes the
// hub lock itself. False with why: offline, no build, a transfer running.
bool sendFirmware(uint32_t node, std::string& error);

// For the download routes: whether a live token covers this image, or the
// firmware (filling in the file to serve).
bool allowsImage(const char* token, uint32_t crc);
bool allowsFirmware(const char* token, std::string& path);

// Jobs whose tokens are still live.
size_t jobs();

}  // namespace wifi_xfer
