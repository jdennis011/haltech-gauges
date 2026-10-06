#pragma once

// The hub's side of the management protocol: sends the beacon, keeps a roster
// of the gauges on the chain, and keeps each gauge's stored config equal to
// the one assigned to it. "Assigned" is desired state: whenever a gauge's
// reported config CRC differs from its assignment, the config is pushed. That
// one rule covers a new gauge, a gauge that was reset, a hub restart and an
// edited config. Driven by onFrame() and poll().
//
// The images a config uses go first. Each is named by the CRC32 of its bytes;
// the hub asks the gauge whether it has one (Msg::Asset) and sends it only if
// not, so an image crosses the bus once per gauge however often the config
// around it changes.

#include <stddef.h>

#include "proto.h"
#include "xfer.h"

namespace hg {
namespace proto {

class HubManager {
public:
    static constexpr size_t kMaxGauges = 8;
    // Different images one config may use (cfg::kMaxImagesPerConfig).
    static constexpr size_t kMaxImages = 16;

    // What the hub knows of one image on one gauge.
    enum class ImageState : uint8_t {
        Unknown = 0,  // not asked yet
        Have = 1,     // the gauge has it
        Missing = 2,  // the gauge said no (or never answered); it will be sent
        Refused = 3,  // the gauge would not take it; given up until it says hello again
    };

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

        // Images on this gauge, by CRC: room for a stored config's and a tried
        // config's, the oldest forgotten first. All forgotten when the gauge
        // says hello, since it may have been reset or reflashed.
        struct Image {
            uint32_t crc;
            ImageState state;
            uint8_t failures;  // sends that failed for a passing reason
        };
        Image images[2 * kMaxImages];
        uint8_t imageCount = 0;
    };

    struct Desired {
        const uint8_t* data = nullptr;
        uint32_t size = 0;
        uint32_t crc = 0;     // crc32 of data
        uint8_t schema = 0;   // schema version the config needs
        // The images the config uses, which the gauge gets before the config.
        uint32_t images[kMaxImages] = {};
        uint8_t imageCount = 0;
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
        // Opens the image with this CRC for sending: fills `source` and `size`.
        // False if the hub no longer has it. May be null if there are no images.
        bool (*openImage)(uint32_t crc, XferSource& source, uint32_t& size, void* ctx);
        // The image opened last has been sent, or the send failed. May be null.
        void (*closeImage)(void* ctx);
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
    static ImageState imageState(const Gauge& gauge, uint32_t crc);

    // Sends a command. The gauge's answer appears in its roster entry.
    bool command(uint32_t node, Cmd cmd, uint8_t arg);
    // Shows a message over a gauge's face, or takes it down. Repeat it every
    // kAlertRepeatMs while it applies; the gauge drops it when the repeats stop.
    // False if the gauge is offline or the bus could not take every frame.
    bool alert(uint32_t node, const Alert& alert, uint8_t seq);
    // Sends the theme colours to one gauge, or to every gauge with kBroadcastNode.
    // False if nobody is listening or the bus could not take every frame.
    bool themeColours(uint32_t node, const uint32_t colours[kThemeColourCount]);
    // Shows a config on a gauge without storing it, after sending any of
    // `images` the gauge lacks. `data` must stay valid until pushBusy() is
    // false. Returns false if the gauge is offline or a push is already running.
    bool tryConfig(uint32_t node, const uint8_t* data, uint32_t size, uint32_t nowMs,
                   const uint32_t* images = nullptr, size_t imageCount = 0);
    // Asks a gauge to upload its stored config; it arrives through Handler::upload.
    bool requestUpload(uint32_t node, uint32_t nowMs);

    // A transfer is running, or a "try" is waiting for its images.
    bool pushBusy() const { return sender_.busy() || tryPending_; }
    uint32_t pushNode() const { return tryPending_ && !sender_.busy() ? tryNode_ : sender_.node(); }
    // What is being sent: Config, TryConfig or Image.
    XferKind pushKind() const { return sender_.busy() ? sender_.kind() : XferKind::TryConfig; }
    uint32_t pushFramesAcked() const { return sender_.busy() ? sender_.framesAcked() : 0; }
    uint32_t pushFramesTotal() const { return sender_.busy() ? sender_.totalFrames() : 0; }
    bool uploadBusy() const { return uploadArmed_ && uploadReceiver_.active(); }

private:
    enum class Step : uint8_t { Ready, Working, Waiting };

    Gauge* findMutable(uint32_t node);
    Gauge* findOrAdd(uint32_t node, uint32_t nowMs);
    bool send(const CanFrame& frame) { return link_.send(frame, link_.ctx); }
    void notify();
    void reconcile(uint32_t nowMs);
    void finishPush(uint32_t nowMs);
    // Works towards the gauge having every image in the list: asks about the
    // next one not known, or starts sending the next one it lacks. Ready once
    // each is there or refused; Waiting while a send is held back after a failure.
    Step images(Gauge& g, const uint32_t* crcs, size_t count, uint32_t nowMs);
    static Gauge::Image* findImage(Gauge& g, uint32_t crc);
    static void setImageState(Gauge& g, uint32_t crc, ImageState state);

    XferLink link_ = {};
    Handler handler_ = {};
    Gauge gauges_[kMaxGauges];
    uint32_t startMs_ = 0;
    uint32_t lastBeaconMs_ = 0;
    bool beaconSent_ = false;
    uint8_t beaconFlags_ = 0;

    XferSender sender_;
    bool pushActive_ = false;
    XferKind pushKind_ = XferKind::Config;
    uint32_t pushCrc_ = 0;
    uint32_t pushImages_[kMaxImages] = {};  // the images of the config being pushed
    uint8_t pushImageCount_ = 0;
    bool imageOpen_ = false;
    uint8_t session_ = 0;

    // One question about an image at a time.
    bool queryActive_ = false;
    uint32_t queryNode_ = 0;
    uint32_t queryCrc_ = 0;
    uint32_t querySentMs_ = 0;
    uint8_t queryTries_ = 0;

    // A "try" whose images are still being sent.
    bool tryPending_ = false;
    uint32_t tryNode_ = 0;
    const uint8_t* tryData_ = nullptr;
    uint32_t trySize_ = 0;
    uint32_t tryImages_[kMaxImages] = {};
    uint8_t tryImageCount_ = 0;

    XferReceiver uploadReceiver_;
    bool uploadArmed_ = false;  // uploadReceiver_ has been set up by requestUpload()
    uint32_t helloRequestNode_ = 0;
    bool helloRequestPending_ = false;
    uint32_t lastHelloRequestMs_ = 0;
};

}  // namespace proto
}  // namespace hg
