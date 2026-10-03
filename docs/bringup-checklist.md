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
