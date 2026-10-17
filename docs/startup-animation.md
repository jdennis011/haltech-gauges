# Start-up intro

At key-on the gauges stay dark, then an intro runs across them in physical order, left
to right, fast: one to three seconds in all. Then live data.

## Order

CAN is a bus, so the hub cannot tell where a gauge sits in the chain. The order is set
once by the user on the **Gauges** tab: the planned gauges' order is the display order,
and each card has arrows to move it left or right. A real gauge takes its place by being
linked to a planned one; one the hub has not been told about joins the end, in the order
it first announced itself. Identify confirms which screen is which.

**Guess the order** on the Gauges tab offers a first guess: each gauge reports its supply
voltage, and the chain loses some tens of millivolts per hop, so sorting by voltage
usually matches the physical order. It is offered, never assumed: press it, check with
Identify, fix with the arrows.

## Gauge side

- The panel comes up with brightness at zero; the face is built in memory and shown to
  nobody.
- The gauge waits for the hub's intro frame: up to 2 s after the first beacon it hears.
  With no beacon at all (bare Haltech bus, bench on USB) it plays its own intro 3 s
  after boot and carries on. Brightness and screen commands that arrive before the
  intro are remembered and applied after it.
- On the frame: wait the given delay, run the style for the given duration while the
  brightness comes up, then live data at the set brightness.
- Styles are drawn from the face itself, so each gauge animates what it shows: every
  widget with a channel gets a made-up reading that goes to full scale and back,
  needles sweep, rings and bars fill and empty, digits count up, lights come on. The
  renderer's demo sweep uses the same code with time as the driver.
  - `sweep`: everything together.
  - `chase`: each widget starts a little after the one before it, in the config's order.
  - `fade`: no sweep; the face fades up from black over the duration, live data showing.
- A gauge plays once per start. A second intro frame is ignored unless its force flag is
  set, which is what the page's **Play now** sends.

## Hub side

- The gauges share the hub's 5 V rail, so they all boot together. The hub waits until
  every gauge it expects has said hello, then sends each one its intro frame. Expected
  means the planned gauges linked to hardware, or the number of gauges online last time,
  whichever is more; with some but not all of them heard, it sends 1.5 s after the first
  hello; with none at all it stops waiting after 8 s, and late arrivals play alone.
- Each frame carries the slot index, slot count, delay (index × stagger), duration and
  style. CAN delivers them in well under a millisecond, so the gauges stay aligned
  without a shared clock.
- The hub restarting while the gauges are running (a firmware update, say) sends the
  frames again, and the gauges ignore them because they have played since they started.
- Settings, kept in the hub's preferences and shown in the page's Settings tab, **Start-up
  intro**: on or off at key-on, style, stagger (default 250 ms), duration per gauge
  (default 1200 ms), and **Play now**. **Show start-up** in the All gauges row of the
  Gauges tab does the same.

The sequencing is `lib/hg_intro` (host-tested with simulated gauges); the hub's settings,
saved count and order are `src/hub/intro.cpp`; the gauge's wait and styles are in
`src/gauge/face_ui.cpp` and `face_renderer.cpp`, written with the renderer and not yet
seen on a panel.

## Timeline

Three gauges, 250 ms stagger, 1.2 s each: 1.7 s end to end.

| Time | Left | Middle | Right |
|---|---|---|---|
| 0 ms | needle sweeps up, brightness coming up | dark | dark |
| 250 ms | needle sweeps back | starts | dark |
| 500 ms | live data | needle back | starts |
| 1.7 s | | | live data |

## On the gauge bus

No message type is spent. The intro is a `Command` frame (type 4, hub to gauge) whose
command byte is `Intro` (8), with the rest of the frame filled in:

| Byte | |
|---|---|
| 0 | `Cmd::Intro` (8) |
| 1 | style in the low four bits (0 sweep, 1 chase, 2 fade); bit 4 = force, play even if played since start |
| 2, 3 | delay before starting, ms, big-endian |
| 4, 5 | duration, ms |
| 6 | this gauge's slot, from 0 |
| 7 | number of gauges in the intro |

A gauge acknowledges it like any command. One without the intro handler (older firmware)
answers "not done" and shows live data as before.

## API

| Method and path | |
|---|---|
| GET `/api/intro` | `{enabled, style, stagger, duration, done, expected}`: the settings, whether the key-on intro has gone, and how many gauges the hub waited for |
| PUT `/api/intro` (any of the four settings) | Saves them. 400 with the reason for a bad style or range |
| POST `/api/intro/play` | The intro now on every online gauge, in order: `{ok, gauges}` |
| PUT `/api/virtual/{id}` `{"order": 0}` | Moves a planned gauge to that position in the order |

`GET /api/gauges` lists gauges in display order and gives each its `order` (the intro
slot, from 0, or null when it is not online) and planned gauges their `position`.

Nothing here changes the hardware.
