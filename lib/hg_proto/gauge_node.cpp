#include "gauge_node.h"

#include <string.h>

namespace hg {
namespace proto {

uint32_t GaugeNode::nodeFromMac(const uint8_t mac[6]) {
    uint32_t node = (uint32_t(mac[3]) << 16) | (uint32_t(mac[4]) << 8) | mac[5];
    // The broadcast id is reserved.
    if (node == kBroadcastNode) node = 0xFFFFFE;
    return node;
}

void GaugeNode::init(const XferLink& link, const uint8_t mac[6], uint32_t node, const Info& info,
                     uint8_t schemaVersion, const Handler& handler) {
    link_ = link;
    handler_ = handler;
    memcpy(hello_.mac, mac, 6);
    hello_.protocolVersion = kProtocolVersion;
    hello_.schemaVersion = schemaVersion;
    info_ = info;
    node_ = node;
    haveBeacon_ = false;
    hubPresent_ = false;
    helloPending_ = infoPending_ = ackPending_ = statusDue_ = false;
    conflict_ = false;
    receiver_.init(link, node, Dir::HubToGauge, handler.transfer);
}

void GaugeNode::setNode(uint32_t node) {
    node_ = node;
    conflict_ = false;
    receiver_.setNode(node);
    helloPending_ = infoPending_ = true;
    statusDue_ = true;
}

bool GaugeNode::upload(const uint8_t* data, uint32_t size, uint32_t nowMs) {
    if (sender_.busy()) return false;
    sender_.start(link_, node_, Dir::GaugeToHub, ++uploadSession_, XferKind::Config, data, size,
                  nowMs);
    return sender_.busy();
}

void GaugeNode::onFrame(const CanFrame& frame, uint32_t nowMs) {
    if (!frame.extended) return;
    const Msg msg = idMsg(frame.id);
    const uint32_t node = idNode(frame.id);

    if (idDir(frame.id) == Dir::GaugeToHub) {
        // A CAN controller does not receive its own frames, so this came from
        // another gauge using our id.
        if (node == node_) conflict_ = true;
        return;
    }

    if (msg == Msg::Beacon) {
        Beacon b;
        if (!unpack(frame, b)) return;
        // Announce ourselves when a hub appears, or when it has restarted.
        if (!hubPresent_ || b.uptimeSec < lastUptime_) {
            helloPending_ = infoPending_ = true;
            statusDue_ = true;
        }
        haveBeacon_ = true;
        hubPresent_ = true;
        lastBeaconMs_ = nowMs;
        lastUptime_ = b.uptimeSec;
        return;
    }

    if (node != node_ && node != kBroadcastNode) return;

    switch (msg) {
        case Msg::Hello:
            helloPending_ = infoPending_ = true;
            break;

        case Msg::Command: {
            Command c;
            if (node != node_ || !unpack(frame, c)) return;
            ack_.cmd = c.cmd;
            ack_.ok = handler_.command(c.cmd, c.arg, handler_.ctx);
            ackPending_ = true;
            statusDue_ = true;  // let the hub see the effect promptly
            break;
        }

        case Msg::XferBegin:
        case Msg::XferData:
        case Msg::XferDataLast:
        case Msg::XferEnd:
        case Msg::XferAbort:
            if (node == node_) receiver_.onFrame(frame, nowMs);
            break;

        case Msg::XferAck:
            sender_.onFrame(frame, nowMs);
            break;

        default:
            break;
    }
}

void GaugeNode::poll(uint32_t nowMs) {
    hubPresent_ = haveBeacon_ && uint32_t(nowMs - lastBeaconMs_) < kBeaconTimeoutMs;
    if (!hubPresent_) return;

    if (helloPending_ && send(pack(hello_, node_))) helloPending_ = false;
    if (infoPending_ && send(pack(info_, node_))) infoPending_ = false;
    if (ackPending_ && send(pack(ack_, node_))) ackPending_ = false;

    if (statusDue_ || uint32_t(nowMs - lastStatusMs_) >= kStatusPeriodMs) {
        Status s;
        handler_.status(s, handler_.ctx);
        if (send(pack(s, node_))) {
            lastStatusMs_ = nowMs;
            statusDue_ = false;
        }
    }

    receiver_.poll(nowMs);
    sender_.poll(nowMs);
}

}  // namespace proto
}  // namespace hg
