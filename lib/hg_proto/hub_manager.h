#pragma once

// The hub's side of the management protocol: sends the beacon, keeps a roster
// of the gauges on the chain, and keeps each gauge's stored config equal to
// the one assigned to it. "Assigned" is desired state: whenever a gauge's
// reported config CRC differs from its assignment, the config is pushed. That
// one rule covers a new gauge, a gauge that was reset, a hub restart and an
// edited config. Driven by onFrame() and poll().

#include <stddef.h>

#include "proto.h"
#include "xfer.h"

namespace hg {
namespace proto {

class HubManager {
public:
    static constexpr size_t kMaxGauges = 8;

    struct Gauge {
        bool used = false;
        bool online = false;
        uint32_t node = 0;
        uint32_t lastSeenMs = 0;

        bool haveHello = false;
        uint8_t mac[6] = {};
        uint8_t protocolVersion = 0;
        uint8_t schemaVersion = 0;
        uint32_t helloMs = 0;
        bool duplicateId = false;  // two different MACs announced this node id

        bool haveInfo = false;
        Info info;
        bool haveStatus = false;
        Status status;

        // Outcome of the last attempt to push the assigned config.
        bool pushFailed = false;   // the gauge refused it; not retried until something changes
        XferStatus pushResult = XferStatus::Ok;
        uint32_t failedCrc = 0;
        uint32_t nextPushMs = 0;

        bool haveAck = false;
        CommandAck ack;
    };

    struct Desired {
        const uint8_t* data = nullptr;
        uint32_t size = 0;
        uint32_t crc = 0;     // crc32 of data
        uint8_t schema = 0;   // schema version the config needs
    };

    struct Handler {
        // The config assigned to this gauge, if any. Return false for none.
        // The data must stay valid until pushBusy() is false.
        bool (*desired)(const Gauge& gauge, Desired& out, void* ctx);
        // Receives a config a gauge uploads after requestUpload().
        XferReceiver::Handler upload;
        // The roster or a gauge's state changed. May be null.
        void (*changed)(void* ctx);
        // A gauge has just come online (before it has said hello). May be null.
        void (*online)(const Gauge& gauge, void* ctx);
        // A gauge asks for a setting to be applied, to itself or to all. May be null.
        void (*request)(const Gauge& gauge, const Request& request, void* ctx);
        void* ctx;
    };

    void init(const XferLink& link, const Handler& handler, uint32_t nowMs);
    void setBeaconFlags(uint8_t flags) { beaconFlags_ = flags; }
    void onFrame(const CanFrame& frame, uint32_t nowMs);
    void poll(uint32_t nowMs);

    const Gauge& gauge(size_t index) const { return gauges_[index]; }
    const Gauge* find(uint32_t node) const;
    // Live data is only worth forwarding while a gauge is listening.
    bool anyOnline() const;

    // Sends a command. The gauge's answer appears in its roster entry.
    bool command(uint32_t node, Cmd cmd, uint8_t arg);
    // Shows a config on a gauge without storing it. `data` must stay valid
    // until pushBusy() is false. Returns false if a transfer is already running.
    bool tryConfig(uint32_t node, const uint8_t* data, uint32_t size, uint32_t nowMs);
    // Asks a gauge to upload its stored config; it arrives through Handler::upload.
    bool requestUpload(uint32_t node, uint32_t nowMs);

    bool pushBusy() const { return sender_.busy(); }
    uint32_t pushNode() const { return sender_.node(); }
    uint32_t pushFramesAcked() const { return sender_.framesAcked(); }
    uint32_t pushFramesTotal() const { return sender_.totalFrames(); }
    bool uploadBusy() const { return uploadArmed_ && uploadReceiver_.active(); }

private:
    Gauge* findMutable(uint32_t node);
    Gauge* findOrAdd(uint32_t node, uint32_t nowMs);
    bool send(const CanFrame& frame) { return link_.send(frame, link_.ctx); }
    void notify();
    void reconcile(uint32_t nowMs);
    void finishPush(uint32_t nowMs);

    XferLink link_ = {};
    Handler handler_ = {};
    Gauge gauges_[kMaxGauges];
    uint32_t startMs_ = 0;
    uint32_t lastBeaconMs_ = 0;
    bool beaconSent_ = false;
    uint8_t beaconFlags_ = 0;

    XferSender sender_;
    bool pushActive_ = false;
    bool pushIsTry_ = false;
    uint32_t pushCrc_ = 0;
    uint8_t session_ = 0;

    XferReceiver uploadReceiver_;
    bool uploadArmed_ = false;  // uploadReceiver_ has been set up by requestUpload()
    uint32_t helloRequestNode_ = 0;
    bool helloRequestPending_ = false;
    uint32_t lastHelloRequestMs_ = 0;
};

}  // namespace proto
}  // namespace hg
