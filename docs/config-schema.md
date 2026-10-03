# Gauge config format (schema 1)

A config is one JSON document, at most 32 KB, holding a theme and up to 8
faces. A face is a list of up to 16 widgets drawn in order. The gauge swipes
between faces. The parser is `lib/hg_config/face_parse.cpp`; anything it
rejects comes back with the location, for example
`faces[1].widgets[2]: unknown channel 'boost'`. Unknown keys are ignored, so
newer configs still load on older firmware as long as `schema` matches.

```json
{
  "schema": 1,
  "name": "Street",
  "theme": "white",
  "faces": [
    {
      "name": "Boost",
      "widgets": [
        { "type": "dial", "channel": "manifold_pressure", "unit": "psi_gauge",
          "min": -15, "max": 30, "major": 9,
          "zones": [ { "from": 22, "to": 30, "color": "alert" } ] },
        { "type": "number", "channel": "manifold_pressure", "unit": "psi_gauge",
          "y": 330, "font": "digital", "size": 64, "decimals": 1,
          "warn": { "above": 25, "flash": true } },
        { "type": "label", "text": "BOOST", "y": 140, "size": 28, "color": "dim" }
      ]
    }
  ]
}
```

## Conventions

- **Geometry** is in pixels on the 466 x 466 round panel, origin top-left. `x`, `y` are the
  widget's centre and default to the middle of the screen (233, 233).
- **Angles** are degrees, 0 at 3 o'clock, clockwise. A classic dial is `start` 135, `sweep` 270.
- **Colours** are `#RRGGBB`, `#RGB`, or a theme role: `bg`, `fg`, `accent`, `dim`, `warn`, `alert`.
- **Channels** are the names in `lib/hg_haltech/haltech_channels.def` (`rpm`, `oil_pressure`,
  `coolant_temp`, ...).
- **Units** are optional. Without one the channel's native unit is used. `min`, `max`, zones and
  thresholds are all in the chosen unit.

| Channel kind | `unit` values |
|---|---|
| Gauge pressure | `kPa`, `psi`, `bar` |
| Absolute pressure (manifold, baro) | `kPa`, `psi`, `bar`, and relative to atmosphere: `kPa_gauge`, `psi_gauge`, `bar_gauge` |
| Temperature | `C`, `F` |
| Speed | `km/h`, `mph` |
| Lambda | `lambda`, `afr` |
| Acceleration | `m/s2`, `g` |
| Volume | `L`, `gal` |

- **Fonts**: `sans` (Fira Sans), `condensed` (Barlow Condensed), `digital` (DSEG7, 7-segment),
  `mono` (Share Tech Mono), `display` (Russo One), `carter` (Carter One), `racing` (Racing Sans One),
  `trade` (Trade Winds). Characters a family lacks are drawn in `sans`.
  The hub's editor lists them, drawn with the real files, and offers them per widget.
- **Theme**: a preset name (`white`, `red`, `amber`, `green`, `cyan`, `nfs`) or an object giving any of the six
  roles as hex colours.

## Top level and faces

| Key | Required | Notes |
|---|---|---|
| `schema` | yes | `1` |
| `name` | | up to 32 characters |
| `theme` | | preset name or colour object; default `white` |
| `faces` | yes | 1 to 8 faces |
| face `name` | | |
| face `bg` | | default: the theme's `bg` |
| face `widgets` | yes | 1 to 16 widgets |

## Widgets

Every widget takes `type`, `x`, `y`, `color` (default `fg`). Every widget except `label` needs
`channel` and may take `unit`. Every widget except `light` may take
`warn: { "above": n }` or `{ "below": n }` with optional `color` (default `alert`) and `flash`,
which recolours it past the threshold.

| Type | Keys (default) |
|---|---|
| `dial` | `min`, `max` (required); `r` outer radius (220); `start` (135), `sweep` (270); `major` divisions (10), `minor` subdivisions (5); `tickLen` (16), `tickWidth` (3), `ticks` (true; false leaves only the labels), `tickShape`: `line`, `triangle` (`line`), `arcWidth` (0; a thin arc along the sweep where the ticks start); `labels` (true), `labelFont` (`sans`), `labelSize` (22), `labelScale` (1; use 1000 for RPM x1000); `needle`: `line`, `tapered`, `tapered_cap`, `none`, `marker` (`tapered`; `none` makes a labelled scale to pair with a ring; `marker` is an arrow on the rim pointing inward at the value, `needleLen` long (28) and `needleWidth` half as wide at its base, for a set point or target); `needleColor` (`accent`), `needleWidth` (6), `needleLen` (r - 24); `zones`, `zoneWidth` (6) |
| `ring` | `min`, `max` (required); `r` (225), `thickness` (20); `start`, `sweep`; `rounded` (true); `track` (true), `trackColor` (`dim`); `segments` (0 = solid), `gap` degrees (3); `zones` |
| `bar` | `min`, `max` (required); `style`: `horizontal`, `vertical`; `w`, `h` (260 x 28, swapped when vertical); `track`, `trackColor`; `segments`, `gap` pixels (3); `cornerRadius` (4); `rotate` (0; degrees clockwise about the centre); `zones` |
| `number` | `font` (`sans`), `size` (72), `decimals` (0); `rotate` (0; degrees clockwise); `format`: `plain`, `gear` (`plain`; `gear` shows N, R, P or the gear number); `pad` (0; minimum digits, zero-filled, e.g. 3 for `000`); `showUnit` (true), `unitSize` (size / 3), `unitPos`: `below`, `right`; `align`; `hold`: `none`, `max`, `min` |
| `label` | `text` (required, up to 32 characters); `font`, `size` (24), `align`; `rotate` (0; degrees clockwise, for text that follows an arc) |
| `light` | `r` half size (14); `shape`: `dot`, `ring`, `square`, `text` (shows `text` alone, only while lit); `rotate` (0); `color` when lit (`alert`), `offColor` (`dim`); `on`: `{ "above": n }` or `{ "below": n }` (default above 0.5, i.e. a status flag is set); `flash`; `text` caption |
| `rim` | `from` (required); `mode`: `flash` (whole rim lights from `from` upward) or `fill` (fills between `from` and `to`, `to` required); `r` (233), `thickness` (12); `color` (`alert`) |

`zones` is a list of up to 6 `{ "from": a, "to": b, "color": c }` ranges. On a dial they are
drawn as a coloured band on the rim (a redline). On a ring or bar the indicator takes the
zone's colour while the value is inside it.
