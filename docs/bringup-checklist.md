# Bring-up checklist

Bench steps that need real hardware, in order. Each one ends with something to
observe and a few things to write down; the results decide details of the
adapter board and the next firmware steps.

## Building and flashing

Open this folder in VS Code and pick the environment in the PlatformIO status
bar, or use the command line:

```powershell
$pio = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"   # the `pio` on PATH is an old 4.3.4
& $pio run -e gauge_bringup -t upload
& $pio device monitor
```

| Environment | Board | Purpose |
|---|---|---|
| `gauge_bringup` | gauge | Step 1: display, touch, PMIC, header pins |
| `gauge` | gauge | Step 2 onward: the gauge firmware |
| `hub_selftest` | ESP32-C6 | Step 3: the hub's two CAN controllers talking to each other |
| `hub` | ESP32-C6 | Step 4 onward: the hub firmware |
| `hub_dev_c3` | Lolin C3 Mini (or SuperMini) | Steps 4 and 4a without a C6: the hub minus the ECU bridge. Transceiver on GPIO4 (TX) and GPIO5 (RX) |
| `gauge_m5dial` | M5Stack Dial | Steps 2 and 4 without a Waveshare board: the gauge firmware at 240 px. Transceiver on Port B |

## Step 4a: hub web UI (no gauge or transceiver needed)

Flash `hub_dev_c3` (or `hub`) and power it from USB.

| Check | Expect |
|---|---|
| Phone Wi-Fi list | a network called `HaltechGauges` |
| Join it (password `haltech123`), open http://192.168.4.1/ | the page loads; the top chips show `gauge bus running` and `0 gauges online` |
| Tap `simulator off` | it turns green; the Live data tab fills with moving values |
| Configs tab, New from template, Save | the config appears in the library; the preview animates with the simulator's data |
| Gauges tab, Add a planned gauge, pick that config in its Config dropdown | a card with a small animated preview of the config's first face; the Face dropdown switches it |
| Power-cycle the hub, reload the page | the planned gauge and its config are still there |
| Settings tab | memory stays roughly level while the page is open |

To let a PC on your home network reach it, enter the network in Settings, or
type `W<network>;<password>` into the serial console. See `docs/web-ui.md`.

## Today's hardware: M5Dial + ESP32-C3

The whole of step 4 runs on boards you already have. The Dial stands in for a gauge and the
C3 Mini for the hub; only the panel-specific checks (step 1) and the ECU bridge (step 3) wait for
the real boards.

