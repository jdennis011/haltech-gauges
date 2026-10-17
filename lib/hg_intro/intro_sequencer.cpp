#include "intro_sequencer.h"

#include <string.h>

namespace hg {
namespace intro {

using proto::HubManager;

void Sequencer::begin(uint32_t nowMs, size_t expected) {
    startMs_ = nowMs;
    firstHelloMs_ = 0;
    anyHello_ = false;
    expected_ = expected;
    done_ = false;
}

void Sequencer::setOrder(const Slot* slots, size_t count) {
    orderCount_ = count < kMaxSlots ? count : kMaxSlots;
    for (size_t i = 0; i < orderCount_; i++) order_[i] = slots[i];
}

// Every online gauge that has said hello, in display order: the ones in the
// given order first, the rest by when they announced themselves.
size_t Sequencer::ranked(const HubManager& manager, Ranked* out) const {
    size_t n = 0;
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = manager.gauge(i);
        if (!g.used || !g.online || !g.haveHello) continue;
        uint32_t rank = 0x100000u + (uint32_t(g.helloMs - startMs_) & 0xFFFFFu);
        for (size_t k = 0; k < orderCount_; k++) {
            if (memcmp(order_[k].mac, g.mac, 6) == 0) {
                rank = uint32_t(k);
                break;
            }
        }
        // Insertion sort: eight entries at most.
        size_t at = n;
        while (at > 0 && out[at - 1].rank > rank) {
            out[at] = out[at - 1];
            at--;
        }
        out[at] = {g.node, rank, g.mac};
        n++;
    }
    return n;
}

size_t Sequencer::send(HubManager& manager, bool force) {
    Ranked list[kMaxSlots];
    const size_t n = ranked(manager, list);
    size_t sent = 0;
    for (size_t i = 0; i < n; i++) {
        proto::Intro m;
        m.style = settings_.style;
        m.force = force;
        const uint32_t delay = uint32_t(i) * settings_.staggerMs;
        m.delayMs = uint16_t(delay > 0xFFFF ? 0xFFFF : delay);
        m.durationMs = settings_.durationMs;
        m.index = uint8_t(i);
        m.count = uint8_t(n);
        if (manager.intro(list[i].node, m)) sent++;
    }
    return sent;
}

bool Sequencer::poll(HubManager& manager, uint32_t nowMs) {
    if (done_) return false;
    size_t heard = 0;
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = manager.gauge(i);
        if (g.used && g.online && g.haveHello) heard++;
    }
    if (heard > 0 && !anyHello_) {
        anyHello_ = true;
        firstHelloMs_ = nowMs;
    }
    if (!anyHello_) {
        if (uint32_t(nowMs - startMs_) >= kGiveUpMs) done_ = true;  // nobody: late arrivals play alone
        return false;
    }
    const bool everyone = expected_ > 0 && heard >= expected_;
    const bool settled = uint32_t(nowMs - firstHelloMs_) >= kSettleMs;
    if (!everyone && !settled) return false;
    done_ = true;
    if (!settings_.enabled) return false;
    send(manager, false);
    return true;
}

size_t Sequencer::play(HubManager& manager, uint32_t nowMs) {
    (void)nowMs;
    return send(manager, true);
}

int Sequencer::slotOf(const HubManager& manager, const uint8_t mac[6]) const {
    Ranked list[kMaxSlots];
    const size_t n = ranked(manager, list);
    for (size_t i = 0; i < n; i++) {
        if (memcmp(list[i].mac, mac, 6) == 0) return int(i);
    }
    return -1;
}

}  // namespace intro
}  // namespace hg
