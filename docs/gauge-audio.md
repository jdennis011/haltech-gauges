# Gauge speaker

The Waveshare ESP32-S3-Touch-AMOLED-1.75 carries an ES8311 audio codec with a small
speaker amplifier and an MX1.25 2-pin speaker socket on the back of the board. From gauge
firmware 0.1.4 the codec is set up at start and the gauge can play tones through it; 0.1.5
adds the alert sounds and the volume from the hub.

## Wiring on the board

| Signal | GPIO | Note |
|---|---|---|
| I2S MCLK | 42 | 4.096 MHz, 256 times the sample rate |
| I2S BCLK | 9 | |
| I2S WS (LRCK) | 45 | |
| I2S data to the codec | 8 | the codec's DSDIN |
| I2S data from the codec | 10 | the microphone path, not used |
| Amplifier enable | 46 | high = on; driven high only while something plays and for 300 ms after |
| Codec control | I2C 0x18 | on the shared bus (SDA 15, SCL 14) with the touch controller and the PMIC |

Pins from Waveshare's `08_ES8311` demo and the board's ESP-IDF board support header.
The codec runs at 16 kHz, 16-bit, as the I2S slave, with MCLK from the MCLK pin, which is
the sequence in Espressif's `es8311` driver reduced to the playback path.

## What plays

- **Identify.** The hub's Identify (the button on a gauge card, or for all gauges) shows
  the gauge's id on the screen as before, and now also plays a two-note chime
  (880 Hz then 1320 Hz). That is the test from the page: a gauge with a speaker plugged
  in sounds when identified.
- **Alerts.** A rule on the hub's Alerts tab can name a sound (`beep`, `chime` or
  `alarm`) and how often to play it again while the alert holds (docs/alerts.md). The
  hub sends `PlaySound` commands; the gauge plays them at its volume.
- **Volume from the hub.** A gauge with a speaker reports its volume in its status, so
  its card on the Gauges tab shows a volume slider (0 to 100 in steps of 4) and a
  **Beep** button to hear it; the All gauges strip sets every speaker at once. The
  gauge keeps the value in flash. The commands are `SetVolume` and `PlaySound`.
- **Console.** On the gauge's USB serial port:

  | Command | Does |
  |---|---|
  | `tone [HZ] [MS]` | a sine tone, 50 to 8000 Hz, 10 to 5000 ms (default 1000 Hz, 300 ms) |
  | `chime` | the identify chime |
  | `volume N` | speaker volume 0 to 100, kept in flash (default 60); 0 mutes the codec |
  | `info` | now ends with `audio codec` or `audio none` |

Tones are queued and rendered by a task of their own, so neither the CAN task nor the
screen waits on them. Each note has a 5 ms ramp in and out so it does not click, and the
amplifier is woken on silence before the first note.

A gauge whose codec does not answer (or the M5Stack Dial build, which has no audio)
prints `Audio: no ES8311 codec on the I2C bus` at start and ignores the commands.

## Proven

2026-10-28, gauge D02424 with a speaker on the socket, 0.1.4 flashed over USB: the console's
`chime`, `tone 1000 600` and `tone 440 400` and the hub's Identify all sounded at the default
volume of 60. The codec answered first time with no PMIC rail to enable.

## Not done

- The pull-down menu on the gauge has no volume slider; the card on the hub's page and
  the console set it.
- No sound clips. Planned as a future feature (the plan file's "Future features"): WAV clips
  converted in the browser to 16 kHz mono, kept in a hub library like the images and sent the
  same way, played with the I2S library's WAV player; an MP3 would be converted on the page,
  not decoded on the gauge.
- The microphone path is left off.
