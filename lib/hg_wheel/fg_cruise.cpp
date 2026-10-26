#include "fg_cruise.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace hg::proto;

namespace hg {
namespace wheel {

const char* const kFgButtonNames[kFgButtons] = {"on_off", "set", "resume", "accel", "decel", "cancel"};

namespace {

// A number, or a string such as "0x437" or "437".
bool number(JsonVariantConst v, uint32_t& out) {
    if (v.is<const char*>()) {
        const char* s = v.as<const char*>();
        if (!s || !*s) return false;
        char* end = nullptr;
        const unsigned long n = strtoul(s, &end, 0);
        if (end == s || *end) return false;
        out = uint32_t(n);
        return true;
    }
    if (v.is<unsigned int>() || v.is<int>()) {
        const long long n = v.as<long long>();
        if (n < 0) return false;
        out = uint32_t(n);
        return true;
    }
    return false;
}

// "00 11 22 ..." into bytes; spaces optional. False on a bad digit or length.
bool hexBytes(const char* s, uint8_t* out, size_t max, size_t& count) {
    count = 0;
    if (!s) return true;
    int have = 0;
    uint8_t acc = 0;
    for (; *s; s++) {
        const char c = *s;
        if (c == ' ' || c == ',' || c == ':') continue;
        int d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else return false;
        acc = uint8_t((acc << 4) | d);
        if (++have == 2) {
            if (count >= max) return false;
            out[count++] = acc;
            have = 0;
            acc = 0;
        }
    }
    return have == 0;
}

int lowestBit(uint8_t mask) {
    for (int i = 0; i < 8; i++) {
        if (mask & (1u << i)) return i;
    }
    return 0;
}

}  // namespace

bool fgValid(const FgCruiseFrame& t) {
    if (t.id == 0 || t.len == 0 || t.len > 8) return false;
    if (!t.extended && t.id > 0x7FF) return false;
    if (t.extended && t.id > 0x1FFFFFFF) return false;
    for (const FgCruiseFrame::Bit& b : t.button) {
        if (b.mask && b.byte >= t.len) return false;
    }
    if (t.counterByte >= 0 && (t.counterByte >= int(t.len) || t.counterMask == 0)) return false;
    return true;
}

bool fgEncode(const FgCruiseFrame& t, uint16_t cruiseMask, uint8_t counter, CanFrame& out) {
    if (!fgValid(t)) return false;
    out = CanFrame();
    out.id = t.id;
    out.extended = t.extended;
    out.len = t.len;
    memcpy(out.data, t.idle, 8);
    for (size_t i = 0; i < kFgButtons; i++) {
        const FgCruiseFrame::Bit& b = t.button[i];
        if (b.mask && (cruiseMask & (kInputCruiseOnOff << i))) out.data[b.byte] |= b.mask;
    }
    if (t.counterByte >= 0) {
        uint8_t& c = out.data[t.counterByte];
        c = uint8_t((c & ~t.counterMask) | ((counter << lowestBit(t.counterMask)) & t.counterMask));
    }
    return true;
}

bool fgFromJson(JsonVariantConst in, FgCruiseFrame& out, std::string& error) {
    FgCruiseFrame t;
    uint32_t n;
    if (!in["id"].isNull()) {
        if (!number(in["id"], n)) {
            error = "'id' is a CAN id, as a number or \"0x...\"";
            return false;
        }
        t.id = n;
    }
    if (!in["ext"].isNull()) t.extended = in["ext"].as<bool>();
    if (!in["len"].isNull()) {
        if (!number(in["len"], n) || n < 1 || n > 8) {
            error = "'len' is 1 to 8";
            return false;
        }
        t.len = uint8_t(n);
    }
    if (!in["rate"].isNull()) {
        if (!number(in["rate"], n) || n < 1 || n > 100) {
            error = "'rate' is 1 to 100 frames a second";
            return false;
        }
        t.rateHz = uint8_t(n);
    }
    if (!in["idle"].isNull()) {
        size_t count = 0;
        if (!in["idle"].is<const char*>() || !hexBytes(in["idle"].as<const char*>(), t.idle, 8, count)) {
            error = "'idle' is up to eight bytes in hex, such as \"00 00 10 00 00 00 00 00\"";
            return false;
        }
    }
    JsonObjectConst buttons = in["buttons"].as<JsonObjectConst>();
    for (size_t i = 0; i < kFgButtons; i++) {
        JsonVariantConst b = buttons[kFgButtonNames[i]];
        if (b.isNull()) continue;
        uint32_t byte = 0, mask = 0;
        if (!number(b["byte"], byte) || byte > 7 || !number(b["mask"], mask) || mask > 255) {
            error = std::string("'buttons.") + kFgButtonNames[i] + "' needs 'byte' 0 to 7 and 'mask' 0 to 255";
            return false;
        }
        t.button[i].byte = uint8_t(byte);
        t.button[i].mask = uint8_t(mask);
    }
    JsonVariantConst counter = in["counter"];
    if (!counter.isNull()) {
        uint32_t byte = 0, mask = 0x0F;
        if (counter["byte"].isNull()) {
            t.counterByte = -1;
        } else {
            if (!number(counter["byte"], byte) || byte > 7) {
                error = "'counter.byte' is 0 to 7";
                return false;
            }
            if (!counter["mask"].isNull() && (!number(counter["mask"], mask) || mask == 0 || mask > 255)) {
                error = "'counter.mask' is 1 to 255";
                return false;
            }
            t.counterByte = int8_t(byte);
            t.counterMask = uint8_t(mask);
        }
    }
    if (t.id != 0 && !fgValid(t)) {
        error = "a button or the counter sits past the frame's length, or the id is out of range";
        return false;
    }
    out = t;
    return true;
}

void fgToJson(const FgCruiseFrame& t, JsonObject out) {
    char buf[32];
    snprintf(buf, sizeof(buf), "0x%lX", (unsigned long)t.id);
    out["id"] = buf;
    out["ext"] = t.extended;
    out["len"] = t.len;
    out["rate"] = t.rateHz;
    char idle[8 * 3];
    size_t at = 0;
    for (int i = 0; i < 8; i++) at += size_t(snprintf(idle + at, sizeof(idle) - at, i ? " %02X" : "%02X", t.idle[i]));
    out["idle"] = idle;
    JsonObject buttons = out["buttons"].to<JsonObject>();
    for (size_t i = 0; i < kFgButtons; i++) {
        JsonObject b = buttons[kFgButtonNames[i]].to<JsonObject>();
        b["byte"] = t.button[i].byte;
        snprintf(buf, sizeof(buf), "0x%02X", t.button[i].mask);
        b["mask"] = buf;
    }
    JsonObject counter = out["counter"].to<JsonObject>();
    if (t.counterByte >= 0) {
        counter["byte"] = t.counterByte;
        snprintf(buf, sizeof(buf), "0x%02X", t.counterMask);
        counter["mask"] = buf;
    }
}

}  // namespace wheel
}  // namespace hg
