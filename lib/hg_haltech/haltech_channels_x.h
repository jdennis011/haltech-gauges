// Expands haltech_channels.def through two caller-supplied macros:
//
//   HG_FRAME(can_id, rate_hz)
//   HG_CH(name, label, can_id, byte, bit_hi, width, is_signed, scale, offset, unit, zero_is_error)
//
// Both are undefined again at the end. No include guard: this is included
// once per expansion.

#define HG_U16(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 16, false, s, o, u, false)
#define HG_S16(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 16, true, s, o, u, false)
#define HG_U16E(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 16, false, s, o, u, true)
#define HG_S16E(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 16, true, s, o, u, true)
#define HG_U8(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 8, false, s, o, u, false)
#define HG_S8(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 8, true, s, o, u, false)
#define HG_U32(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 32, false, s, o, u, false)
#define HG_S32(n, l, id, b, s, o, u) HG_CH(n, l, id, b, 7, 32, true, s, o, u, false)
#define HG_BIT(n, l, id, b, bit) HG_CH(n, l, id, b, bit, 1, false, 1.0f, 0.0f, Bool, false)
#define HG_BITS(n, l, id, b, bit, w, u) HG_CH(n, l, id, b, bit, w, false, 1.0f, 0.0f, u, false)

#include "haltech_channels.def"

#undef HG_U16
#undef HG_S16
#undef HG_U16E
#undef HG_S16E
#undef HG_U8
#undef HG_S8
#undef HG_U32
#undef HG_S32
#undef HG_BIT
#undef HG_BITS
#undef HG_CH
#undef HG_FRAME
