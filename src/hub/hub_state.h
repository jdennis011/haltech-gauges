#pragma once

// What the hub's tasks share. The manager and the simulator are only touched
// while holding lock(); the live channel store is written by the CAN tasks and
// read lock-free by the web layer, which its design allows.

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdint.h>

#include "can_ports.h"
#include "channel_store.h"
#include "hub_manager.h"

struct HubStats {
    bool ecuPresent;      // false on single-controller development boards
    uint32_t ecuFrames;
    CanPortStats ecu;
    uint32_t forwarded;
    uint32_t forwardDropped;
    uint32_t simulated;
    CanPortStats bus;
    uint32_t txPending;
    bool simulator;
    uint32_t gaugesOnline;
};

namespace hub {

SemaphoreHandle_t lock();
hg::proto::HubManager& manager();  // hold lock() while using it
hg::ChannelStore& live();
HubStats stats();
void setSimulator(bool on);
bool simulatorEnabled();
// Increments whenever the roster or a gauge's state changes.
uint32_t rosterVersion();
// Marks the roster changed, e.g. after a planned gauge is edited.
void rosterTouched();
const char* boardName();
// Why the hub last restarted, e.g. "power on", "panic", "brownout".
const char* resetReason();

struct Lock {
    Lock() { xSemaphoreTake(lock(), portMAX_DELAY); }
    ~Lock() { xSemaphoreGive(lock()); }
};

}  // namespace hub
