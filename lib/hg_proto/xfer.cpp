#include "xfer.h"

#include <string.h>

#include "crc32.h"

namespace hg {
namespace proto {

namespace {

constexpr Dir opposite(Dir d) { return d == Dir::HubToGauge ? Dir::GaugeToHub : Dir::HubToGauge; }

uint32_t frameCountFor(uint32_t size) {
    return (size + kXferPayloadPerFrame - 1) / kXferPayloadPerFrame;
}

}  // namespace

// ---------------------------------------------------------------- sender

void XferSender::start(const XferLink& link, uint32_t node, Dir dir, uint8_t session,
                       XferKind kind, const uint8_t* data, uint32_t size, uint32_t nowMs,
                       const XferTiming& timing) {
    link_ = link;
    timing_ = timing;
    if (timing_.blockFrames < 1) timing_.blockFrames = 1;
    if (timing_.blockFrames > 64) timing_.blockFrames = 64;
    if (timing_.framesPerPoll < 1) timing_.framesPerPoll = 1;
    node_ = node;
    dir_ = dir;
    session_ = session;
    kind_ = kind;
    data_ = data;
    size_ = size;
    crc_ = crc32(data, size);
    total_ = frameCountFor(size);
    blockStart_ = 0;
    cursor_ = 0;
    retries_ = 0;
    sentAtMs_ = nowMs;
    result_ = XferStatus::Ok;
    if (size > kXferMaxSize) {
        fail(XferStatus::TooBig);
        return;
    }
    state_ = State::Begin;
    needSend_ = true;
}

void XferSender::fail(XferStatus status) {
    state_ = State::Failed;
    result_ = status;
}

bool XferSender::retry() {
    if (retries_ >= timing_.maxRetries) {
        fail(XferStatus::Timeout);
        return false;
    }
    retries_++;
    return true;
}

void XferSender::abort() {
    if (!busy()) return;
    XferAbort m;
    m.session = session_;
    link_.send(pack(m, dir_, node_), link_.ctx);
    fail(XferStatus::Timeout);
}

void XferSender::poll(uint32_t nowMs) {
    switch (state_) {
        case State::Begin:
        case State::End: {
            const uint32_t timeout = state_ == State::Begin ? timing_.beginAckMs : timing_.endAckMs;
            if (!needSend_ && uint32_t(nowMs - sentAtMs_) >= timeout) {
                if (!retry()) return;
                needSend_ = true;
            }
            if (needSend_) {
                CanFrame f;
                if (state_ == State::Begin) {
                    XferBegin m;
                    m.session = session_;
                    m.kind = kind_;
                    m.size = size_;
                    m.blockFrames = timing_.blockFrames;
                    f = pack(m, dir_, node_);
                } else {
                    XferEnd m;
                    m.session = session_;
                    m.crc = crc_;
                    f = pack(m, dir_, node_);
                }
                if (link_.send(f, link_.ctx)) {
                    needSend_ = false;
                    sentAtMs_ = nowMs;
                }
            }
            break;
        }

        case State::Data: {
            uint32_t blockEnd = blockStart_ + timing_.blockFrames;
            if (blockEnd > total_) blockEnd = total_;
            for (uint8_t n = 0; n < timing_.framesPerPoll && cursor_ < blockEnd; n++) {
                const uint32_t offset = cursor_ * kXferPayloadPerFrame;
                uint32_t len = size_ - offset;
                if (len > kXferPayloadPerFrame) len = kXferPayloadPerFrame;
                const bool last = cursor_ + 1 == blockEnd;
                const CanFrame f =
                    packXferData(dir_, node_, uint8_t(cursor_), data_ + offset, uint8_t(len), last);
                if (!link_.send(f, link_.ctx)) break;
                cursor_++;
            }
            if (cursor_ >= blockEnd) {
                state_ = State::WaitAck;
                sentAtMs_ = nowMs;
            }
            break;
        }

        case State::WaitAck:
            if (uint32_t(nowMs - sentAtMs_) >= timing_.blockAckMs) {
                if (!retry()) return;
                cursor_ = blockStart_;
                state_ = State::Data;
            }
            break;

        case State::Idle:
        case State::Done:
        case State::Failed:
            break;
    }
}

void XferSender::onFrame(const CanFrame& frame, uint32_t nowMs) {
    (void)nowMs;
    if (!busy() || !frame.extended) return;
    if (idMsg(frame.id) != Msg::XferAck || idDir(frame.id) != opposite(dir_) ||
        idNode(frame.id) != node_) {
        return;
    }
    XferAck ack;
    if (!unpack(frame, ack)) return;

    // The receiver has no session at all (it restarted, or gave up on us).
    // It cannot name our session, so this is accepted without a session match
    // once we are past Begin.
    if (ack.status == XferStatus::NoSession) {
        if (state_ != State::Begin) fail(XferStatus::NoSession);
        return;
    }
    if (ack.session != session_) return;

    const bool failure = ack.status != XferStatus::Ok && ack.status != XferStatus::Done;
    if (failure) {
        fail(ack.status);
        return;
    }

    switch (state_) {
        case State::Begin:
            if (ack.status != XferStatus::Ok || ack.nextFrame > total_) return;
            blockStart_ = cursor_ = ack.nextFrame;
            retries_ = 0;
            if (blockStart_ == total_) {
                state_ = State::End;
                needSend_ = true;
            } else {
                state_ = State::Data;
            }
            break;

        case State::WaitAck:
            // The receiver's count only ever grows, so anything lower is stale.
            if (ack.status != XferStatus::Ok || ack.nextFrame < blockStart_ ||
                ack.nextFrame > total_) {
                return;
            }
            if (ack.nextFrame > blockStart_) {
                retries_ = 0;
            } else if (!retry()) {
                return;
            }
            blockStart_ = cursor_ = ack.nextFrame;
            if (blockStart_ == total_) {
                state_ = State::End;
                needSend_ = true;
            } else {
                state_ = State::Data;
            }
            break;

        case State::End:
            if (ack.status == XferStatus::Done) {
                state_ = State::Done;
                result_ = XferStatus::Done;
                blockStart_ = total_;
            } else if (ack.nextFrame < total_ && ack.nextFrame >= blockStart_) {
                // The receiver is still missing frames.
                if (!retry()) return;
                blockStart_ = cursor_ = ack.nextFrame;
                state_ = State::Data;
            }
            break;

        case State::Data:  // block acknowledgements are only acted on once the block is out
        case State::Idle:
        case State::Done:
        case State::Failed:
            break;
    }
}

// -------------------------------------------------------------- receiver

void XferReceiver::init(const XferLink& link, uint32_t node, Dir dataDir, const Handler& handler,
                        const XferTiming& timing) {
    link_ = link;
    node_ = node;
    dataDir_ = dataDir;
    handler_ = handler;
    timing_ = timing;
    active_ = false;
    haveLast_ = false;
    ackPending_ = false;
    sentNoSession_ = false;
}

void XferReceiver::queueAck(uint8_t session, XferStatus status, uint32_t nextFrame) {
    pendingAck_.session = session;
    pendingAck_.status = status;
    pendingAck_.nextFrame = nextFrame;
    ackPending_ = true;
    flushAck();
}

void XferReceiver::flushAck() {
    if (!ackPending_) return;
    if (link_.send(pack(pendingAck_, opposite(dataDir_), node_), link_.ctx)) ackPending_ = false;
}

void XferReceiver::drop() {
    if (!active_) return;
    active_ = false;
    buffer_ = nullptr;
    if (handler_.dropped) handler_.dropped(handler_.ctx);
}

void XferReceiver::finish(XferStatus status) {
    active_ = false;
    haveLast_ = true;
    lastSession_ = session_;
    lastResult_ = status;
    lastTotal_ = total_;
    buffer_ = nullptr;
    queueAck(session_, status, total_);
}

void XferReceiver::onFrame(const CanFrame& frame, uint32_t nowMs) {
    if (!frame.extended || idDir(frame.id) != dataDir_ || idNode(frame.id) != node_) return;

    switch (idMsg(frame.id)) {
        case Msg::XferBegin: {
            XferBegin m;
            if (!unpack(frame, m)) return;
            if (active_ && m.session == session_) {
                // Our first acknowledgement was lost; repeat it.
                lastFrameMs_ = nowMs;
                queueAck(session_, XferStatus::Ok, expected_);
                return;
            }
            drop();
            uint8_t* buffer = nullptr;
            XferStatus status = handler_.accept(m.kind, m.size, &buffer, handler_.ctx);
            if (status == XferStatus::Ok && !buffer && m.size > 0) status = XferStatus::Busy;
            if (status != XferStatus::Ok) {
                queueAck(m.session, status, 0);
                return;
            }
            active_ = true;
            haveLast_ = false;
            session_ = m.session;
            kind_ = m.kind;
            buffer_ = buffer;
            size_ = m.size;
            total_ = frameCountFor(m.size);
            expected_ = 0;
            lastFrameMs_ = nowMs;
            ackedSinceFrame_ = true;
            queueAck(session_, XferStatus::Ok, 0);
            break;
        }

        case Msg::XferData:
        case Msg::XferDataLast: {
            if (!active_) {
                if (!sentNoSession_ || uint32_t(nowMs - lastNoSessionMs_) >= 100) {
                    sentNoSession_ = true;
                    lastNoSessionMs_ = nowMs;
                    queueAck(0, XferStatus::NoSession, 0);
                }
                return;
            }
            if (frame.len < 1) return;
            lastFrameMs_ = nowMs;
            ackedSinceFrame_ = false;

            // Rebuild the full frame index from its low byte. The sender never
            // runs more than one block (<= 64 frames) either side of expected_.
            const int32_t delta = int8_t(uint8_t(frame.data[0] - uint8_t(expected_)));
            if (delta == 0 && expected_ < total_) {
                const uint32_t offset = expected_ * kXferPayloadPerFrame;
                uint32_t want = size_ - offset;
                if (want > kXferPayloadPerFrame) want = kXferPayloadPerFrame;
                if (uint32_t(frame.len - 1) == want) {
                    memcpy(buffer_ + offset, frame.data + 1, want);
                    expected_++;
                }
            }
            if (idMsg(frame.id) == Msg::XferDataLast) {
                ackedSinceFrame_ = true;
                queueAck(session_, XferStatus::Ok, expected_);
            }
            break;
        }

        case Msg::XferEnd: {
            XferEnd m;
            if (!unpack(frame, m)) return;
            if (active_ && m.session == session_) {
                lastFrameMs_ = nowMs;
                if (expected_ < total_) {
                    ackedSinceFrame_ = true;
                    queueAck(session_, XferStatus::Ok, expected_);
                    return;
                }
                if (crc32(buffer_, size_) != m.crc) {
                    if (handler_.dropped) handler_.dropped(handler_.ctx);
                    finish(XferStatus::CrcFail);
                    return;
                }
                const bool ok = handler_.complete(kind_, buffer_, size_, handler_.ctx);
                finish(ok ? XferStatus::Done : XferStatus::Rejected);
            } else if (!active_ && haveLast_ && m.session == lastSession_) {
                queueAck(lastSession_, lastResult_, lastTotal_);
            } else {
                queueAck(m.session, XferStatus::NoSession, 0);
            }
            break;
        }

        case Msg::XferAbort: {
            XferAbort m;
            if (unpack(frame, m) && active_ && m.session == session_) drop();
            break;
        }

        default:
            break;
    }
}

void XferReceiver::poll(uint32_t nowMs) {
    flushAck();
    if (!active_) return;
    const uint32_t quiet = uint32_t(nowMs - lastFrameMs_);
    if (quiet >= timing_.idleDropMs) {
        drop();
        return;
    }
    // A block stopped part-way (its last frame was lost): tell the sender
    // where we are instead of waiting for it to time out.
    if (!ackedSinceFrame_ && quiet >= timing_.stallAckMs) {
        ackedSinceFrame_ = true;
        queueAck(session_, XferStatus::Ok, expected_);
    }
}

}  // namespace proto
}  // namespace hg
