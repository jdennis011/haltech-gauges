#pragma once

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

#include "proto.h"

// The cruise buttons as a Ford FG Falcon CAN frame, for a Haltech Nexus
// running the FG Falcon vehicle integration on its second CAN bus
// (docs/steering-wheel.md). Nobody has published the frame the FG's
// instrument cluster sends the PCM for its cruise switches, so the frame is
// a table, not code: an id, the bytes with nothing pressed, and which byte
// and bit each button sets. With no id nothing is ever sent.
namespace hg {
namespace wheel {

constexpr size_t kFgButtons = 6;  // on/off, set, resume, accel, decel, cancel: the kInputCruise* bit order
extern const char* const kFgButtonNames[kFgButtons];

struct FgCruiseFrame {
    uint32_t id = 0;  // 0: not known yet
    bool extended = false;
    uint8_t len = 8;
    uint8_t idle[8] = {};  // the frame with no button down
    uint8_t rateHz = 20;   // repeated this often while enabled
    struct Bit {
        uint8_t byte = 0;
        uint8_t mask = 0;  // 0: this button is not carried
    };
    Bit button[kFgButtons];
    int8_t counterByte = -1;  // a rolling counter's byte, or -1 for none
    uint8_t counterMask = 0x0F;
};

// The table makes sense: an id, a length of 1 to 8, every used byte inside it.
bool fgValid(const FgCruiseFrame& table);
// Fills `out` for the cruise buttons held (the kInputCruise* bits of `mask`)
// and the rolling counter. False when the table is not valid.
bool fgEncode(const FgCruiseFrame& table, uint16_t cruiseMask, uint8_t counter, CanFrame& out);

// JSON form, for the hub's preferences and the page:
// {"id":"0x437","ext":false,"len":8,"rate":20,"idle":"00 00 00 00 00 00 00 00",
//  "buttons":{"set":{"byte":2,"mask":"0x04"},...},"counter":{"byte":7,"mask":"0x0F"}}
// Numbers may be plain or "0x" hex strings. Missing parts keep their defaults.
bool fgFromJson(JsonVariantConst in, FgCruiseFrame& out, std::string& error);
void fgToJson(const FgCruiseFrame& table, JsonObject out);

}  // namespace wheel
}  // namespace hg
