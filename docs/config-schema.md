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
- **Colours** are `#RRGGBB`, `#RGB`, a theme role (`bg`, `fg`, `accent`, `dim`, `warn`, `alert`),
  or one of the hub's theme colours, `themecolour1` to `themecolour8` (below).
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
  roles as hex colours or theme colours.

### Theme colours

The hub keeps eight colours of its own, shared by every config and every gauge: by
default 1 white, 2 orange, 3 red, 4 yellow, 5 green, 6 blue, 7 grey and 8 black, each with
a name of up to 16 characters. A config uses them as `themecolour1` to `themecolour8` (the
US spelling `themecolor1` also works) anywhere a colour goes: a widget's `color`,
`needleColor`, `trackColor`, `offColor` or `strokeColor`, a zone or warn colour, a face's
`bg`, or a role in the config's own theme object, so that `"theme": {"fg": "themecolour1"}`
puts every widget in its default colour onto hub colour 1.

Change a theme colour on the hub (the Theme colours card on the Configs tab, or
`PUT /api/theme-colours`) and every widget that uses it changes with it, on every face of
every gauge, without pushing any config again. A config's own theme roles stay per
config; theme colours are the way to share a colour across configs.

The gauges get the colours over the bus (message `ThemeColours`, type 7: four frames, each
the frame number then two colours as 3-byte RGB) when they change, when a gauge comes
online, and every 10 seconds. A gauge keeps the last ones in flash, so it shows the same
colours when running without a hub.

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
`channel` and may take `unit`; for `shape` and `path` the channel is optional. Every widget except
`light`, `shape` and `path` may take
`warn: { "above": n }` or `{ "below": n }` with optional `color` (default `alert`) and `flash`,
which recolours it past the threshold.

| Type | Keys (default) |
|---|---|
| `dial` | `min`, `max` (required); `r` outer radius (220); `start` (135), `sweep` (270); `major` divisions (10), `minor` subdivisions (5); `tickLen` (16), `tickWidth` (3), `ticks` (true; false leaves only the labels), `tickShape`: `line`, `triangle` (`line`), `arcWidth` (0; a thin arc along the sweep where the ticks start); `labels` (true), `labelFont` (`sans`), `labelSize` (22), `labelScale` (1; use 1000 for RPM x1000); `needle`: `line`, `tapered`, `tapered_cap`, `none`, `marker` (`tapered`; `none` makes a labelled scale to pair with a ring; `marker` is an arrow with its base on radius `r` pointing inward at the value, `marker_out` one with its tip on `r` pointing outward, both `needleLen` long (28) and `needleWidth` half as wide at the base, for a set point or target); `needleColor` (`accent`), `needleWidth` (6), `needleLen` (r - 24); `zones`, `zoneWidth` (6) |
| `ring` | `min`, `max` (required); `r` (225), `thickness` (20); `start`, `sweep`; `rounded` (true); `track` (true), `trackColor` (`dim`); `segments` (0 = solid), `gap` degrees (3); `zones` |
| `bar` | `min`, `max` (required); `style`: `horizontal`, `vertical`; `w`, `h` (260 x 28, swapped when vertical); `track`, `trackColor`; `segments`, `gap` pixels (3); `cornerRadius` (4); `rotate` (0; degrees clockwise about the centre); `zones` |
| `number` | `font` (`sans`), `size` (72), `decimals` (0); `rotate` (0; degrees clockwise); `format`: `plain`, `gear` (`plain`; `gear` shows N, R, P or the gear number); `pad` (0; minimum digits, zero-filled, e.g. 3 for `000`); `showUnit` (true), `unitSize` (size / 3), `unitPos`: `below`, `right`, `left`, `above` (`below`), `unitDx`, `unitDy` (0; nudge the unit from that position, pixels); `align`; `hold`: `none`, `max`, `min` |
| `label` | `text` (required, up to 32 characters); `font`, `size` (24), `align`; `rotate` (0; degrees clockwise); `arc` (0; radius of a circle centred on `x`, `y` that the text follows, so 0 is straight text), `arcAngle` (270; where the middle of the text sits, degrees clockwise from 3 o'clock, so 270 is the top), `arcSide`: `auto`, `inside`, `outside` (`auto` faces the letters outward on the top half and inward on the bottom, so both read left to right) |
| `light` | `r` half size (14); `shape`: `dot`, `ring`, `square`, `text` (shows `text` alone, only while lit); `rotate` (0); `color` when lit (`alert`), `offColor` (`dim`); `on`: `{ "above": n }` or `{ "below": n }` (default above 0.5, i.e. a status flag is set); `flash`; `text` caption |
| `rim` | `from` (required); `mode`: `flash` (whole rim lights from `from` upward) or `fill` (fills between `from` and `to`, `to` required); `r` (233), `thickness` (12); `color` (`alert`) |

| `shape` | `shape`: `rect`, `ellipse`, `line`, `triangle`, `polygon` (`rect`); `w`, `h` (100 x 60; a line's length is `w`); `cornerRadius` (0); `filled` (true), `color` fill, `strokeColor`, `strokeWidth` (0, or 3 when not filled), `opacity` (100), `rotate` (0); a polygon takes `points`, 3 to 16 x, y pairs relative to `x`, `y`. `channel` is optional: with one, the shape is drawn in `color` while `on` holds (as for a light) and in `offColor` otherwise, or not at all with `hideWhenOff`; `flash` |
| `path` | `d` (required): SVG path data, up to 1024 characters, as copied from an icon's `<path d="...">`; `box` (24), the size of the path's own coordinate square; `size` (48), how large to draw it; `filled`, `color`, `strokeColor`, `strokeWidth`, `opacity`, `rotate`, and the optional `channel`, `on`, `offColor`, `hideWhenOff`, `flash` as for `shape`. A path with a channel is a vector warning icon |

`zones` is a list of up to 6 `{ "from": a, "to": b, "color": c }` ranges. On a dial they are
drawn as a coloured band on the rim (a redline). On a ring or bar the indicator takes the
zone's colour while the value is inside it.
