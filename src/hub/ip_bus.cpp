#include "ip_bus.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <string.h>

#include "ip_frames.h"
#include "proto.h"

namespace ip_bus {

namespace {

const uint32_t kPeerTimeoutMs = 10000;
const uint32_t kFlushMs = 5;       // management frames wait at most this long for company
const size_t kFlushAt = 32;        // or go when this many are queued
// Live data goes as the latest frame per id, twenty times a second: all a
// gauge can show, and a tenth of the bus's rate over the air.
const uint32_t kLiveFlushMs = 50;
const size_t kLiveIds = 64;
const size_t kMaxPeers = 8;
const size_t kRxQueue = 256;

struct Peer {
    bool used = false;
    sockaddr_in addr = {};
    uint32_t lastMs = 0;
    uint32_t node = 0;  // the last node id heard from it
};

int sock = -1;
QueueHandle_t rxQueue = nullptr;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
Peer peerTable[kMaxPeers];
hg::CanFrame txBuf[hg::kIpMaxFrames];
size_t txCount = 0;
uint32_t txFirstMs = 0;
hg::CanFrame liveBuf[kLiveIds];  // one slot per id, the newest frame kept
bool liveUsed[kLiveIds] = {};
size_t liveCount = 0;
uint32_t liveFlushMs = 0;
uint32_t rxFrames = 0, txPackets = 0, rxDropped = 0, txDropped = 0;

bool samePeer(const Peer& p, const sockaddr_in& a) {
    return p.used && p.addr.sin_addr.s_addr == a.sin_addr.s_addr && p.addr.sin_port == a.sin_port;
}

void notePeer(const sockaddr_in& from, uint32_t node, uint32_t nowMs) {
    portENTER_CRITICAL(&mux);
    Peer* slot = nullptr;
    for (Peer& p : peerTable) {
        if (samePeer(p, from)) {
            slot = &p;
            break;
        }
    }
    if (!slot) {
        for (Peer& p : peerTable) {
            if (!p.used || uint32_t(nowMs - p.lastMs) >= kPeerTimeoutMs) {
                slot = &p;
                break;
            }
        }
    }
    if (slot) {
        const bool fresh = !samePeer(*slot, from);
        slot->used = true;
        slot->addr = from;
        slot->lastMs = nowMs;
        if (node) slot->node = node;
        if (fresh) {
            char ip[16];
            inet_ntoa_r(from.sin_addr, ip, sizeof(ip));
            portEXIT_CRITICAL(&mux);
            Serial.printf("ip bus: gauge at %s:%u\n", ip, unsigned(ntohs(from.sin_port)));
            return;
        }
    }
    portEXIT_CRITICAL(&mux);
}

void rxTask(void*) {
    uint8_t buf[hg::kIpMaxPacket + 16];
    hg::CanFrame frames[hg::kIpMaxFrames];
    for (;;) {
        sockaddr_in from = {};
        socklen_t fromLen = sizeof(from);
        const int n = recvfrom(sock, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&from), &fromLen);
        if (n < 0) {
            vTaskDelay(1);  // a timeout, or no network yet
            continue;
        }
        const int count = hg::unpackIpFrames(buf, size_t(n), frames, hg::kIpMaxFrames);
        if (count < 0) continue;
        const uint32_t now = millis();
        uint32_t node = 0;
        for (int i = 0; i < count; i++) {
            if (frames[i].extended) node = hg::proto::idNode(frames[i].id);
            if (xQueueSend(rxQueue, &frames[i], 0) == pdTRUE) {
                rxFrames++;
            } else {
                rxDropped++;
            }
        }
        notePeer(from, node, now);
    }
}

}  // namespace

void begin() {
    rxQueue = xQueueCreate(kRxQueue, sizeof(hg::CanFrame));
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        Serial.println("ip bus: could not open a socket");
        return;
    }
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(hg::kIpBusPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        Serial.println("ip bus: could not bind the port");
        close(sock);
        sock = -1;
        return;
    }
    timeval tv = {0, 200000};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    xTaskCreate(rxTask, "ipbus", 4096, nullptr, 5, nullptr);
    Serial.printf("Gauge bus over Wi-Fi on UDP port %u\n", unsigned(hg::kIpBusPort));
}

bool read(hg::CanFrame& out) {
    return rxQueue && xQueueReceive(rxQueue, &out, 0) == pdTRUE;
}

