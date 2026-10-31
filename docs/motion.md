# G meter and drag timer

The Waveshare 1.75 gauge board carries a QMI8658 accelerometer, and the ECU
broadcasts the vehicle speed twenty times a second. Together they give two
widgets that need nothing new on the car: a **G meter** (`gmeter`) drawn from
the gauge's own sensor, and a **drag timer** (`dragtimer`) that times
standing-start and rolling runs from the speed, with the gauge's accelerometer
bringing the start forward. Finished runs go to the hub, which lists them on
the Gauges tab and appends them to `runs.csv` on the SD card.

## The G meter

```
{ "type": "gmeter", "r": 180, "range": 1.5 }
```

A dot on crosshair rings: lateral across (right is a right turn, the car
accelerating to the right), longitudinal up (up is accelerating, down is
braking). Rings every half g out to `range` (0.5 to 4 g, default 1.5), a fading
trail of the last two seconds, the furthest point each way kept as a mark
until **Reset peaks**, and the two values as text in the lower half, in g to two
decimals. Options: `r` (180), `range`, `trail`, `peaks`, `numbers` (all true),
`color` (the numbers), `dotColor` (`accent`), `trackColor` (the rings, `dim`),
`peakColor` (`warn`), `font`, `size` (22). No channel.

With no live data ever received the dot plays a figure of eight, as the page's
preview does; in the key-on intro it sweeps to the right and back. A board with
no accelerometer (the M5Stack Dial) shows the dot at rest.

### Mounting calibration

The sensor sits however the gauge sits: tilted back in a pod, turned on a
pillar. Two captures from the pull-down menu tell the gauge which way is up
and which way is forward, and are kept in its flash:

1. **Set level** with the car at rest on level ground. The resting reading is
   gravity, so it gives up. Refused ("NOT LEVEL") when the reading is not about
   1 g, i.e. the car was moving.
2. **Set forward** while braking firmly on a straight road, with the car level
   (not just after Set level: any time later). The horizontal part of the
   reading points backward, so it gives forward. Refused ("BRAKE HARDER") when
   that part is under 0.15 g.

Until Set forward, forward is the board's own guess: the display faces the
driver, so its normal points backward. The console's `imu` command shows the
raw axes and the resolved g; `imu level`, `imu forward` and `imu clear` are the
same captures from the bench. The maths is `lib/hg_motion/motion.h`
(`setLevel`, `setForward`, `resolve`), host-tested with tilted mounts.

The sensor is read every 20 ms from the UI loop, which owns the I2C bus with
the touch controller and the PMIC, so no lock is needed; the display values
are low-passed at 5 Hz, the launch detector uses the raw longitudinal sample.

## The drag timer

```
{ "type": "dragtimer", "run": "strip", "rollout": true, "size": 64 }
```

One run type per widget (`run`):

| `run` | What is timed |
|---|---|
| `0-100` | Standing start to 100 km/h |
| `strip` | Standing start: 60 ft, 330 ft, 1/8 mile and 1/4 mile, with the speed at the quarter (the trap) |
| `80-120` | Rolling: armed below 78 km/h, timed from 80 to 120 |
| `100-200` | Rolling: armed below 98 km/h, timed from 100 to 200 |

The widget shows the run and its state above (STOP TO ARM, READY, RUNNING,
DONE, ABORTED, or LAST while the previous result is shown), the time large, the
strip's splits under it as they land, and the best complete run of the
session. Options: `rollout` (false), `color`, `doneColor` (the time of a
complete run, green), `dimColor` (the small text, `dim`), `font`, `size` (64).
The parser binds the widget to `vehicle_speed` itself, so the hub keeps
forwarding frame 0x370 (`docs/frame-filter.md`).

How a run goes:

- **Arming.** A standing start is armed after a second below 0.5 km/h; a
  rolling run after a second below its first speed less 2 km/h.
- **Start.** A standing start begins on the first of two signs: a speed sample
  above 0.5 km/h, or 0.2 g forward from the accelerometer, which comes tens
  of milliseconds before a wheel speed sensor reads anything. Without the
  accelerometer the start is placed where the slope of the first two moving
  samples says the speed left zero. A bump that moves the sensor but not the
  car is forgotten after 1.5 s. A rolling run starts as the speed crosses its
  first speed, interpolated between the two samples around it.
- **Rollout.** With `rollout`, a standing run's clock starts after a foot of
  travel (305 mm), as a strip's beams do; the state says ROLLOUT until then.
- **Splits.** Distance is the speed integrated between samples. A distance
  split is placed where the speed, taken as changing evenly across the 50 ms
  between the two samples around it, says it was reached; a speed split where
  the speed crossed it. Each split records the speed as it was reached, which
  is the strip's trap for the quarter (the instantaneous speed, not a strip's
  66 ft average).
- **Abandoned** when the speed falls 5 km/h below the run's peak (a lift),
  after 30 s, or when no speed sample has come for a second. The splits
  reached stay, marked ABORTED.
- **After.** The result stays on the widget until the car stops and the timer
  arms again; the best complete run is kept in RAM beside it. **Reset peaks**
  (the pull-down, or the page) forgets both, along with the hold peaks and the
  G meter's marks.

The timers run whatever face is shown, one per run type the whole config
uses, so a swipe mid-run loses nothing. The state machine is
`lib/hg_motion/drag_timer.h`, host-tested on made-up traces
(`test/test_native_motion`); the gauge feeds it in `src/gauge/drag_runs.cpp`.

## Runs on the hub

A finished run (complete or abandoned) goes to the hub as `Cmd::RunResult`
frames on the gauge bus, one per split reached (a single frame with count 0
when none was): the run type, a sequence number, which split of how many, the
abandoned flag, the split's time from the start and the speed as it was
reached. The hub puts the frames of a run together (`HubManager`, into a
`RunReport`) and:

- keeps the latest fifty for the **Runs** card on the Gauges tab (hidden until
  there is one), newest first, with the gauge, the run, the time and the
  splits, pushed to the page as it lands; **Clear** forgets the list;
- appends a row to `/logs/runs.csv` on the SD card: time (when the hub has a
  clock), uptime, gauge, MAC, run, abandoned, the splits reached with their
  names, seconds and km/h, and the total.

| Method and path | |
|---|---|
| GET `/api/runs` | `{runs: [{seq, at_s, time, node, mac, run, name, aborted, complete, total_ms, splits: [{name, ms, kmh}]}]}` |
| DELETE `/api/runs` | Forgets the list (the file on the card stays) |

The WebSocket carries the same list as `{"t":"runs","runs":[...]}` whenever it
changes. Runs in demo mode (no live data) are replayed on the widget but never
remembered or sent.

## Not built

- The perspective view of the G meter: the car's footprint as a tilted plane
  with the dot moving on it, as a second style.
- The accelerometer as a channel (a bar of lateral g, an alert on braking g):
  the G meter reads the sensor directly.
- A yaw-rate display from the gyro.
