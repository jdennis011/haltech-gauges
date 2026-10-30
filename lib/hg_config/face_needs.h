#pragma once

#include <stddef.h>
#include <stdint.h>

#include "face_model.h"

namespace hg {
namespace cfg {

// Sets, in `parts`, a bit for every Haltech frame a widget of any face of the
// config reads: bit i of part p is the frame at position p * partBits + i of
// the frame table (hg::frameIndexOf). Clears the rest. The gauge sends this to
// the hub as proto::Needs so that only those frames come down the bus.
void framesUsed(const Config& cfg, uint64_t* parts, size_t partCount, unsigned partBits);

}  // namespace cfg
}  // namespace hg