bool send(const hg::CanFrame& frame) {
    if (sock < 0) return false;
    const uint32_t now = millis();
    bool any = false;
    portENTER_CRITICAL(&mux);
    for (const Peer& p : peerTable) {
        if (p.used && uint32_t(now - p.lastMs) < kPeerTimeoutMs) {
            any = true;
            break;
        }
    }
    if (any) {
        if (!frame.extended) {
            // Live data: the newest frame for this id replaces the one waiting.
            size_t slot = kLiveIds;
            for (size_t i = 0; i < kLiveIds; i++) {
                if (liveUsed[i] && liveBuf[i].id == frame.id) {
                    slot = i;
                    break;
                }
            }
            if (slot == kLiveIds) {
                for (size_t i = 0; i < kLiveIds; i++) {
                    if (!liveUsed[i]) {
                        slot = i;
                        break;
                    }
                }
            }
            if (slot == kLiveIds) {
                txDropped++;
            } else {
                if (!liveUsed[slot]) {
                    liveUsed[slot] = true;
                    liveCount++;
                }
                liveBuf[slot] = frame;
            }
        } else if (txCount < hg::kIpMaxFrames) {
            if (txCount == 0) txFirstMs = now;
            txBuf[txCount++] = frame;
        } else {
            txDropped++;
        }
    }
    portEXIT_CRITICAL(&mux);
    return any;
}

void flush(uint32_t nowMs) {
    if (sock < 0) return;
    hg::CanFrame local[hg::kIpMaxFrames];
    size_t n = 0;
    sockaddr_in targets[kMaxPeers];
    size_t targetCount = 0;
    portENTER_CRITICAL(&mux);
    if (txCount && (txCount >= kFlushAt || uint32_t(nowMs - txFirstMs) >= kFlushMs)) {
        n = txCount;
        memcpy(local, txBuf, n * sizeof(hg::CanFrame));
        txCount = 0;
    }
    if (liveCount && uint32_t(nowMs - liveFlushMs) >= kLiveFlushMs) {
        liveFlushMs = nowMs;
        for (size_t i = 0; i < kLiveIds && n < hg::kIpMaxFrames; i++) {
            if (!liveUsed[i]) continue;
            local[n++] = liveBuf[i];
            liveUsed[i] = false;
        }
        liveCount = 0;
    }
    if (n) {
        for (const Peer& p : peerTable) {
            if (p.used && uint32_t(nowMs - p.lastMs) < kPeerTimeoutMs) targets[targetCount++] = p.addr;
        }
    }
    portEXIT_CRITICAL(&mux);
    if (!n || !targetCount) return;
    uint8_t packet[hg::kIpMaxPacket];
    const size_t len = hg::packIpFrames(local, n, packet, sizeof(packet));
    if (!len) return;
    for (size_t i = 0; i < targetCount; i++) {
        sendto(sock, packet, len, 0, reinterpret_cast<sockaddr*>(&targets[i]), sizeof(targets[i]));
        txPackets++;
    }
}

size_t peers() {
    const uint32_t now = millis();
    size_t n = 0;
    portENTER_CRITICAL(&mux);
    for (const Peer& p : peerTable) {
        if (p.used && uint32_t(now - p.lastMs) < kPeerTimeoutMs) n++;
    }
    portEXIT_CRITICAL(&mux);
    return n;
}

bool isIpNode(uint32_t node) {
    const uint32_t now = millis();
    bool found = false;
    portENTER_CRITICAL(&mux);
    for (const Peer& p : peerTable) {
        if (p.used && p.node == node && uint32_t(now - p.lastMs) < kPeerTimeoutMs) {
            found = true;
            break;
        }
    }
    portEXIT_CRITICAL(&mux);
    return found;
}

void statusJson(JsonObject out) {
    out["port"] = hg::kIpBusPort;
    JsonArray list = out["peers"].to<JsonArray>();
    const uint32_t now = millis();
    Peer copy[kMaxPeers];
    portENTER_CRITICAL(&mux);
    memcpy(copy, peerTable, sizeof(copy));
    portEXIT_CRITICAL(&mux);
    for (const Peer& p : copy) {
        if (!p.used || uint32_t(now - p.lastMs) >= kPeerTimeoutMs) continue;
        JsonObject o = list.add<JsonObject>();
        char ip[16];
        inet_ntoa_r(p.addr.sin_addr, ip, sizeof(ip));
        o["ip"] = ip;
        char node[8];
        snprintf(node, sizeof(node), "%06lX", (unsigned long)p.node);
        o["node"] = node;
        o["age_s"] = (now - p.lastMs) / 1000;
    }
    out["rx_frames"] = rxFrames;
    out["tx_packets"] = txPackets;
    out["dropped"] = rxDropped + txDropped;
}

}  // namespace ip_bus
