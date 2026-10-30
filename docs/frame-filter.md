# The hub forwards only the frames in use

The Haltech broadcast is 62 kinds of frame, about 1400 frames a second. A face uses
a handful of channels, so most of that was crossing the gauge bus (and the Wi-Fi link,
docs/ip-link.md) to be decoded and dropped on every gauge. From gauge 0.1.10 and hub
0.3.20 each gauge tells the hub which frames its faces read, and the hub puts only the
union of those on the gauge bus, whether they come from the ECU or from the simulator.
The hub itself still decodes everything it hears from the ECU: the page, the alert
rules and the datalog are unchanged.

## How

- The frame table in `lib/hg_haltech/haltech_channels.def` is sorted by CAN id;
  `hg::frameIndexOf(id)` is a frame's position in it and `hg::frameTableTag()` an
  8-bit hash of the whole table, so the order is part of the protocol.
- After building a face, the gauge works out the frames of every widget of every face
  of its config (`hg::cfg::framesUsed`, `lib/hg_config/face_needs.cpp`) and sends them as
  `proto::Needs`: a Command frame, gauge to hub, with `Cmd::Needs` (13) in the first
  byte, then the part number, the table tag, and 40 bits, bit *i* of part *p* standing
  for the frame at position *p* x 40 + *i*. Two parts cover 80 frames. It goes when the
  config changes (a push, a try, a face set with different widgets does not change it:
  all faces are counted), when the hub appears, and every 3 s in case a frame was lost.
- The hub keeps each gauge's parts in its roster record (`HubManager::Gauge::needs`,
  forgotten at a hello) and `HubManager::frameFilter(tag)` gives the union over the online
  gauges, or **everything** while any online gauge has not said, or said against a
  different table tag than the hub's (a gauge on older firmware, or one built from a
  different `haltech_channels.def`). Steering wheel modules do not count. The gauge-bus
  task refreshes an atomic copy every loop; the bridge task and the simulator check it
  before each write (`frameWanted()` in `src/hub/main.cpp`).
- `/api/status` reports it under `bus` as `wanted` of `kinds`, and the Settings card's
  gauge-bus line says "9 of 62 frame kinds wanted".

## What it does not do

- It does not close a gap. A needle that falls to its minimum has seen no frame for
  half a second (`ChannelStore::staleAfterMs`); fewer frames on the bus make that less
  likely, not impossible.
- A gauge on a raw Haltech bus (no hub) hears everything as before.
- A config pasted on the console or a built-in face counts like any other: the gauge
  reports whatever it is showing.
