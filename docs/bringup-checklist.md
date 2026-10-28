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

## What to send back

- Header positions of GPIO16/17/18
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
| 5 Header pins | the stage runs (GPIO16, 17, 18 each driven high in turn, named on screen); **positions 6, 7, 8 not yet mapped**: needs a multimeter from a GND pin |
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