| | Wiring |
|---|---|
| Dial, Port B Grove cable | white = GPIO1 to transceiver TX, yellow = GPIO2 to transceiver RX, black = GND |
| Dial transceiver power | The Grove red wire is 5 V and SN65HVD230 breakouts want 3.3 V. Power the transceiver from the C3 DevKit's 3V3 pin (grounds are shared on the bench) or a small 3.3 V regulator; do not feed it the Grove 5 V |
| C3 DevKitM-1 | GPIO4 to transceiver TX, GPIO5 to transceiver RX, 3V3 and GND |
| Bus | H to H, L to L, 120 ohm at both ends (the breakouts' own resistors, if fitted, are those) |
| Power | Both boards from USB. The Dial can also take 6-36 V on its rear terminal |

Flash `gauge_m5dial` to the Dial and `hub_dev_c3` to the C3, then follow step 4 below. On the
Dial the tachometer is drawn at half size and the frame-rate figure is for that panel, not the
AMOLED, so step 2's numbers still need the Waveshare board.

If a gauge board does not enter download mode by itself, hold BOOT while
plugging in USB.

Host tests need no hardware: `.\scripts\test.ps1`

## Bench parts

- 1 x Waveshare ESP32-S3-Touch-AMOLED-1.75 (standard or -B, not the GPS -G version)
- 1 x ESP32-C6-DevKitC-1 (8 MB flash)
- 3 x 3.3 V CAN transceiver breakouts (SN65HVD230 is fine on the bench). Most carry a 120 ohm
  resistor: keep it on the two boards at the ends of a bus, remove it from any in the middle.
- Jumper wires, a multimeter, and a 5 V bench supply with a current readout

## Step 1: bare gauge board (`gauge_bringup`)

USB only, nothing on the header. Press BOOT to move through the stages.

| Check | Expect | Record |
|---|---|---|
| Serial banner | PSRAM about 8 MB, flash 16 MB | |
| I2C scan | 0x18, 0x20, 0x34, 0x40, 0x51, 0x5A, 0x6B | any missing |
| 1 Colour bars | red, green, blue, white, yellow, left to right | wrong order or offset image |
| 2 Brightness ramp | smooth fade down and up | |
| 3 Full white | | supply current (worst case for the power budget) |
| 4 Touch | a dot appears under your finger | mirrored or rotated? |
| 5 Header pins | one GPIO at 3.3 V at a time, named on screen | **which header position is GPIO16, 17, 18** (position 1 = VBUS end) |

Then, for the power design:

1. Unplug and replug USB without touching the PWR button. Does it start by itself?
   The serial log says `Powered on by 5 V being applied` if the PMIC agrees.
2. With USB unplugged, feed 5 V into header position 1 (VBUS) and ground into position 2.
   Does it start by itself?
3. On the bench supply at stage 3 (full white), lower the voltage slowly. Record the voltage
   where the display dims or the board resets. The chain needs to stay above that at the
   last gauge.

## Step 2: display performance (`gauge`)

USB only. A tachometer sweeps 0 to 8000 by itself (it says DEMO).

| Check | Expect | Record |
|---|---|---|
| Frame rate on screen | 30 fps or better | the number |
| Needle edges | clean, no trail of stray pixels | any artefacts |
| Large digits | change smoothly about 20 times a second | stutter |
| Serial heap report | internal heap "min" stays well above 0 | the numbers |

If the digits stutter, the Number widget will use pre-rendered digit fonts instead of TTF.

## Step 3: hub CAN self-test (`hub_selftest`)

Two transceivers on the C6, their bus sides joined H to H and L to L, 120 ohm at each end.

| C6 pin | Goes to |
|---|---|
| GPIO18 | transceiver A TX (CTX / D) |
| GPIO19 | transceiver A RX (CRX / R) |
| GPIO20 | transceiver B TX |
| GPIO21 | transceiver B RX |
| 3V3, GND | both transceivers |

| Check | Expect |
|---|---|
| Serial after 60 s | `PASS (gaps 0)`, about 2000 frames a second each way |
| Short CAN H to CAN L for a few seconds | state changes, `recoveries` counts up, traffic resumes after the short is removed |

## Step 4: hub and gauge together (`hub` + `gauge`)

Third transceiver on the gauge: its TX and RX to two of GPIO16/17/18 (set `PIN_CAN_TX` and
`PIN_CAN_RX` in `src/gauge/board_pins.h` to match step 1), 3V3 and GND from the header. Join
its bus side to the hub's transceiver B. Transceiver A (the ECU side) stays unconnected.

No C6 yet? An ESP32-C3-DevKitM-1 running `hub_dev_c3` does this whole step: one transceiver on
GPIO4 (TX) and GPIO5 (RX), 120 ohm at both ends of the two-node bus. Only the ECU bridge is
missing, which this step does not use.

Hub serial console: `?` lists commands.

| Check | Expect |
|---|---|
| Power both | hub prints the gauge in its roster within 2 s; gauge screen shows `+HUB` |
| `s` on the hub | simulator on; the gauge needle follows an 8 s rev sweep and the label says CAN |
| `i` on the hub | the gauge shows its id for 5 s |
| Unplug the CAN wires | gauge shows `---` and NO DATA within a second; hub lists it offline within 3 s |
| Plug back in | both recover without a reset |

## Step 5: the Haltech, engine off (`hub_p4` on the carrier)

Only after the carrier has run from a bench supply with its sockets empty (the rev A rule in
the results below). The Rebel broadcasts the Haltech CAN protocol on its Haltech CAN port
(CAN 1, the DTM-4 on the main loom, not CAN 2) whenever it is powered, so the key alone is
enough: no cranking, RPM reads 0. The hub's ECU port listens and acknowledges at 1 Mbit/s
and never transmits.

Before plugging in:

| Check | Expect |
|---|---|
| Loom, key on | DTM-4 pin 1 = +12 V, pin 2 = ground (spec section 1.6). CAN H and L cannot be told apart with a meter |
| Key off, meter across CAN H and L at the loom | 60 ohm: terminated at both ends already, JP1 stays open. 120 ohm: the Rebel is the only terminator, fit JP1 |
| The socket | J10, the ECU one. J11 is the chain and has the same shell (spec 1.5) |
| Hub page | simulator off: it would put its own frames on the gauge bus beside the real ones |

Then plug in, key on:

| Check | Expect |
|---|---|
| Status chip `ECU` | green, `ECU N frames`, N climbing by several hundred a second (the table in `lib/hg_haltech/haltech_channels.def` adds up to 1400 a second if the Rebel sent every frame; it sends the ones its setup uses) |
| Settings, ECU bus row | a healthy state, 0 missed, 0 errors, still 0 after ten minutes |
| Live data tab | channels fill in: RPM 0, battery voltage near 12, coolant and air temperatures at room temperature, manifold pressure near 100 kPa |
| A gauge on the chain | shows the live values, label says CAN |
| No frames at all | swap CAN H and L at the hub end and retry (nothing is damaged); then check in NSP that the Haltech CAN protocol is on for that port |
| Errors or missed climbing | termination (the 60 ohm check) or the H/L pair not twisted |

## What to send back

- Header positions of GPIO16/17/18 (answered 2026-10-29: 6 = GPIO16, 7 = GPIO17, nothing on 8)
- Whether the board starts by itself on USB and on header 5 V
- Full-white current, and the lowest voltage it runs at
- Step 2 frame rate and anything that looked wrong
- Step 3 result line
- Anything in step 4 that did not match

## Results

### 2026-10-26: first Waveshare board, step 1

Board on COM5 (Espressif USB, MAC B8:1F:3F:D0:29:F0). The `gauge_bringup` build was
flashed over USB at the first attempt.

| Check | Result |
|---|---|
| Serial banner | ESP32-S3 rev 2, 16 MB flash, 8 MB PSRAM |
| I2C scan | all seven: 0x18 0x20 0x34 0x40 0x51 0x5A 0x6B |
| PMIC | found; "Powered on by 5 V being applied"; VBUS 4.97-5.01 V, system 3.72 V on USB |
| 1 Colour bars | correct order, no offset |
| 2 Brightness ramp | smooth |
| 3 Full white | bright; supply current not measured (no inline meter yet) |
| 4 Touch | CST9217; dot under the finger, two fingers tracked; reported x 49-416, y 10-444 across the panel, so the panel's coordinates are the screen's with no mirroring |
| 5 Header pins | the stage runs (GPIO16, 17, 18 each driven high in turn, named on screen); mapped with a meter on 2026-10-29 while chasing the first chain test: position 6 = GPIO16, 7 = GPIO17, nothing found on 8 |
| Power checks | not done: no bench supply yet (replug without PWR, 5 V on the header, brown-out voltage) |

A plain multimeter is enough for the pin map; only the brown-out and the 5 V-on-header
checks wait for the supply.

### 2026-10-26: first Waveshare board, step 2

The `gauge` build (0.1.1 plus the day's renderer change) over USB: built-in classic tach
in DEMO, the start-up sweep plays on its own at power-on, colours well defined, large
digits clean, no needle trails.

| Build | fps | panel push ms/s | LVGL ms/s |
|---|---|---|---|
| As first flashed: QSPI 40 MHz, 2 KB transfers, whole dial redrawn each frame | 11 | 545 | 990 |
| QSPI 80 MHz, 8 KB transfers (`ESP32QSPI_MAX_PIXELS_AT_ONCE=4096`) | 13 | 480 | 1015 |
| Plus dials and rings invalidating only the swept region | 30 | 190 | 700 |

So the bus clock was not the limit: a full 466 px frame is 434 KB, and both rendering it
and pushing it cost about 40-50 ms. Redrawing only where the needle was and where it goes
meets the 30 fps target. The remaining ~17 ms a frame is LVGL re-issuing every draw call
of the dial (clipped) plus the digits; caching the static scale as an image would cut it
further if needed. Internal heap 58 KB free (min 50 KB) with the LVGL draw buffer
(466 x 116 x 2 = 108 KB) in PSRAM; the two 8 KB QSPI DMA buffers took 16 KB of it.
LittleFS formatted itself on the first boot (the mount errors in that log are expected once).

### 2026-10-26: first Waveshare board on the hub, over Wi-Fi

With no CAN hardware yet, the gauge joined the P4 hub's access point from its serial console
(`link HaltechGauges haltech123`, docs/ip-link.md) and the hub drove it from there.

| Check | Result |
|---|---|
| Roster | node D029F0, hardware 1 (Waveshare 1.75), link "wifi", VBUS 4.95 V, within seconds of the join |
| Config push | the four-face Temps config with its 8.9 KB JPEG: image first, then the config, landed over the link |
| Face switching from the hub | all four faces |
| Reboots | four hub-commanded reboots re-registered in 4 to 8 s |

Found and fixed on the way (all in the gauge firmware):

- **Internal RAM.** With Wi-Fi up (43 KB) the gauge had 15 KB of internal RAM left and
  aborted twice: in `fopen()` while opening the image, then at every boot in Wi-Fi start
  (`phy_track_pll_init`, `ESP_ERR_NO_MEM`), looping until the boot-loop guard showed the
  built-in face. LVGL now takes a 4 MB pool in PSRAM (`LV_MEM_POOL_ALLOC` in `lv_conf.h`)
  instead of the C allocator, and the display's draw buffer is 40 lines (37 KB) rather than
  a quarter screen (108 KB). Internal RAM free went from 15 KB to 148 KB with the link up.
- **Images.** LVGL's JPEG decoder reopens the file and decodes it a block at a time on every
  redraw that touches the image, so the Temps faces (a photo under a dial) swung between 7
  and 28 fps. Images are now decoded once into PSRAM when a face is built (`decodeWhole()`):
  the same faces hold 25 to 29 fps.
- **Silent link.** Once, after a reset, the gauge joined, got its address and sent keepalives
  without error, yet the hub heard nothing until the link was dropped and remade from the
  console; it could not be reproduced with four further reboots. The gauge now remakes the
  link itself after 15 s without a packet from the hub (`kSilentRejoinMs`), and prints a
  link line every 10 s (packets sent, failures, address, RSSI) for the bench.

Not yet checked on this panel: the pull-down menu, swiping between faces, the alert band,
the clock widget, the start-up order with more than one gauge, Wi-Fi image and firmware
delivery (`WifiJoin`), and anything over CAN.

### 2026-10-26: gauge firmware over Wi-Fi

Gauge 0.1.2 released to the site, fetched by the hub, delivered to the Waveshare gauge over
the hub's access point: join frames, download, restart, back online on 0.1.2 in 21 s, and
still there after the 10 s rollback window. First gauge update that never touched USB.
The hub itself restarted on a task watchdog near the end of its own fetch of the image
(reset reason "task watchdog", uptime 70 s when read); the fetch had completed and the
stored build was intact. Being looked at with the hub's serial port.

### 2026-10-26, later: the hub's watchdog resets explained

The P4 hub's "task watchdog" resets (during both gauge-firmware fetches) and an update
that sat at "installing, 100 %" came from one thing: with the simulator on, the gauge
online over Wi-Fi and no CAN transceiver fitted, the gauge controller is bus-off and the
driver logged a refusal for every one of the simulator's 1400 frames a second, which
starved the main loop and the idle task. Hub 0.3.11 hands nothing to a bus-off controller
(the Wi-Fi tap still delivers) and pauses briefly every 32 KB of a download. With the
simulator on and the gauge linked, twelve seconds of serial now show no such line.

### 2026-10-26: second Waveshare gauge

COM6, MAC B8:1F:3F:D0:24:24, node D02424. Flashed gauge 0.1.3 over USB without the
bring-up build (the board type was proven on the first): built-in face at 30 fps, linked to
the hub's access point as 192.168.4.3, took the two-face NFS config in 3 s. The roster lists
both gauges (D029F0 in slot 0, D02424 in slot 1) and Play now sent the intro to both.

### 2026-10-27: carrier rev A first power-up, and the P4 it took

The first P4 carrier (rev A, from the EasyEDA project) was powered from 12 V at J10 with
the P4 fitted. The library footprint used for the Pololu D36V28F5 socket had VIN and VOUT
swapped; the module itself reads EN, VIN, GND, GND, VOUT left to right. Twelve volts went
into the module's output, through the synchronous switch's body diode to its VIN pin, and
from there onto the carrier's 5 V net and the P4's VSYS. The Waveshare board's USB bridge
still enumerates, but the P4 gives no boot log and does not answer esptool.

Replaced by a new board (COM7, MAC e8:f6:0a:e8:a6:24), flashed 0.3.14 by USB. The account
sync brought back the configs, the image, the alerts and the theme colours; planned gauges,
assignments and the hub's own settings have to be redone. The footprint is fixed in EasyEDA
and in `scripts/gen_eagle_sch.py`; `docs/bom-jlcpcb.md` names the pin order.

Rules from this: a carrier is first powered with its sockets empty and must show 5.0 V at
TP2 and at the VSYS pad before any processor board is fitted; a library footprint for a
module is checked against the module in hand before the order; rev B gets an over-voltage
cutoff on the 5 V feed to the board.

### 2026-10-28: carrier rev A running

The Pololu D36V28F5 went in on jumper wires in the module's own pin order (EN, VIN, GND,
GND, VOUT) rather than the swapped footprint. The carrier was run from a bench supply at
J10 with the P4 socket empty first, then the replacement P4 was fitted: the hub runs on the
carrier. Its ECU side has not seen a Haltech yet (step 5).

Seen from the page once it was back on the home network: the carrier's DS3231 is found and
NTP sets it (time card: RTC ok), the account sync runs, the ECU port reads 0 frames with
0 errors (nothing plugged in). With nothing on the chain's wire the gauge-bus error counter
on Settings climbs by about 6000 a second: the beacon is never acknowledged and the
controller keeps retrying it, so the transmit queue sits at 8. The chip stays green, and
gauges linked over Wi-Fi still get every frame (the write path sends over the IP link
before it looks at the CAN queue). It settles as soon as a gauge is on the wire.

### 2026-10-28: first gauge on the chain from the carrier

Carrier on a 12 V bench supply, one rev A adapter on J11 with the terminator plug in its
J2, gauge D02424 on the adapter, no USB on either board. Power with the gauge at full
brightness 1.455 W, screen off 1.395 W: about 0.3 A at the gauge, the figure the 5 V
budget (spec 2.4) assumed. The small difference between on and off is the AMOLED showing
a mostly black face; full white is the worst case and was not measured. The gauge reports
4.625 V at its supply pin, the 5.0 V rail less the adapter's D1.

The CAN link did not come up: the gauge stayed on the Wi-Fi link, the hub's gauge-bus
error counter kept climbing at the same rate as with nothing on the wire and its transmit
queue sat at 62 of 64, so nothing acknowledged the hub. Suspects, in order: the adapter's
header positions 6-8 were never mapped (step 1 left them open; the firmware assumes
position 6 = GPIO17 TX and 7 = GPIO18 RX); the carrier's gauge transceiver U3 on GPIO32/33,
in use for the first time; CAN H/L crossed somewhere in the cable. Resolved by the meter
checks in step 4 above and the header map from `gauge_bringup` stage 5.
Narrowed down the same evening. Meter on the adapter's header, gauge running on the chain:
position 3 = 3.3 V, positions 4 and 5 = 3 V (UART idle), position 6 = 1.4 V average,
position 7 = 3 V. Hub 0.3.17 added the gauge bus's received-frame count and its TEC/REC to
the status and the Settings row: received 0, TEC pinned at 128, REC 0, state running, 62
frames waiting. TEC stuck at exactly 128 is the acknowledge-error signature: the hub puts
whole frames on the bus and reads its own bits back (a bit error would push it past 128 to
bus-off), and no other node ever acknowledges; REC 0 and received 0 say nothing the gauge
sends reaches the hub, not even as a damaged frame. So position 6 is the gauge's CAN TX
retrying for ever (GPIO17, the firmware's assumption holds), and the adapter's transceiver
U1 is neither driving the bus nor echoing it: its R output (position 7) would toggle if it
saw either the hub's frames or the gauge's own. Single faults that fit: U1 without 3.3 V at
the chip, RS not at ground (standby, driver off), or a bad joint on U1. Next: meter U1's
pins directly (SOIC-8: 1 D, 2 GND, 3 VCC, 4 R, 5 Vref, 6 CANL, 7 CANH, 8 RS) and the H/L
resistance with the power off (60 ohm expected: the hub's 120 in parallel with the plug's).

### 2026-10-29: the header map was the fault; CAN link up

Meter on the adapter's transceiver with the gauge on the chain: pin 3 (VCC) 3.3 V, pin 5
(Vref) 1.6 V, pin 8 (RS) 0 V, pin 4 (R) 1.9 to 2.4 V and moving, pin 1 (D) 3.3 V, pins 7
and 6 (CANH, CANL) 2.4 and 1.4 V, continuity from pin 1 to header position 6 and from pin 4
to position 7. So the chip was alive, its receiver was passing the hub's frames to the
header, and the adapter was fine. Then the gauge's socket was traced: position 6 is GPIO16
and position 7 is GPIO17, with nothing found on position 8. Waveshare's hardware reference
has 6 and 7 the other way round and the firmware had followed it (transmit on GPIO17,
receive on GPIO18): the transceiver's D input hung on a pin the firmware never drove, so
the gauge never put a bit on the bus, and the R output was fighting the firmware's transmit
output on GPIO17, which is where the 1.9 to 2.4 V on the receive line came from.

Gauge 0.1.7: `PIN_CAN_TX 16` and `PIN_CAN_RX 17` in `src/gauge/board_pins.h`, plus a `can`
console command (state, frames received, TEC, REC, bus errors, missed, failed to send, hub
heard, link), flashed over USB on COM6 with the adapter's JP1 pulled. Within 30 s of the
hub powering: hub gauge bus TEC 0, REC 0, 30 frames received from the gauge, 0 waiting to
send; gauge `can: running, 51133 frames received, TEC 0, REC 0, hub heard`. The gauge also
joined the hub's access point, so the roster shows its link as `wifi` while it is on both,
and every management frame reaches it twice (harmless so far; a `PlaySound` would play
twice).

Not yet clean: the hub's gauge-bus error counter still climbs in bursts, 323 in one 30 s
window and 2 in another (about 1 % of the 900 simulator frames a second), and the gauge's
counters reset at least once in its first minute, which is its driver restarting after a
bus-off. TEC and REC sit at 0 on both ends between bursts. The gauge was on PC USB power
with JP1 pulled throughout, so the ground path through the PC is the first suspect:
re-measure on chain power only, then with the Wi-Fi link off (`link off` on the console
forgets the stored network; the hub sends it again when it needs a transfer).

Chain power only (USB out, JP1 back in), 60 s watched from the hub with the simulator at
1400 frames a second: 1 bus error, TEC 0, REC 0, ten status frames from the gauge every
10 s, VBUS 4.60 V at the gauge. So the bursts were the bench USB ground path and the chain
is clean; the Wi-Fi-off test was not needed. The 300 000 errors the counter had gathered by
then were the minutes with the gauge unplugged (the hub retries about a thousand times a
second with nobody acknowledging); the counter only clears at a hub restart. Still open:
the roster says `wifi` while the gauge is on both links and management frames arrive
twice; the hub should prefer CAN once it hears a gauge there, or the gauge should drop the
Wi-Fi link while CAN carries the beacon.

Two gauges on the chain the same afternoon (D02424 then D029F0 on the second adapter, the
terminator plug moved to its spare port). D029F0 came up on 0.1.3 with the old pin map and
was on Wi-Fi only, as expected; the user released gauge 0.1.7 and hub 0.3.17 to the site
with `scripts/release_fw.ps1` and installed it from the card's Update button. Then, 30 s
watched from the hub with the simulator at 1400 frames a second: two status frames a
second from the chain (one per gauge), 0 bus errors, TEC 0, REC 0, VBUS 4.60 V at the
first gauge and 4.575 V at the second. The site serves gauge 0.1.7 and hub 0.3.17.

Boot splash, 2026-10-29, both gauges on USB (gauge 0.1.9, with the peak-hold work of
0.1.8): the first flash boot-looped, a LoadProhibited in the idle task on core 0 with the
IDLE0 stack canary tripping while the panic printed. Cause: ThorVG keeps a 16 KB cell
buffer on the stack of the task that draws a filled path (`tvgSwRle.cpp`, `rleRender`),
and the loop task's whole stack was 16 KB, so the buffer landed below the stack, past
the canary, and over the heap (the task watchdog's list, among others). No path widget
had been drawn on a panel before; the splash was the first. `SET_LOOP_TASK_STACK_SIZE`
is now 48 KB; internal heap free at run time went from 139 KB to about 100 KB. Then:
lettering rasterised in 43 ms, entrance from 0.86 s after power, solo intro at 3.1 s,
splash shown 3.2 s. The first hand-over, a cross-fade with the face showing through,
dropped the intro from 25 to 6 frames a second (25 vector draws per strip, each with
a 75 KB temporary ARGB buffer on this RGB565 panel), then 15 with the letters
pre-rendered (two full redraws of the face under the splash); fading the lettering
into the black and letting the brightness ramp bring the face up gives 22, against
25 with no splash at all, on the built-in face.
