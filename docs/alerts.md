# Alerts

An alert is a rule the hub checks against the live data: a channel, a condition,
and what the gauges do while the condition holds. It is the hub's own warning
system on top of the ECU's: an extra "check engine light" for anything the ECU
broadcasts, and a way to put a different gauge in front of the driver when
something is going on.

- Coolant temperature goes above 105 °C: every gauge shows **COOLANT HOT**.
- Lambda is outside 0.75 to 1.15 for a second: the left gauge switches to its
  mixture face and shows **CHECK MIXTURE**, then goes back.
- The check engine light comes on: every gauge shows **CHECK ENGINE**.
- The gear changes: the centre gauge switches to its gear face for two seconds.
- Cruise control engages (the ECU's cruise state goes above 1): the centre gauge
  switches to the **Cruise** config from the library, on its first face, and goes
  back to its own config a second after cruise disengages. A car that never had a
  cruise light gets one.

Alerts are edited on the page's **Alerts** tab and kept on the hub in
`/alerts.json`, so they work with no phone or laptop connected. The hub checks
them ten times a second.

## A rule

```json
{
  "name": "Coolant hot",
  "enabled": true,
  "channel": "coolant_temp",
  "unit": "C",
  "when": "above",
  "value": 105,
  "for": 1,
  "hold": 3,
  "message": "COOLANT HOT",
  "color": "#FF3B30",
  "face": 2,
  "restore": true,
  "gauge": "all"
}
```

| Key | Meaning (default) |
|---|---|
| `name` | Up to 24 characters, for the list (none) |
| `enabled` | `false` switches the rule off without deleting it (`true`) |
| `channel` | Required. Any channel name from the Live data tab |
| `unit` | The unit `value`, `low` and `high` are in: any unit a face config accepts for that channel, e.g. `C`, `F`, `kPa`, `psi`, `lambda`, `afr` (the channel's own unit) |
| `when` | Required. `above`, `below`: compared with `value`. `outside`, `inside`: compared with `low` and `high` (`inside` includes both ends). `on`, `off`: a status flag is set or clear. `changes`: the reading is different from the one before |
| `value` | Required for `above` and `below` |
| `low`, `high` | Required for `outside` and `inside`; `low` must be less than `high` |
| `for` | Seconds the condition must hold before the alert fires, 0 to 600 (0). Rides out a spike or a noisy sensor. Not used by `changes` |
| `hold` | Seconds the alert stays in force after the condition stops holding, 0 to 600 (3). For `changes`, how long each change is shown |
| `message` | Text shown over the face, up to 32 characters (none) |
| `color` | Colour of the message, `#RRGGBB` (`#FF3B30`) |
| `sound` | Played on the gauge's speaker when the alert fires: `none`, `beep`, `chime` or `alarm` (`none`). Needs gauge firmware 0.1.5 and a speaker (docs/gauge-audio.md) |
| `repeat` | Seconds between plays while the alert stays in force, 0 to 600 (0: once, when it fires) |
| `config` | The name of a config in the hub's library to show in place of the gauge's own while the alert is in force, up to 32 characters (none). The gauge goes back to its own config when the alert ends |
| `face` | Face to switch to, counted from 0, up to 7 (no change). With a `config`, a face of that config |
| `restore` | Go back to the face that was showing when the alert ends (`true`) |
| `gauge` | `"all"`, or the MAC of one gauge, e.g. `"A0:76:4E:11:22:33"` (`"all"`) |

A rule needs a `message`, a `config`, a `face`, or some of them. Up to 16 rules.

The cruise control example:

```json
{
  "name": "Cruise", "channel": "cruise_state", "when": "above", "value": 1,
  "hold": 1, "config": "Cruise", "face": 0, "gauge": "A0:76:4E:11:22:33"
}
```

## How it behaves

- **Firing.** The condition must hold continuously for `for` seconds. A reading
  that dips back restarts the wait.
- **Ending.** The alert stays in force for `hold` seconds after the condition
  last held, so a value hovering at the limit does not make the message flicker.
- **No data.** A channel that has stopped arriving meets no condition: an alert
  on it ends after its hold time, and "below 100" is not true of a dead sensor.
  Data coming back is not counted as a change.
- **Two at once.** Where two alerts in force apply to the same gauge, the one
  higher in the list wins, separately for the message, for the config and for
  the face: a gauge can show the first alert's message on the second alert's
  face if the first has no face of its own. While an alert's config is showing,
  the face is that alert's own; other alerts' faces wait until it ends.
- **The message** is a band across the middle of the face, in the alert's colour,
  over whatever face is showing. The hub repeats it every second while the alert
  is in force; a gauge takes it down three seconds after the last repeat, so a
  hub that is switched off or unplugged cannot leave a message up.
- **The sound** plays on each gauge the rule applies to when the alert comes into
  force, and again every `repeat` seconds while it holds. It goes as a command on
  the bus, so a play lost to a busy bus is owed until it gets through. A gauge
  that connects while an alert is already in force does not play the sound it
  missed, and neither does any gauge when the rules are replaced; the repeats
  carry on. A gauge without a speaker ignores it.
- **The face.** The hub notes the face the gauge was on, switches it, and with
  `restore` switches it back when the alert ends. Swiping to another face during
  the alert is not fought: the hub sends the switch once.
- **The config.** The hub sends the named config to the gauge the way **Try on
  gauge** does: the images it uses first, if the gauge lacks them, then the
  config, shown from RAM without touching the gauge's stored config. Once the
  gauge reports it is showing it, the hub switches to the rule's `face`. When
  the alert ends the gauge goes back to its own config, and with `restore` to
  the face it was on. A 4 KB config lands in under a second; images the gauge
  does not have yet take a few seconds each the first time, and the gauge keeps
  them until a stored config no longer uses them. A config the hub does not have
  does nothing. A gauge that ends up back on its own config while the alert
  still holds (the page's **End try**, or a config stored over it) is switched
  again a few seconds later; a gauge that resets is switched again when it is
  back.
- **A gauge that connects late** gets the messages and faces of whatever alerts
  are in force at that moment.
- **Test** on the page puts one alert in force for five seconds whatever its
  condition says, to see it on the gauges. It is not counted as a firing.

Thresholds are compared in the rule's unit, so `"unit": "F", "value": 221` and
`"unit": "C", "value": 105` are the same alert.

## On the page

The **Alerts** tab lists the rules in priority order. Each row has the
condition (channel, kind, limits, unit, delay), what to do (message and colour,
sound and how often, face, back afterwards, which gauge, hold time), a switch to turn it off, **Test**,
and arrows to change its priority. A chip on each row shows whether it is in
force, the current reading and how many times it has fired since the rules were
last changed. Changes are saved as they are made; the hub checks the whole list
and refuses it, saying which rule is wrong and why, if any rule is.

**Add alert** starts from a preset: coolant hot, oil pressure low, lambda out of
range, check engine light, battery low, shift.

The previews on the **Gauges** tab show the message band and the alert's face
while an alert is in force, for planned gauges too, so alerts can be set up and
tried against the simulator or a recording before the hardware exists.

**Export all** and **Import all** on the Configs tab carry the alerts with the
configs. In the online copy alerts are kept in the browser, and in the account
when signed in; the demo checks them the same way the hub does.

## API

| Method and path | Purpose |
|---|---|
| GET `/api/alerts` | `{"rules": [...], "states": [{"active", "count", "value"}, ...], "max": 16}` |
| PUT `/api/alerts` (body: the list of rules) | Replaces every rule. 400 with `alert 2 (Name): reason` if one is wrong, and nothing changes |
| POST `/api/alerts/test` `{"index": 0, "seconds": 5}` | Puts one alert in force for a while |

The WebSocket pushes `{"t":"alerts","states":[...]}` when an alert fires or ends,
and once a second while there are rules, so the readings on the page stay current.

## On the gauge bus

A face switch is the existing `SetFace` command. A config switch is a segmented
transfer of kind `SwitchConfig` (4): the same as the page's `TryConfig` (2),
shown from RAM and reported with the status's "trying" flag, except that the
gauge does not mark it as a trial on the screen; `EndTry` ends either. The
message is a new message type, `Alert` (6), from the hub to one gauge:

| Frame | Bytes |
|---|---|
| Header | `seq << 4`, flags (bit 0: show), text length, colour R, G, B, seconds to live |
| Text 1 to 5 | `seq << 4 \| n`, then up to 7 bytes of UTF-8 text |

`seq` (0 to 15) changes with each new message, so a gauge can tell a repeat from
a replacement. A header with the show bit clear takes the message down. Frames
lost on the bus are made good by the next repeat.

## What is and is not built

Done: the rule format, its evaluation and the sending to the gauges
(`lib/hg_alerts`), the hub's storage and API (`src/hub/alert_monitor.cpp`), the
protocol message with its receiver on the gauge side (`lib/hg_proto`), the
config switch, the page, and the online copy. Host tests cover the rules and run
them end to end against simulated gauges, config switch included.

On the gauge, the band on the screen, `SetFace` and the switched config are in
the face screen (`src/gauge/face_ui.cpp`, `face_store.cpp`), written with the
renderer and not yet seen on a panel.

Sounds (2026-10-28): a rule's `sound` and `repeat` go to the gauges as `PlaySound`
commands from the dispatcher; the gauge plays them through its speaker
(docs/gauge-audio.md). Covered by the host tests.

Ideas left out for now: the live reading inside the message ("COOLANT 108"), a
second condition on a rule (oil pressure low *while* the engine is running), and
an output pin on the hub.
