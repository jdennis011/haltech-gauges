# Steering wheel module

A new steering wheel has more buttons than the car's clock spring has wires. A small
module inside the wheel reads the buttons and joins the gauge chain through the clock
spring's four conductors; the hub turns the reports into the horn, the cruise buttons for
the Nexus, and (later) the head unit's volume. Built 2026-10-26; the phase row is 6j in
the plan.

## How it hangs together

```
 wheel buttons --ladder lines--> [wheel module: ESP32-C3 + CAN]
                                           | 5 V, GND, CAN H, CAN L  (the clock spring)
 last gauge adapter's spare port <---------+
                                           |
 hub: Input frames -> horn relay (GPIO 49) / FG Falcon frames on the vehicle CAN (3rd port)
                      / wired-remote outputs (GPIO 2-5)
```

- The module is a node on the gauge bus like a gauge, with `Info.hwRevision` 3 so the hub
  knows it is a wheel: no config, no intro slot, no alerts, no brightness, not counted as a
  gauge for live data. It still gets firmware updates over Wi-Fi through the hub
  (`docs/firmware-updates.md`, build `wheel-c3`).
- The clock spring carries the chain's own pin order: 1 = 5 V, 2 = GND, 3 = CAN H, 4 = CAN L.
  The module ends the bus with its own 120 R, so it takes the place of the terminator plug
  on the last adapter (with rev B adapters the plug into the last adapter carries the pin
  5-6 loop so that adapter's termination switches out).

## The buttons

Most wheels put several buttons on one wire as a resistor ladder: each button pulls the
line to a voltage of its own. The module reads up to three such lines (four with
`HG_WHEEL_FOUR_LINES`) by ADC, a thousand times a second, with a median filter and a 20 ms
debounce, and matches each reading against a calibration table: the voltage of each
button on each line, with a tolerance (80 mV by default). A line that reads where no button
and no idle sits for half a second is a fault (a short or a broken wire), reported to the
hub, which then keeps the horn off.

Button names, which are also the bits of the report: `horn`, `cruise_on_off`, `cruise_set`,
`cruise_resume`, `cruise_accel`, `cruise_decel`, `cruise_cancel`, `vol_up`, `vol_down`,
`mode`, `mute`, `seek_up`, `seek_down`, `phone`.

### Calibrating

From the module's USB console (`pio device monitor` or any terminal at 115200):

```
raw                      millivolts per line, ten times a second; any key stops
cal 0 vol_up             the current reading of line 0 is vol_up (hold the button)
cal 0 vol_down
cal tol 80               tolerance around every centre
cal show                 the table as JSON
cal save                 keep it in the module's preferences
cal clear
info
```

From the page, with the module on the chain: the Wheel card's Calibration section starts a
raw stream (the module sends its readings ten times a second for the time asked), shows a
bar per line, and Capture records the current reading against a button; Save stores the
table on the hub and pushes it to the wheel. The hub's copy is the master: a wheel whose
table differs (its status carries the table's CRC) is sent the hub's. So calibrate on the
console before the hub has a table, or on the page; a console edit after that is
overwritten by the hub's copy.

Holding BOOT for three seconds while powering the module up clears its calibration.

## The report

`Msg::Input` (type 15), wheel to hub, eight bytes: the op and layout in byte 0, a sequence
number, the button mask (a level: a bit is set while that button is down), flags
(heartbeat, fault, uncalibrated, raw streaming) and the line count. Sent at once on a
change, every 20 ms while any button is down, and every 200 ms as a heartbeat when none is,
so the hub can tell silence from idleness. Under one percent of the bus. Raw readings go
as a second op, three lines' millivolts a frame, only while the hub asks
(`Cmd::InputRaw`, seconds). The calibration reaches the wheel as a transfer of kind
`WheelCalibration`, JSON, like a config.

## On the hub

`lib/hg_wheel/wheel_controller` holds the rule: an output follows the wheel's report only
while the reports keep coming. **Any output held with nothing heard from the wheel for
100 ms is released**, whatever the cause: a pulled plug, a reset, a dead bus. Each output
is off until enabled on the page, and the horn follows the report only from a calibrated
wheel with no line fault.

- **Horn**: `PIN_HORN` (GPIO 49 on the P4 carrier, pulled down on the board, so reset and
  brown-out leave it off) drives an opto-isolated relay module whose contact grounds the
  column's horn wire. The hub's gauge-bus task polls the fail-safe every millisecond.
- **Cruise**: the six cruise bits go out on the vehicle CAN as a Ford FG Falcon frame, for
  the Nexus Rebel's CAN 2 set to Haltech's "FG Falcon" vehicle integration (whose feature
  list has "Cruise Control: CAN Input"). The frame is a table, not code (see below).
- **Volume**: a provision. Four outputs (GPIO 2 to 5) each meant to switch a fitted
  resistor onto the head unit's wired-remote line while a media button is held; which
  button drives which output is a setting; the resistor values wait for the head unit.
- **Test**: the page pulses an output for up to two seconds (the horn behind a confirm);
  refused while the wheel holds that output.

## The FG cruise frame

Nobody has published the frame an FG Falcon's instrument cluster sends the PCM for the
cruise switches (the switches are a resistor ladder into the cluster: 2.7 k reference,
COAST 820 R, SET+ 390 R, SET- 270 R, ON/OFF a short, cluster C-171 pins 5 and 12). The
PCM's own cruise outputs are known from jakka351's FG_Falcon_HighSpeed_CAN.dbc: 0x12D
byte 6 mask 0x38 Cruise_Status, 0x425 byte 0 bits 0 and 1 Cruise_Enable and Cruise_Set
with the set speed in bytes 1-2, 0x427 byte 5 bit 6 Cruise_Set_Ind, all HS-CAN at
500 kbit/s. The input frame is therefore a table on the Wheel card:

```
id, extended, length, rate (frames a second while enabled), idle bytes (nothing pressed),
per button: byte and mask   (on_off, set, resume, accel, decel, cancel; mask 0 = not carried),
counter: byte and mask      (a rolling counter, if the frame has one)
```

With no id nothing is sent. Finding the frame:

1. The jakka351 repository's sheets and DBCs, grepped for cruise, SWC and cluster frames;
   0x437 (cluster, bytes 2-7 undecoded) is the first suspect.
2. Haltech support: which id and bits the FG integration reads for the cruise switches,
   at what speed, from which module.
3. **The Rebel as the oracle**: the Rebel's CAN 2 in FG mode on the hub's vehicle CAN port,
   the Haltech bus on the ECU port as usual. **Probe** on the card walks a candidate id
   bit by bit (an idle frame with one bit set, held 250 ms) while the hub watches the
   Haltech's own `cruise_state` and `cruise_input_state` channels (frame 0x472), and lists
   which bit moved which. **Listen only** first shows what the Rebel itself sends on that
   bus and confirms the speed.
4. A real FG at its OBD port (HS-CAN pins 6 and 14 at 500 k, MS-CAN 3 and 11 at 125 k)
   while pressing the buttons, if one is ever reachable.

### Without the third port

The `hub_p4_fgtest` build runs the vehicle CAN on the gauge port's pins (GPIO32/33) and
transceiver instead of GPIO26/27, and does not start the gauge CAN bus, so a hub with only
its two transceivers can do the listen, probe and send tests above; the gauges link over
Wi-Fi meanwhile. The status card's board name ends in "vehicle CAN test build", and the
Wheel card is always shown. Wiring: the gauge port's transceiver to the Rebel's CAN 2 (a
two-node bus, 120 R at each end, so a breakout's own resistor is right), and the ECU port
to the Haltech bus as usual if the probe is to watch `cruise_input_state` itself (NSP
shows it too). The silent pin stays GPIO52; Listen only needs it wired to the transceiver's
RS pin, otherwise it is a software setting only.

```
pio run -e hub_p4_fgtest
curl -X PUT -H "Content-Type: application/octet-stream" --data-binary @.pio/build/hub_p4_fgtest/firmware.bin http://<hub>/api/update
```

Back to the normal build the same way with `.pio/build/hub_p4/firmware.bin`, or from the
site once a newer release exists.

Fallback if no frame turns up: the Nexus's own switch inputs (confirm in NSP whether Cruise
Control takes a ladder on an AVI or momentary DPIs with the integration on), driven by the
wired-remote outputs' open-drain switches.

## Page and API

The **Wheel** card on the Gauges tab appears once a wheel has been seen or settings exist:
link state and firmware (with Update firmware), the buttons as pills lit while held with
when each last changed, a fault banner, the Outputs toggles, Tests, Calibration and the FG
bus table with its capture log and probe.

| Method and path | |
|---|---|
| GET `/api/wheel` | The whole state: `present, online, node, mac, fw, link_lost, fail_safe_releases, seq_gaps, mask, flags, fault, buttons[], raw[], outputs{horn,cruise,media}, settings{horn,cruise,media}, calibration{stored,crc,in_sync,pending,json}, fg{...}, swc{...}` |
| PUT `/api/wheel/settings` | `{horn, cruise, media}` toggles; `fg: {enabled, kbps, send_idle, table}`; `swc: {enabled, map}` |
| PUT `/api/wheel/calibration` (body: the table JSON) | Stored on the hub and pushed to the wheel |
| POST `/api/wheel/test` `{"output":"horn","ms":200}` | A timed pulse; `output` is a button name |
| POST `/api/wheel/raw` `{"seconds":30}` | The wheel streams raw readings (0 stops) |
| POST `/api/wheel/fg` `{"listen":true}` or `{"probe":"0x437"}` | Listen only, or probe an id (0 stops) |

The WebSocket carries `{"t":"wheel", ...}` within 100 ms of a button and every two seconds.

## Building and flashing

`pio run -e wheel -t upload` over the C3 Mini's USB for the first firmware. After that the
release script publishes `wheel-c3` (`scripts\release_fw.ps1 -Envs wheel`, version from
`src/wheel/version.h`) and the hub's Fetch and the card's Update button deliver it over
Wi-Fi like a gauge's. Host tests: `test_native_wheel` (the controller and the FG table) and
the wheel tests in `test_native_system` (a simulated wheel on the bus: joins without being
a gauge, buttons drive the outputs, the horn releases when the wheel goes quiet, raw
streaming, calibration push, a lossy bus).

Hardware: `docs/electrical-spec.md` section 6.
