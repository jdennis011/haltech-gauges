# Time of day

The hub keeps the clock and the gauges show it. A gauge never needs a zone, a network
or a battery of its own: the hub's beacon carries the local time twice a second.

## Where the hub gets the time

In order of trust, each taking over from the one before:

1. **NTP**, whenever the hub is on the home network. Checked every hour.
2. **The phone or PC** looking at the page. When the hub reports no time, the page sends
   its device's clock once; **Set from this device** on Settings, Time, does it by hand.
   Ignored once NTP has set the time.
3. **The clock chip**, a DS3231 on the carrier's I2C bus (GPIO7/8 on the P4 carrier, GPIO6/7
   on the C6 carrier; electrical-spec.md), with its ML2032 cell. The hub reads it at start,
   and writes it after every NTP sync, after the device's clock, and once a day. The chip
   keeps UTC and reports whether its oscillator has stopped since it was last set, in
   which case its time is not used.

A hub with none of these shows no time, and the gauges' clocks show dashes.

**Time zone.** Settings, Time: a list of zones (Australian capitals, New Zealand, a few
others), or a custom POSIX TZ string such as `AEST-10` or
`AEST-10AEDT,M10.1.0,M4.1.0/3` for daylight saving. Kept in the hub's preferences;
the default is Brisbane. Daylight saving rules in the string are applied by the hub, so
the gauges need no change at the switch.

## On the gauge bus

The beacon (type 0, 2 Hz) grew from four bytes to eight: bytes 4 to 7 carry the local
time as seconds since 1970 in the hub's zone, big-endian, and flag bit 1 says they are
valid. A gauge keeps the last value running on its own millisecond clock between
beacons, and after the hub goes away, so the clock does not stop at key-off while the
gauge is still powered. Gauges with older firmware ignore the extra bytes.

## The `clock` widget

A widget with no channel, like a label:

```json
{ "type": "clock", "y": 233, "size": 56, "seconds": false, "format": "24h" }
{ "type": "clock", "style": "analogue", "r": 120, "seconds": true }
```

| Key | |
|---|---|
| `style` | `digital` (text) or `analogue` (hands) (`digital`) |
| `seconds` | Show seconds, or a second hand (`false`) |
| `format` | `24h` or `12h`, digital only: `13:05` or `1:05 pm` (`24h`) |
| `font`, `size`, `color`, `align`, `rotate` | As for a number; `size` defaults to 56 |
| `r` | Analogue: radius (120) |
| `thickness` | Analogue: width of the hour hand; the minute hand is a little thinner, the second hand thin (6) |
| `ticks`, `tickColor` | Analogue: twelve marks around the edge (`true`, `dim`) |
| `secondColor` | Analogue: the second hand (`accent`) |

Without a time the digital clock shows `--:--` and the analogue one its marks and no
hands. The page's preview shows the browser's own clock.

## API

| Method and path | |
|---|---|
| GET `/api/time` | `{set, source, unix, local, tz, tz_name, rtc, rtc_ok, synced_s}`; also under `time` in `/api/status`. `source` is `ntp`, `phone or PC`, `clock chip` or `none` |
| PUT `/api/time` `{"unix": 1760600000}` | The device's clock; refused with 409 once NTP has the time |
| PUT `/api/time` `{"tz": "AEST-10", "tz_name": "Australia/Brisbane"}` | The zone |

## What is and is not built

Built: the hub's sources, zone and beacon field; the gauge's clock from the beacon and
the widget in the format, the page's preview and the renderer; host tests for the format
and the beacon. Not yet seen on a panel, like the rest of the renderer. The clock chip
code is written to the DS3231 datasheet and waits for a carrier with one fitted.
