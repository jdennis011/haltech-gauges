# Start-up animation (planned)

At key-on the gauges stay dark, then an intro runs across them in physical order, left
to right, fast: one to three seconds in all. Then live data.

## Order

CAN is a bus, so the hub cannot tell where a gauge sits in the chain. The order is set
once by the user: the Gauges tab gets drag-to-reorder, the order is stored with the
planned gauges on the hub, and Identify confirms which screen is which. A gauge the hub
has not been told about joins the end in the order it first announced itself.

The hub can offer a first guess: each gauge reports its supply voltage in STATUS, and
the chain loses some tens of millivolts per hop, so sorting by voltage usually matches
the physical order. Offered as "guess the order", confirmed by the user, never assumed.

## Gauge side

- Bring the panel up with a black frame and brightness at zero; build the face in
  memory; show nothing.
- Wait for the hub's intro message, at most about 2 s after the first beacon. With no
  hub at all (bare Haltech bus, bench), play a solo intro about 3 s after boot and carry
  on.
- On the message: wait the given delay, run the chosen style for the given duration,
  then fade brightness up to the set level and show live data.
- Styles are drawn from the face itself, so each gauge animates what it shows: needles
  sweep to full scale and back, rings and bars fill and empty, digits count up, the rim
  does a chase. Named styles to choose from: `sweep`, `chase`, `fade`.

## Hub side

- The gauges share the hub's 5 V rail, so they all boot together. The hub waits until
  every gauge it expects (planned gauges linked to a MAC, plus the last roster it saved)
  has announced itself, or a short timeout after the first HELLO, then triggers.
- It sends each gauge one small intro message: slot index, slot count, stagger,
  duration, style. CAN delivers it in well under a millisecond, so the gauges stay
  aligned without a shared clock. A gauge that announces later plays its intro alone.
- Settings, kept in the hub's preferences and shown in the page's Settings tab: on or
  off at boot, style, stagger (default 250 ms), duration per gauge (default 1200 ms),
  and a Play button to run it on demand.

## Timeline

Three gauges, 250 ms stagger, 1.2 s each: 1.7 s end to end.

| Time | Left | Middle | Right |
|---|---|---|---|
| 0 ms | rim chase, needle sweeps up | dark | dark |
| 250 ms | needle sweeps back, ring fills | starts | dark |
| 500 ms | face fades in, live data | needle back | starts |
| 1.7 s | | | live data |

## What it needs

- Protocol: one new message, `Intro`, hub to gauge, fitting one CAN frame (style, delay
  ms, duration ms, index, count). Tests in `test/test_native_proto`.
- Hub: the settle-and-trigger step in the manager, the saved roster, the settings,
  `POST /api/intro` to play now, and an `order` field on planned gauges with
  `PUT /api/virtual/{id}`.
- Page: drag-to-reorder on the Gauges tab, "guess the order", the Settings card, Play.
- Gauge: the blank boot, the wait, the animation styles. This belongs with the face
  renderer work (main plan phase 4) and the management client (phase 6).

Nothing here changes the hardware.
