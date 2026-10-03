#pragma once

#include <stdint.h>

namespace hg {

// Driver-independent CAN frame. ESP32-TWAI-CAN's CanFrame is twai_message_t on
// the S3 and a library-defined struct on the C6, so shared code uses this.
struct CanFrame {
    uint32_t id = 0;
    bool extended = false;
    uint8_t len = 0;
    uint8_t data[8] = {};
};

}  // namespace hg
