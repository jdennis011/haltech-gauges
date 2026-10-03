#pragma once

// The gauge's side of the management protocol: stays silent until a hub
// beacon is heard, then announces itself, reports status once a second,
// carries out commands and receives configs. Driven by onFrame() and poll();
// no dependence on the CAN driver or clock.

#include "proto.h"
#include "xfer.h"

namespace hg {
namespace proto {

class GaugeNode {
public:
    struct Handler {
        // A command from the hub. Return true if it was carried out.
        bool (*command)(Cmd cmd, uint8_t arg, void* ctx);
        // Fill in the current state for the periodic status message.
        void (*status)(Status& out, void* ctx);
        // Incoming configs.
        XferReceiver::Handler transfer;
        void* ctx;
    };

    // The default node id for a gauge: the low 24 bits of its MAC.
    static uint32_t nodeFromMac(const uint8_t mac[6]);

    void init(const XferLink& link, const uint8_t mac[6], uint32_t node, const Info& info,
              uint8_t schemaVersion, const Handler& handler);

    void onFrame(const CanFrame& frame, uint32_t nowMs);
    void poll(uint32_t nowMs);

    // True while beacons are arriving. Nothing is transmitted otherwise, so a
    // gauge plugged straight into a Haltech bus never sends a frame.
    bool hubPresent() const { return hubPresent_; }
    uint32_t node() const { return node_; }

    // Set when another device was seen transmitting with this node id. The
    // application picks a new random id, stores it, and calls setNode().
    bool idConflict() const { return conflict_; }
    void setNode(uint32_t node);

    // Sends a stored config to the hub (in answer to Cmd::SendConfig). `data`
    // must stay valid until uploading() is false.
    bool upload(const uint8_t* data, uint32_t size, uint32_t nowMs);
    bool uploading() const { return sender_.busy(); }

private:
    bool send(const CanFrame& frame) { return link_.send(frame, link_.ctx); }

    XferLink link_ = {};
    Handler handler_ = {};
    Hello hello_;
    Info info_;
    uint32_t node_ = 0;

    bool haveBeacon_ = false;
    bool hubPresent_ = false;
    uint32_t lastBeaconMs_ = 0;
    uint16_t lastUptime_ = 0;
    uint32_t lastStatusMs_ = 0;
    bool statusDue_ = false;
    bool helloPending_ = false;
    bool infoPending_ = false;
    bool ackPending_ = false;
    CommandAck ack_;
    bool conflict_ = false;
    uint8_t uploadSession_ = 0;

    XferReceiver receiver_;
    XferSender sender_;
};

}  // namespace proto
}  // namespace hg
