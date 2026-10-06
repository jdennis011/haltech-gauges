#include "hub_manager.h"

#include <string.h>

namespace hg {
namespace proto {

namespace {

// How long to leave a gauge alone after a push failed for a transient reason.
const uint32_t kPushRetryMs = 3000;
// Two hellos with different MACs this close together mean two gauges share an id.
const uint32_t kDuplicateWindowMs = 5000;
// A question about an image is asked again if not answered this fast. After
// kQueryTries the gauge is taken not to have the image: firmware that knows
// nothing of images never answers, and then refuses the image when it comes.
const uint32_t kQueryTimeoutMs = 200;
const uint8_t kQueryTries = 3;
// Sends of one image that fail for a passing reason before it is given up on.
const uint8_t kImageFailures = 3;

// True once `now` has reached `deadline`, across the 32-bit wrap.
bool reached(uint32_t now, uint32_t deadline) { return int32_t(now - deadline) >= 0; }

bool listed(const uint32_t* list, size_t count, uint32_t crc) {
    for (size_t i = 0; i < count; i++) {
        if (list[i] == crc) return true;
    }
    return false;
}

}  // namespace

void HubManager::init(const XferLink& link, const Handler& handler, uint32_t nowMs) {
    link_ = link;
    handler_ = handler;
    for (Gauge& g : gauges_) g = Gauge();
    startMs_ = nowMs;
    beaconSent_ = false;
    pushActive_ = false;
    imageOpen_ = false;
    queryActive_ = false;
    tryPending_ = false;
    helloRequestPending_ = false;
    uploadArmed_ = false;
    sender_ = XferSender();
    uploadReceiver_ = XferReceiver();
}

void HubManager::notify() {
    if (handler_.changed) handler_.changed(handler_.ctx);
}

const HubManager::Gauge* HubManager::find(uint32_t node) const {
    for (const Gauge& g : gauges_) {
        if (g.used && g.node == node) return &g;
    }
    return nullptr;
}

HubManager::Gauge* HubManager::findMutable(uint32_t node) {
    return const_cast<Gauge*>(find(node));
}

HubManager::Gauge* HubManager::findOrAdd(uint32_t node, uint32_t nowMs) {
    if (Gauge* g = findMutable(node)) return g;

    // Prefer a free slot, otherwise reuse the gauge that has been offline longest.
    Gauge* slot = nullptr;
    for (Gauge& g : gauges_) {
        if (!g.used) {
            slot = &g;
            break;
        }
        if (!g.online && (!slot || uint32_t(nowMs - g.lastSeenMs) > uint32_t(nowMs - slot->lastSeenMs))) {
            slot = &g;
        }
    }
    if (!slot) return nullptr;
    *slot = Gauge();
    slot->used = true;
    slot->node = node;
    slot->nextPushMs = nowMs;
    return slot;
}

bool HubManager::anyOnline() const {
    for (const Gauge& g : gauges_) {
        if (g.used && g.online) return true;
    }
    return false;
}

HubManager::Gauge::Image* HubManager::findImage(Gauge& g, uint32_t crc) {
    for (size_t i = 0; i < g.imageCount; i++) {
        if (g.images[i].crc == crc) return &g.images[i];
    }
    return nullptr;
}

HubManager::ImageState HubManager::imageState(const Gauge& gauge, uint32_t crc) {
    for (size_t i = 0; i < gauge.imageCount; i++) {
        if (gauge.images[i].crc == crc) return gauge.images[i].state;
    }
    return ImageState::Unknown;
}

void HubManager::setImageState(Gauge& g, uint32_t crc, ImageState state) {
    if (Gauge::Image* im = findImage(g, crc)) {
        im->state = state;
        if (state == ImageState::Have) im->failures = 0;
        return;
    }
    const size_t room = sizeof(g.images) / sizeof(g.images[0]);
    if (g.imageCount == room) {
        // Full: forget the oldest; if it is still needed it is asked about again.
        memmove(&g.images[0], &g.images[1], (room - 1) * sizeof(g.images[0]));
        g.imageCount--;
    }
    g.images[g.imageCount++] = {crc, state, 0};
}

bool HubManager::command(uint32_t node, Cmd cmd, uint8_t arg) {
    Gauge* g = findMutable(node);
    if (!g || !g->online) return false;
    Command c;
    c.cmd = cmd;
    c.arg = arg;
    if (!send(pack(c, node))) return false;
    g->haveAck = false;
    return true;
}

bool HubManager::alert(uint32_t node, const Alert& alert, uint8_t seq) {
    const Gauge* g = find(node);
    if (!g || !g->online) return false;
    CanFrame frames[kAlertMaxFrames];
    const size_t count = packAlert(alert, seq, node, frames);
    for (size_t i = 0; i < count; i++) {
        if (!send(frames[i])) return false;
    }
    return true;
}

bool HubManager::themeColours(uint32_t node, const uint32_t colours[kThemeColourCount]) {
    if (node == kBroadcastNode) {
        if (!anyOnline()) return false;
    } else {
        const Gauge* g = find(node);
        if (!g || !g->online) return false;
    }
    CanFrame frames[kThemeColourFrames];
    const size_t count = packThemeColours(colours, node, frames);
    for (size_t i = 0; i < count; i++) {
        if (!send(frames[i])) return false;
    }
    return true;
}

bool HubManager::tryConfig(uint32_t node, const uint8_t* data, uint32_t size, uint32_t nowMs,
                           const uint32_t* images, size_t imageCount) {
    (void)nowMs;
    const Gauge* g = find(node);
    if (!g || !g->online || pushBusy()) return false;
    // Sent from poll() once the gauge has the images.
    tryPending_ = true;
    tryNode_ = node;
    tryData_ = data;
    trySize_ = size;
    tryImageCount_ = uint8_t(imageCount < kMaxImages ? imageCount : kMaxImages);
    for (size_t i = 0; i < tryImageCount_; i++) tryImages_[i] = images[i];
    return true;
}

bool HubManager::requestUpload(uint32_t node, uint32_t nowMs) {
    const Gauge* g = find(node);
    (void)nowMs;
    if (!g || !g->online || uploadBusy()) return false;
    uploadReceiver_.init(link_, node, Dir::GaugeToHub, handler_.upload);
    uploadArmed_ = true;
    return command(node, Cmd::SendConfig, 0);
}

void HubManager::onFrame(const CanFrame& frame, uint32_t nowMs) {
    if (!frame.extended || idDir(frame.id) != Dir::GaugeToHub) return;
    const uint32_t node = idNode(frame.id);
    const Msg msg = idMsg(frame.id);

    Gauge* g = findOrAdd(node, nowMs);
    if (!g) return;  // roster full of online gauges
    g->lastSeenMs = nowMs;
    if (!g->online) {
        g->online = true;
        notify();
        if (handler_.online) handler_.online(*g, handler_.ctx);
    }

    switch (msg) {
        case Msg::Hello: {
            Hello h;
            if (!unpack(frame, h)) return;
            const bool macChanged = g->haveHello && memcmp(g->mac, h.mac, 6) != 0;
            g->duplicateId = macChanged && uint32_t(nowMs - g->helloMs) < kDuplicateWindowMs;
            memcpy(g->mac, h.mac, 6);
            g->protocolVersion = h.protocolVersion;
            g->schemaVersion = h.schemaVersion;
            g->haveHello = true;
            g->helloMs = nowMs;
            // A gauge that (re)announces itself may have new firmware or empty
            // storage; forget old refusals and what it had.
            g->pushFailed = false;
            g->nextPushMs = nowMs;
            g->imageCount = 0;
            if (queryActive_ && queryNode_ == node) queryActive_ = false;
            notify();
            break;
        }
        case Msg::Info:
            if (unpack(frame, g->info)) {
                g->haveInfo = true;
                notify();
            }
            break;

        case Msg::Status: {
            Status s;
            if (!unpack(frame, s)) return;
            const bool changed = !g->haveStatus || s.configCrc != g->status.configCrc ||
                                 s.activeFace != g->status.activeFace ||
                                 s.faceCount != g->status.faceCount || s.flags != g->status.flags;
            g->status = s;
            g->haveStatus = true;
            // We restarted and this gauge is already running: ask who it is.
            if (!g->haveHello && !helloRequestPending_) {
                helloRequestPending_ = true;
                helloRequestNode_ = node;
            }
            if (changed) notify();
            break;
        }
        case Msg::Command: {
            Request rq;
            if (unpack(frame, rq) && handler_.request) handler_.request(*g, rq, handler_.ctx);
            break;
        }
        case Msg::CommandAck:
            if (unpack(frame, g->ack)) {
                g->haveAck = true;
                notify();
            }
            break;

        case Msg::Asset: {
            AssetReply r;
            if (!unpack(frame, r) || r.kind != XferKind::Image) return;
            if (queryActive_ && queryNode_ == node && queryCrc_ == r.crc) queryActive_ = false;
            setImageState(*g, r.crc, r.have ? ImageState::Have : ImageState::Missing);
            notify();
            break;
        }

        case Msg::XferAck:
            sender_.onFrame(frame, nowMs);
            break;

        case Msg::XferBegin:
        case Msg::XferData:
        case Msg::XferDataLast:
        case Msg::XferEnd:
        case Msg::XferAbort:
            if (uploadArmed_) uploadReceiver_.onFrame(frame, nowMs);
            break;

        default:
            break;
    }
}

void HubManager::finishPush(uint32_t nowMs) {
    pushActive_ = false;
    if (imageOpen_) {
        imageOpen_ = false;
        if (handler_.closeImage) handler_.closeImage(handler_.ctx);
    }
    Gauge* g = findMutable(sender_.node());
    if (!g) return;
    const XferStatus result = sender_.result();

    if (pushKind_ == XferKind::Image) {
        switch (result) {
            case XferStatus::Done:
                setImageState(*g, pushCrc_, ImageState::Have);
                break;
            case XferStatus::Rejected:
            case XferStatus::TooBig:
            case XferStatus::BadKind:
                // No room, or firmware without images: the config goes without it.
                setImageState(*g, pushCrc_, ImageState::Refused);
                break;
            default: {
                Gauge::Image* im = findImage(*g, pushCrc_);
                if (im && ++im->failures >= kImageFailures) im->state = ImageState::Refused;
                g->nextPushMs = nowMs + kPushRetryMs;
                break;
            }
        }
        notify();
        return;
    }

    g->pushResult = result;
    if (pushKind_ == XferKind::TryConfig) {
        notify();
        return;
    }

    switch (result) {
        case XferStatus::Done: {
            // The gauge stored it; its next status will confirm. Storing a
            // config, the gauge deletes the images it no longer uses.
            g->status.configCrc = pushCrc_;
            g->pushFailed = false;
            size_t kept = 0;
            for (size_t i = 0; i < g->imageCount; i++) {
                const Gauge::Image& im = g->images[i];
                if (im.state == ImageState::Have && !listed(pushImages_, pushImageCount_, im.crc)) continue;
                g->images[kept++] = im;
            }
            g->imageCount = uint8_t(kept);
            break;
        }
        case XferStatus::Rejected:
        case XferStatus::TooBig:
        case XferStatus::BadKind:
            // Sending the same bytes again would get the same answer.
            g->pushFailed = true;
            g->failedCrc = pushCrc_;
            break;
        default:
            g->nextPushMs = nowMs + kPushRetryMs;
            break;
    }
    notify();
}

HubManager::Step HubManager::images(Gauge& g, const uint32_t* crcs, size_t count, uint32_t nowMs) {
    for (size_t i = 0; i < count; i++) {
        const uint32_t crc = crcs[i];
        switch (imageState(g, crc)) {
            case ImageState::Have:
            case ImageState::Refused:
                break;

            case ImageState::Unknown: {
                if (queryActive_) return Step::Working;  // an answer is awaited first
                AssetQuery q;
                q.crc = crc;
                if (send(pack(q, g.node))) {
                    queryActive_ = true;
                    queryNode_ = g.node;
                    queryCrc_ = crc;
                    querySentMs_ = nowMs;
                    queryTries_ = 1;
                }
                return Step::Working;
            }

            case ImageState::Missing: {
                if (!reached(nowMs, g.nextPushMs)) return Step::Waiting;
                XferSource source = {};
                uint32_t size = 0;
                if (!handler_.openImage || !handler_.openImage(crc, source, size, handler_.ctx)) {
                    // The hub no longer has it: the config goes without it.
                    setImageState(g, crc, ImageState::Refused);
                    break;
                }
                imageOpen_ = true;
                pushActive_ = true;
                pushKind_ = XferKind::Image;
                pushCrc_ = crc;
                sender_.start(link_, g.node, Dir::HubToGauge, ++session_, XferKind::Image, source, size,
                              crc, nowMs);
                notify();
                return Step::Working;
            }
        }
    }
    return Step::Ready;
}

void HubManager::reconcile(uint32_t nowMs) {
    for (Gauge& g : gauges_) {
        if (!g.used || !g.online || !g.haveHello || !g.haveStatus) continue;
        if (g.protocolVersion != kProtocolVersion) continue;
        if (g.status.flags & kStatusFlagTrying) continue;  // don't interrupt a preview
        if (!reached(nowMs, g.nextPushMs)) continue;

        Desired d;
        if (!handler_.desired(g, d, handler_.ctx)) continue;
        const bool inSync = d.crc == g.status.configCrc;
        if (!inSync) {
            if (g.pushFailed && g.failedCrc == d.crc) continue;
            if (d.schema > g.schemaVersion) {
                // The gauge's firmware is too old to understand this config.
                g.pushFailed = true;
                g.failedCrc = d.crc;
                g.pushResult = XferStatus::Rejected;
                notify();
                continue;
            }
        }

        // The images first, so the face is whole when the config lands. A
        // gauge already in sync is checked too: it may have lost one.
        const size_t imageCount = d.imageCount < kMaxImages ? d.imageCount : kMaxImages;
        const Step step = images(g, d.images, imageCount, nowMs);
        if (step == Step::Working) return;  // one thing at a time
        if (step == Step::Waiting) continue;

        if (inSync) {
            g.pushFailed = false;
            continue;
        }
        pushActive_ = true;
        pushKind_ = XferKind::Config;
        pushCrc_ = d.crc;
        pushImageCount_ = uint8_t(imageCount);
        for (size_t i = 0; i < imageCount; i++) pushImages_[i] = d.images[i];
        sender_.start(link_, g.node, Dir::HubToGauge, ++session_, XferKind::Config, d.data, d.size,
                      nowMs);
        return;  // one transfer at a time
    }
}

void HubManager::poll(uint32_t nowMs) {
    if (!beaconSent_ || uint32_t(nowMs - lastBeaconMs_) >= kBeaconPeriodMs) {
        Beacon b;
        b.flags = beaconFlags_;
        b.uptimeSec = uint16_t(uint32_t(nowMs - startMs_) / 1000);
        if (send(pack(b))) {
            beaconSent_ = true;
            lastBeaconMs_ = nowMs;
        }
    }

    if (helloRequestPending_ && uint32_t(nowMs - lastHelloRequestMs_) >= 250) {
        if (send(packHelloRequest(helloRequestNode_))) {
            helloRequestPending_ = false;
            lastHelloRequestMs_ = nowMs;
        }
    }

    for (Gauge& g : gauges_) {
        if (g.used && g.online && uint32_t(nowMs - g.lastSeenMs) >= kGaugeTimeoutMs) {
            g.online = false;
            notify();
        }
    }

    if (queryActive_ && uint32_t(nowMs - querySentMs_) >= kQueryTimeoutMs) {
        Gauge* g = findMutable(queryNode_);
        if (!g || !g->online) {
            queryActive_ = false;
        } else if (queryTries_ < kQueryTries) {
            AssetQuery q;
            q.crc = queryCrc_;
            if (send(pack(q, queryNode_))) {
                queryTries_++;
                querySentMs_ = nowMs;
            }
        } else {
            setImageState(*g, queryCrc_, ImageState::Missing);
            queryActive_ = false;
        }
    }

    sender_.poll(nowMs);
    if (uploadArmed_) uploadReceiver_.poll(nowMs);

    if (pushActive_ && !sender_.busy()) finishPush(nowMs);
    if (sender_.busy() || uploadBusy()) return;

    if (tryPending_) {
        // A "try" waits for its images, then goes.
        Gauge* g = findMutable(tryNode_);
        if (!g || !g->online) {
            tryPending_ = false;
            return;
        }
        if (images(*g, tryImages_, tryImageCount_, nowMs) != Step::Ready) return;
        tryPending_ = false;
        pushActive_ = true;
        pushKind_ = XferKind::TryConfig;
        sender_.start(link_, tryNode_, Dir::HubToGauge, ++session_, XferKind::TryConfig, tryData_,
                      trySize_, nowMs);
        return;
    }
    reconcile(nowMs);
}

}  // namespace proto
}  // namespace hg
