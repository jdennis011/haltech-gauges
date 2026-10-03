#pragma once

#include <stddef.h>
#include <stdint.h>

namespace hg {

// Standard CRC-32 (zlib / IEEE 802.3). Pass the previous result as `crc` to
// continue over several buffers.
uint32_t crc32(const uint8_t* data, size_t size, uint32_t crc = 0);

}  // namespace hg
