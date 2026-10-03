# Gauge pull-down menu (planned on the gauge; hub side built)

Swipe down from the top edge of any gauge and a small panel slides over the face:

```
   +------------------------------+
   |  Brightness  [=======----]   |
   |  Screen      [ off ]         |
   |  Apply to    ( this ) ( all )|
   +------------------------------+
```

- **Brightness**: a slider, live as it moves. The gauge keeps its own brightness in
  flash, so it survives a restart.
- **Screen off**: the panel goes dark and the gauge stops rendering. Any touch, or the
  hub, brings it back. Not remembered across a restart: at key-on the screen comes on.
- **Apply to**: *this gauge* changes only the one you are touching. *All gauges* sends
  the change to the hub, which applies it to every gauge on the bus and remembers the
  brightness, so a gauge that reconnects gets it too.
- The panel closes with a swipe up, a tap outside it, or by itself after five seconds.
- Night dimming from the headlight signal still applies on top of the chosen brightness.

## What is already in place

- Protocol: `Cmd::SetDisplay` (0 off, 1 on) alongside `SetBrightness`, and a gauge-to-hub
  **Request** message, `{cmd, arg, all}`, sent with `GaugeNode::request()`. The hub
  answers "all" requests by commanding every online gauge and remembering brightness.
- Hub: `display_control` keeps the all-gauges brightness in preferences and the
  screens-off state in RAM, re-applies both to any gauge that comes online, and
  exposes them on the page and the API: `POST /api/gauges/all/brightness {level}`,
  `POST /api/gauges/all/display {on}`, `POST /api/gauges/{node}/display {on}`, with
  the state in `/api/status` as `displays`.
- Page: an **All gauges** strip on the Gauges tab with a brightness slider and screens
  off/on, and screen off/on per card.

## What the gauge still needs

The panel itself, in the gauge's screen stack (main plan phase 4): the swipe gesture,
the slider, `SetDisplay` handling (panel sleep and wake, with touch waking it), the
request message for *all*, and brightness stored in NVS.
