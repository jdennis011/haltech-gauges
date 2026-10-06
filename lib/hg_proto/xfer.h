#pragma once

// Segmented transfer of payloads larger than one CAN frame (configs and
// images, later fonts). Both ends are plain state machines driven by onFrame()
// and poll(), with no dependence on the CAN driver or the clock source, so
// they run unchanged on the hub, the gauge and in host tests.
//
//   sender                          receiver
//   XferBegin(session,kind,size) ->
//                                <- XferAck(Ok, next=0)
//   XferData x N-1, XferDataLast ->
//                                <- XferAck(Ok, next)       resend from `next` if short
//   ...
//   XferEnd(session, crc32)      ->
//                                <- XferAck(Done | CrcFail | Rejected)
//
// Data frames carry the low byte of their frame index. The receiver only
// stores the frame it expects next, so a lost frame makes the sender repeat
// from that point after the block's acknowledgement.

#include "proto.h"

namespace hg {
namespace proto {

struct XferLink {
    // Queues a frame for transmission. Returns false if there is no room yet;
    // the caller tries again on a later poll.
    bool (*send)(const CanFrame& frame, void* ctx);
    void* ctx;
};

// Where a sender reads its payload when it is not all in memory, e.g. an
// image file on the hub. Read a block at a time, as it goes out.
struct XferSource {
    // Copies `len` bytes starting at `offset` into `out`. False if they could not be read.
    bool (*read)(uint32_t offset, uint8_t* out, uint32_t len, void* ctx);
    void* ctx;
};

struct XferTiming {
    uint32_t beginAckMs = 200;
    uint32_t blockAckMs = 400;
    uint32_t endAckMs = 1500;     // the receiver validates and stores before answering
    uint32_t stallAckMs = 150;    // receiver re-acknowledges a block that stopped arriving
    uint32_t idleDropMs = 3000;   // receiver abandons a silent session
    uint8_t maxRetries = 8;
    uint8_t blockFrames = 32;     // 1..64
    uint8_t framesPerPoll = 8;
};

class XferSender {
public:
    enum class State : uint8_t { Idle, Begin, Data, WaitAck, End, Done, Failed };

    // Starts sending. `data` must stay valid until the transfer finishes.
    // `dir` is the direction the payload travels.
    void start(const XferLink& link, uint32_t node, Dir dir, uint8_t session, XferKind kind,
               const uint8_t* data, uint32_t size, uint32_t nowMs,
               const XferTiming& timing = XferTiming());
    // Sends a payload read from `source` a block at a time. `crc` is the
    // CRC32 of the whole payload, which the caller already knows. A failed
    // read ends the transfer with ReadFail.
    void start(const XferLink& link, uint32_t node, Dir dir, uint8_t session, XferKind kind,
               const XferSource& source, uint32_t size, uint32_t crc, uint32_t nowMs,
               const XferTiming& timing = XferTiming());
    void onFrame(const CanFrame& frame, uint32_t nowMs);
    void poll(uint32_t nowMs);
    // Stops and tells the receiver to drop the session.
    void abort();

    State state() const { return state_; }
    bool busy() const { return state_ != State::Idle && state_ != State::Done && state_ != State::Failed; }
    // Done after success; otherwise why it failed.
    XferStatus result() const { return result_; }
    uint32_t node() const { return node_; }
    XferKind kind() const { return kind_; }
    uint32_t framesAcked() const { return blockStart_; }
    uint32_t totalFrames() const { return total_; }

private:
    static constexpr uint32_t kMaxBlockBytes = 64 * kXferPayloadPerFrame;

    void begin(const XferLink& link, uint32_t node, Dir dir, uint8_t session, XferKind kind,
               uint32_t size, uint32_t nowMs, const XferTiming& timing);
    void fail(XferStatus status);
    bool retry();
    // The payload bytes of the frame at `index`, which is in the current block;
    // null if they could not be read.
    const uint8_t* payloadAt(uint32_t index);

    XferLink link_ = {};
    XferTiming timing_;
    State state_ = State::Idle;
    XferStatus result_ = XferStatus::Ok;
    Dir dir_ = Dir::HubToGauge;
    XferKind kind_ = XferKind::Config;
    uint32_t node_ = 0;
    uint8_t session_ = 0;
    const uint8_t* data_ = nullptr;  // the payload, when it is all in memory
    XferSource source_ = {};         // otherwise where it is read from
    uint8_t block_[kMaxBlockBytes];  // the block being sent, read from source_
    uint32_t blockLoaded_ = 0;       // frame index block_ starts at
    bool haveBlock_ = false;
    uint32_t size_ = 0;
    uint32_t crc_ = 0;
    uint32_t total_ = 0;       // data frames in the payload
    uint32_t blockStart_ = 0;  // first frame not yet acknowledged
    uint32_t cursor_ = 0;      // next frame to send
    uint32_t sentAtMs_ = 0;
    uint8_t retries_ = 0;
    bool needSend_ = false;    // Begin/End frame still to be queued
};

class XferReceiver {
public:
    struct Handler {
        // A transfer is starting. Return Ok and point *buffer at storage for
        // `size` bytes to take it, or any other status to refuse.
        XferStatus (*accept)(XferKind kind, uint32_t size, uint8_t** buffer, void* ctx);
        // The payload arrived and its CRC matched. Return true if it was applied.
        bool (*complete)(XferKind kind, const uint8_t* data, uint32_t size, void* ctx);
        // An accepted transfer was abandoned; the buffer can be released. May be null.
        void (*dropped)(void* ctx);
        void* ctx;
    };

    // `dataDir` is the direction the payload travels (towards this receiver).
    void init(const XferLink& link, uint32_t node, Dir dataDir, const Handler& handler,
              const XferTiming& timing = XferTiming());
    void setNode(uint32_t node) { node_ = node; }
    void onFrame(const CanFrame& frame, uint32_t nowMs);
    void poll(uint32_t nowMs);

    bool active() const { return active_; }
    uint32_t framesReceived() const { return expected_; }
    uint32_t totalFrames() const { return total_; }

private:
    void queueAck(uint8_t session, XferStatus status, uint32_t nextFrame);
    void flushAck();
    void drop();
    void finish(XferStatus status);

    XferLink link_ = {};
    XferTiming timing_;
    Handler handler_ = {};
    Dir dataDir_ = Dir::HubToGauge;
    uint32_t node_ = 0;

    bool active_ = false;
    uint8_t session_ = 0;
    XferKind kind_ = XferKind::Config;
    uint8_t* buffer_ = nullptr;
    uint32_t size_ = 0;
    uint32_t total_ = 0;
    uint32_t expected_ = 0;      // index of the next frame to store
    uint32_t lastFrameMs_ = 0;
    bool ackedSinceFrame_ = true;

    bool haveLast_ = false;      // result of the most recent finished session,
    uint8_t lastSession_ = 0;    // kept so a repeated XferEnd gets the same answer
    XferStatus lastResult_ = XferStatus::Ok;
    uint32_t lastTotal_ = 0;

    bool ackPending_ = false;
    XferAck pendingAck_;
    uint32_t lastNoSessionMs_ = 0;
    bool sentNoSession_ = false;
};

}  // namespace proto
}  // namespace hg
