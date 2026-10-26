#pragma once

#include <ESP32-TWAI-CAN.hpp>
#include <sdkconfig.h>

#include "can_frame.h"

// Chips with two CAN controllers (the ESP32-C6 hub) get both buses. A
// single-controller chip (an ESP32-C3 on the bench) gets only the gauge bus:
// enough for the simulator, the roster and config pushes, but no ECU bridge.
#ifdef TWAI_CAN_NEW_DRIVER
#define HG_HAS_ECU_PORT 1
#else
#define HG_HAS_ECU_PORT 0
#endif

struct CanPortStats {
    uint32_t rxMissed;   // frames dropped because the receive queue was full
    uint32_t txRejected; // writes refused because every transmit slot was busy
    uint32_t busErrors;
    uint16_t txErrors;
    uint16_t rxErrors;
    uint32_t state;      // TWAI_STATE_*
};

#if HG_HAS_ECU_PORT
// The Haltech bus. Receive only: there is deliberately no way to transmit an
// application frame from here. The controller still acknowledges frames, so
// the ECU sees a healthy bus even when the hub is the only other node.
class EcuPort {
public:
    bool begin();
    bool read(hg::CanFrame& frame, uint32_t timeoutMs);
    CanPortStats stats();
    // Restarts the controller after bus-off. Call periodically.
    void service();
};
#endif

// The private bus to the gauges.
class GaugePort {
public:
    bool begin();
    bool read(hg::CanFrame& frame, uint32_t timeoutMs);
    // Returns false if no transmit slot is free; the frame is not queued.
    bool write(const hg::CanFrame& frame);
    uint32_t txPending();
    CanPortStats stats();
    void service();
};

// The vehicle CAN bus to the Nexus's second CAN, carrying the steering
// wheel's cruise buttons as FG Falcon frames (docs/steering-wheel.md). Only
// the P4 has a third controller for it.
#if CONFIG_IDF_TARGET_ESP32P4 && defined(TWAI_CAN_NEW_DRIVER)
#define HG_HAS_FG_PORT 1
#else
#define HG_HAS_FG_PORT 0
#endif

#if HG_HAS_FG_PORT
class FgPort {
public:
    // Begun after the other two ports: the driver hands out controllers in order.
    bool begin(TwaiSpeed speed);
    bool read(hg::CanFrame& frame, uint32_t timeoutMs);
    bool write(const hg::CanFrame& frame);
    // The transceiver's silent pin: the port listens, never transmits or acknowledges.
    void setListenOnly(bool on);
    CanPortStats stats();
    void service();
};
#endif
