# Alerts

An alert is a rule the hub checks against the live data: a channel, a condition,
and what the gauges do while the condition holds. It is the hub's own warning
system on top of the ECU's: an extra "check engine light" for anything the ECU
broadcasts.

- Coolant temperature goes above 105 °C: every gauge shows **COOLANT HOT**.
- Lambda is outside 0.75 to 1.15 for a second: the left gauge switches to its
  mixture face and shows **CHECK MIXTURE**, then goes back.
- The check engine light comes on: every gauge shows **CHECK ENGINE**.
- The gear changes: the centre gauge switches to its gear face for two seconds.

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
| `face` | Face to switch to, counted from 0, up to 7 (no change) |
| `restore` | Go back to the face that was showing when the alert ends (`true`) |
| `gauge` | `"all"`, or the MAC of one gauge, e.g. `"A0:76:4E:11:22:33"` (`"all"`) |

A rule needs a `message`, a `face`, or both. Up to 16 rules.

## How it behaves

- **Firing.** The condition must hold continuously for `for` seconds. A reading
  that dips back restarts the wait.
- **Ending.** The alert stays in force for `hold` seconds after the condition
  last held, so a value hovering at the limit does not make the message flicker.
- **No data.** A channel that has stopped arriving meets no condition: an alert
  on it ends after its hold time, and "below 100" is not true of a dead sensor.
  Data coming back is not counted as a change.
- **Two at once.** Where two alerts in force apply to the same gauge, the one
  higher in the list wins, separately for the message and for the face: a gauge
  can show the first alert's message on the second alert's face if the first has
  no face of its own.
- **The message** is a band across the middle of the face, in the alert's colour,
  over whatever face is showing. The hub repeats it every second while the alert
  is in force; a gauge takes it down three seconds after the last repeat, so a
  hub that is switched off or unplugged cannot leave a message up.
- **The face.** The hub notes the face the gauge was on, switches it, and with
  `restore` switches it back when the alert ends. Swiping to another face during
  the alert is not fought: the hub sends the switch once.
- **A gauge that connects late** gets the messages and faces of whatever alerts
  are in force at that moment.
- **Test** on the page puts one alert in force for five seconds whatever its
  condition says, to see it on the gauges. It is not counted as a firing.

Thresholds are compared in the rule's unit, so `"unit": "F", "value": 221` and
`"unit": "C", "value": 105` are the same alert.

## On the page

The **Alerts** tab lists the rules in priority order. Each row has the
condition (channel, kind, limits, unit, delay), what to do (message and colour,
face, back afterwards, which gauge, hold time), a switch to turn it off, **Test**,
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

A face switch is the existing `SetFace` command. The message is a new message
type, `Alert` (6), from the hub to one gauge:

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
protocol message with its receiver on the gauge side (`lib/hg_proto`), the page,
and the online copy. Host tests cover the rules and run them end to end against
simulated gauges.

Not yet: the gauge draws nothing for it. The gauge firmware receives the message
and keeps it (`can_task::alertMessage()`), and prints it on its serial port; the
band on the screen, and obeying `SetFace`, come with the face renderer (plan
phase 4).

Ideas left out for now: the live reading inside the message ("COOLANT 108"), a
second condition on a rule (oil pressure low *while* the engine is running), and
a sound or an output pin on the hub.
