#include "wheel_controller.h"

#include <string.h>

using namespace hg::proto;

namespace hg {
namespace wheel {

void Controller::init(const Outputs& outputs, const Settings& settings) {
    outputs_ = outputs;
    settings_ = settings;
    state_ = State();
    held_ = 0;
    lastSeq_ = 0;
    for (Pulse& p : pulse_) p = Pulse();
}

void Controller::setSettings(const Settings& settings, uint32_t nowMs) {
    (void)nowMs;
    settings_ = settings;
    apply();
}

uint16_t Controller::wheelMask() const {
    uint16_t m = 0;
    // The horn only from a calibrated wheel with no line fault: a shorted
    // ladder line must not sound it.
    const bool trusted = (state_.flags & (kInputFlagFault | kInputFlagUncalibrated)) == 0;
    if (settings_.hornEnabled && trusted) m |= held_ & kInputHorn;
    if (settings_.cruiseEnabled) m |= held_ & kInputCruiseMask;
    if (settings_.mediaEnabled) m |= held_ & kInputMediaMask;
    return m;
}

void Controller::apply() {
    const uint16_t m = wheelMask();
    const bool horn = (m & kInputHorn) != 0 || pulse_[0].active;
    const uint16_t cruise = uint16_t((m & kInputCruiseMask) | (pulse_[1].active ? pulse_[1].mask : 0));
    const uint16_t media = uint16_t((m & kInputMediaMask) | (pulse_[2].active ? pulse_[2].mask : 0));
    if (horn != state_.hornOn) {
        state_.hornOn = horn;
        if (outputs_.horn) outputs_.horn(horn, outputs_.ctx);
    }
    if (cruise != state_.cruiseOn) {
        state_.cruiseOn = cruise;
        if (outputs_.cruise) outputs_.cruise(cruise, outputs_.ctx);
    }
    if (media != state_.mediaOn) {
        state_.mediaOn = media;
        if (outputs_.media) outputs_.media(media, outputs_.ctx);
    }
}

void Controller::onInput(uint32_t node, const Input& in, uint32_t nowMs) {
    if (state_.seen && node != state_.node) return;  // one wheel; a second is ignored
    if (state_.seen) {
        // A restart of the wheel starts its count again; that is not a gap.
        const uint8_t gap = uint8_t(in.seq - lastSeq_ - 1);
        if (gap != 0 && gap < 128) state_.seqGaps += gap;
    } else {
        state_.seen = true;
        state_.node = node;
    }
    lastSeq_ = in.seq;
    state_.lastInputMs = nowMs;
    state_.linkLost = false;
    state_.flags = in.flags;
    state_.lines = in.lines;
    const uint16_t changed = uint16_t(state_.mask ^ in.mask);
    for (int b = 0; b < 16; b++) {
        if (changed & (1u << b)) state_.changedMs[b] = nowMs;
    }
    state_.mask = in.mask;
    held_ = in.mask;
    apply();
}

void Controller::onRaw(uint32_t node, const InputRaw& raw, uint32_t nowMs) {
    if (state_.seen && node != state_.node) return;
    if (!state_.seen) {
        state_.seen = true;
        state_.node = node;
    }
    state_.lastInputMs = nowMs;
    state_.linkLost = false;
    for (int i = 0; i < 3; i++) {
        const size_t line = size_t(raw.first) + size_t(i);
        if (line < kLines) state_.rawMv[line] = raw.mv[i];
    }
    state_.rawMs = nowMs;
}

void Controller::poll(uint32_t nowMs) {
    bool changed = false;
    for (Pulse& p : pulse_) {
        if (p.active && int32_t(nowMs - p.untilMs) >= 0) {
            p.active = false;
            changed = true;
        }
    }
    if (state_.seen) {
        const uint32_t silent = nowMs - state_.lastInputMs;
        if (held_ != 0 && silent >= settings_.failSafeMs) {
            // The wheel stopped talking while holding something: let go,
            // and count it only if an output was actually on.
            if (wheelMask() != 0) state_.failSafeReleases++;
            held_ = 0;
            state_.mask = 0;
            changed = true;
        }
        if (!state_.linkLost && silent >= settings_.linkLostMs) state_.linkLost = true;
    }
    if (changed) apply();
}

bool Controller::test(Output which, uint16_t mask, uint32_t ms, uint32_t nowMs) {
    Pulse& p = pulse_[uint8_t(which)];
    if (p.active || ms == 0) return false;
    const uint16_t held = wheelMask();
    switch (which) {
        case Output::Horn:
            if (held & kInputHorn) return false;
            mask = kInputHorn;
            break;
        case Output::Cruise:
            mask &= kInputCruiseMask;
            if (!mask || (held & mask)) return false;
            break;
        case Output::Media:
            mask &= kInputMediaMask;
            if (!mask || (held & mask)) return false;
            break;
    }
    p.active = true;
    p.mask = mask;
    p.untilMs = nowMs + ms;
    apply();
    return true;
}

void Controller::reset(uint32_t nowMs) {
    (void)nowMs;
    held_ = 0;
    for (Pulse& p : pulse_) p = Pulse();
    apply();  // the outputs are told off before the record is wiped
    const uint32_t releases = state_.failSafeReleases;
    state_ = State();
    state_.failSafeReleases = releases;
    lastSeq_ = 0;
}

}  // namespace wheel
}  // namespace hg
