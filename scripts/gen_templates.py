"""Writes the ten "popular style" starter templates into assets/templates.

Each template reproduces a gauge look that sells or gets copied a lot (black
racing dial, smoked colour-lit dial, LED-ring digital, JDM blackout, race dash,
OEM performance cluster, silver competition dial, flat smart-display arc,
white street dial, heritage chrome-bezel dial), using only the widgets in
docs/config-schema.md. Run it after changing a style; the files it writes are
what the hub embeds.
"""

import json
from pathlib import Path

OUT = Path(__file__).resolve().parent.parent / "assets" / "templates"
C = 233  # canvas centre


def face(name, widgets, bg=None):
    f = {"name": name, "widgets": widgets}
    if bg:
        f["bg"] = bg
    return f


def label(text, y, size=24, font="condensed", color=None, x=C, **kw):
    w = {"type": "label", "text": text, "x": x, "y": y, "font": font, "size": size}
    if color:
        w["color"] = color
    w.update(kw)
    return w


def dial(channel, lo, hi, major, minor=5, unit=None, **kw):
    w = {"type": "dial", "channel": channel, "min": lo, "max": hi, "major": major, "minor": minor}
    if unit:
        w["unit"] = unit
    w.update(kw)
    return w


def ring(channel, lo, hi, unit=None, **kw):
    w = {"type": "ring", "channel": channel, "min": lo, "max": hi}
    if unit:
        w["unit"] = unit
    w.update(kw)
    return w


def number(channel, unit=None, **kw):
    w = {"type": "number", "channel": channel}
    if unit:
        w["unit"] = unit
    w.update(kw)
    return w


def bar(channel, lo, hi, unit=None, **kw):
    w = {"type": "bar", "channel": channel, "min": lo, "max": hi}
    if unit:
        w["unit"] = unit
    w.update(kw)
    return w


def light(channel, **kw):
    w = {"type": "light", "channel": channel}
    w.update(kw)
    return w


def zone(lo, hi, color="alert"):
    return {"from": lo, "to": hi, "color": color}


TEMPLATES = {}

# 1. Classic black racing dial: matte black, white condensed numerals, red
#    tapered pointer with hub cap, 270 degree sweep, red redline band.
TEMPLATES["racing-black"] = {
    "schema": 1, "name": "Racing black",
    "theme": {"bg": "#0B0B0B", "fg": "#F2F2F2", "accent": "#FF3B1F", "dim": "#2A2A2A", "warn": "#FFCC00", "alert": "#FF1A1A"},
    "faces": [
        face("Boost", [
            dial("manifold_pressure", -15, 30, 9, unit="psi_gauge", labelFont="condensed", labelSize=30, needle="tapered_cap",
                 zones=[zone(25, 30)]),
            label("BOOST", 150, 26), label("PSI", 320, 22, color="dim")]),
        face("Oil pressure", [
            dial("oil_pressure", 0, 100, 10, unit="psi", labelFont="condensed", labelSize=30, needle="tapered_cap", zones=[zone(0, 15)]),
            label("OIL PRESS", 150, 26), label("PSI", 320, 22, color="dim")]),
        face("Water temp", [
            dial("coolant_temp", 40, 130, 9, unit="C", labelFont="condensed", labelSize=30, needle="tapered_cap", zones=[zone(110, 130)]),
            label("WATER TEMP", 150, 26), label("°C", 320, 22, color="dim")]),
        face("Tach", [
            dial("rpm", 0, 8000, 8, labelScale=1000, labelFont="condensed", labelSize=34, needle="tapered_cap", zones=[zone(6800, 8000)]),
            label("RPM x1000", 150, 24), {"type": "rim", "channel": "rpm", "from": 7200}]),
    ],
}

# 2. Smoked colour-lit dial: blacked out until lit, glowing blue numerals,
#    orange pointer, thin red redline arc, warning dot.
TEMPLATES["smoked-glow"] = {
    "schema": 1, "name": "Smoked glow",
    "theme": {"bg": "#000000", "fg": "#1E90FF", "accent": "#FF7A00", "dim": "#0E2238", "warn": "#FFA500", "alert": "#FF2A2A"},
    "faces": [
        face("Boost", [
            dial("manifold_pressure", -15, 30, 9, unit="psi_gauge", labelFont="condensed", labelSize=30, tickWidth=2,
                 needle="tapered", needleWidth=5, zoneWidth=3, zones=[zone(25, 30)]),
            label("BOOST", 150, 24), label("PSI", 320, 20, color="fg")]),
        face("Oil pressure", [
            dial("oil_pressure", 0, 100, 10, unit="psi", labelFont="condensed", labelSize=30, tickWidth=2,
                 needle="tapered", needleWidth=5, zoneWidth=3, zones=[zone(0, 15)]),
            label("OIL", 150, 24), label("PSI", 320, 20),
            light("oil_pressure", unit="psi", on={"below": 15}, flash=True, y=370, r=9, text="LOW", size=16)]),
        face("Trans temp", [
            dial("gearbox_oil_temp", 40, 150, 11, unit="C", labelFont="condensed", labelSize=28, tickWidth=2,
                 needle="tapered", needleWidth=5, zoneWidth=3, zones=[zone(120, 150)]),
            label("TRANS TEMP", 150, 24), label("°C", 320, 20)]),
    ],
}

# 3. Digital readout with LED ring: big 7-segment number, 24 discrete LEDs
#    round the rim, scale labels outside, name top and unit bottom.
def led_face(name, channel, lo, hi, major, unit, title, unit_text, decimals, zones_):
    return face(name, [
        ring(channel, lo, hi, unit=unit, r=233, thickness=20, start=135, sweep=270, segments=24, gap=4,
             rounded=False, color="accent", zones=zones_),
        dial(channel, lo, hi, major, 1, unit=unit, r=204, needle="none", ticks=False, labelFont="sans", labelSize=18, color="fg"),
        number(channel, unit=unit, font="digital", size=104, y=236, decimals=decimals, showUnit=False),
        label(title, 112, 24), label(unit_text, 352, 20, color="#8A8F98")])


TEMPLATES["led-ring"] = {
    "schema": 1, "name": "LED ring digital",
    "theme": {"bg": "#000000", "fg": "#FFFFFF", "accent": "#2EFF4A", "dim": "#143A1A", "warn": "#FFB000", "alert": "#FF3030"},
    "faces": [
        led_face("AFR", "wideband_1", 8, 18, 5, "afr", "AFR", "WIDEBAND", 1, [zone(8, 11, "warn"), zone(16, 18)]),
        led_face("Boost", "manifold_pressure", -15, 35, 10, "psi_gauge", "BOOST", "PSI", 1, [zone(28, 35)]),
        led_face("Oil pressure", "oil_pressure", 0, 150, 10, "psi", "OIL PRESS", "PSI", 0, [zone(0, 15)]),
        led_face("Oil temp", "oil_temp", 40, 150, 11, "C", "OIL TEMP", "°C", 0, [zone(130, 150)]),
    ],
}

# 4. JDM blackout: nothing visible until lit, thin modern numerals, thin red
#    needle with a small hub, metric scales, peak and warning dots.
def jdm_dial(channel, lo, hi, major, unit=None, **kw):
    return dial(channel, lo, hi, major, unit=unit, labelFont="sans", labelSize=24, tickWidth=2, tickLen=14,
                needle="line", needleWidth=3, needleColor="accent", zoneWidth=4, **kw)


TEMPLATES["jdm-blackout"] = {
    "schema": 1, "name": "JDM blackout",
    "theme": {"bg": "#000000", "fg": "#F4F7FF", "accent": "#E8192C", "dim": "#1C1C24", "warn": "#3A7BFF", "alert": "#FF4D1C"},
    "faces": [
        face("Boost", [
            jdm_dial("manifold_pressure", -1, 2, 6, unit="bar_gauge", zones=[zone(1.6, 2)]),
            label("BOOST", 150, 22, font="sans"), label("bar", 320, 20, font="sans", color="#8A8F98"),
            light("manifold_pressure", unit="bar_gauge", on={"above": 1.5}, y=372, r=7, color="warn", text="PEAK", size=14)]),
        face("Oil temp", [
            jdm_dial("oil_temp", 50, 150, 10, unit="C", zones=[zone(130, 150)]),
            label("OIL TEMP", 150, 22, font="sans"), label("°C", 320, 20, font="sans", color="#8A8F98"),
            light("oil_temp", unit="C", on={"above": 130}, flash=True, y=372, r=7, text="WARN", size=14)]),
        face("Water temp", [
            jdm_dial("coolant_temp", 50, 130, 8, unit="C", zones=[zone(110, 130)]),
            label("WATER TEMP", 150, 22, font="sans"), label("°C", 320, 20, font="sans", color="#8A8F98"),
            light("coolant_temp", unit="C", on={"above": 110}, flash=True, y=372, r=7, text="WARN", size=14)]),
        face("Tach", [
            jdm_dial("rpm", 0, 9000, 9, labelScale=1000, zones=[zone(7500, 9000)]),
            label("RPM x1000", 150, 20, font="sans"),
            light("rpm", on={"above": 7200}, flash=True, y=372, r=9, text="SHIFT", size=14)]),
    ],
}

# 5. Race data dash: RPM bar across the top, big gear digit, speed and the
#    vital channels in a grid with small grey labels.
def dash_face(name, cells):
    widgets = [
        bar("rpm", 0, 8000, x=C, y=56, w=380, h=26, segments=20, gap=3, color="accent",
            zones=[zone(5500, 7000, "warn"), zone(7000, 8000)]),
        {"type": "rim", "channel": "rpm", "from": 7400, "thickness": 10},
        number("gear", format="gear", font="display", size=150, x=C, y=185, showUnit=False),
    ]
    for (text, x, y, w) in cells:
        widgets.append(label(text, y - 34, 15, color="#9AA0A6", x=x))
        widgets.append(w)
    return face(name, widgets)


TEMPLATES["race-dash"] = {
    "schema": 1, "name": "Race dash",
    "theme": {"bg": "#000000", "fg": "#FFFFFF", "accent": "#2ECC40", "dim": "#333333", "warn": "#FFDC00", "alert": "#FF4136"},
    "faces": [
        dash_face("Sprint", [
            ("SPEED km/h", 120, 320, number("vehicle_speed", unit="km/h", font="display", size=52, x=120, y=320, showUnit=False)),
            ("OIL psi", 346, 320, number("oil_pressure", unit="psi", font="display", size=52, x=346, y=320, showUnit=False,
                                          warn={"below": 15, "flash": True})),
            ("WATER °C", 120, 405, number("coolant_temp", unit="C", font="display", size=40, x=120, y=405, showUnit=False,
                                               warn={"above": 110})),
            ("BOOST psi", 346, 405, number("manifold_pressure", unit="psi_gauge", font="display", size=40, x=346, y=405,
                                           decimals=1, showUnit=False)),
        ]),
        dash_face("Endurance", [
            ("OIL °C", 120, 320, number("oil_temp", unit="C", font="display", size=52, x=120, y=320, showUnit=False, warn={"above": 130})),
            ("BATT V", 346, 320, number("battery_voltage", font="display", size=52, x=346, y=320, decimals=1, showUnit=False,
                                        warn={"below": 12.5})),
            ("WATER °C", 120, 405, number("coolant_temp", unit="C", font="display", size=40, x=120, y=405, showUnit=False)),
            ("AFR", 346, 405, number("wideband_1", unit="afr", font="display", size=40, x=346, y=405, decimals=1, showUnit=False)),
        ]),
    ],
}

# 6. OEM performance-mode cluster: one dominant tach ring with red at the
#    redline, thin ticks, big speed and gear in the middle, small temp bars.
def cluster_face(name, left, right):
    (ltext, lchan, llo, lhi, lunit, lzones) = left
    (rtext, rchan, rlo, rhi, runit, rzones) = right
    return face(name, [
        ring("rpm", 0, 8000, r=233, thickness=14, start=135, sweep=270, rounded=False, color="fg", zones=[zone(6500, 8000)]),
        dial("rpm", 0, 8000, 8, 2, r=214, needle="none", tickLen=10, tickWidth=2, labelScale=1000, labelFont="condensed",
             labelSize=26, zoneWidth=8, zones=[zone(6500, 8000)]),
        {"type": "rim", "channel": "rpm", "from": 7000, "thickness": 8},
        number("vehicle_speed", unit="km/h", font="display", size=104, y=205, showUnit=False),
        label("km/h", 268, 20, color="#8A8F98"),
        number("gear", format="gear", font="display", size=56, y=330, showUnit=False),
        bar(lchan, llo, lhi, unit=lunit, x=118, y=392, w=110, h=10, color="fg", zones=lzones),
        label(ltext, 414, 14, color="#8A8F98", x=118),
        bar(rchan, rlo, rhi, unit=runit, x=348, y=392, w=110, h=10, color="fg", zones=rzones),
        label(rtext, 414, 14, color="#8A8F98", x=348),
    ])


TEMPLATES["performance-cluster"] = {
    "schema": 1, "name": "Performance cluster",
    "theme": {"bg": "#0E0E0E", "fg": "#FFFFFF", "accent": "#E0001B", "dim": "#2B2B2B", "warn": "#FFD400", "alert": "#E0001B"},
    "faces": [
        cluster_face("Sport", ("WATER", "coolant_temp", 40, 130, "C", [zone(110, 130)]), ("OIL", "oil_temp", 40, 150, "C", [zone(130, 150)])),
        cluster_face("Track", ("BOOST", "manifold_pressure", -15, 30, "psi_gauge", [zone(25, 30)]), ("OIL psi", "oil_pressure", 0, 100, "psi", [zone(0, 15)])),
    ],
}

# 7. Satin-silver competition dial: silver face, black numerals, red pointer.
def silver_face(name, channel, lo, hi, major, unit, title, unit_text, zones_, decimals_label=False):
    return face(name, [
        dial(channel, lo, hi, major, unit=unit, labelFont="condensed", labelSize=30, needle="tapered_cap", color="fg",
             zones=zones_),
        label(title, 150, 26, color="fg"), label(unit_text, 320, 22, color="#4A4A4E")])


TEMPLATES["silver-competition"] = {
    "schema": 1, "name": "Silver competition",
    "theme": {"bg": "#C8C9CB", "fg": "#111111", "accent": "#FF2E1F", "dim": "#9A9A9E", "warn": "#B35A00", "alert": "#D11A1A"},
    "faces": [
        silver_face("Oil pressure", "oil_pressure", 0, 100, 10, "psi", "OIL PRESS", "PSI", [zone(0, 15)]),
        silver_face("Water temp", "coolant_temp", 40, 130, 9, "C", "WATER TEMP", "°C", [zone(110, 130)]),
        silver_face("Volts", "battery_voltage", 8, 18, 10, None, "VOLTS", "V", [zone(8, 11), zone(15.5, 18)]),
        silver_face("Boost", "manifold_pressure", -15, 30, 9, "psi_gauge", "BOOST", "PSI", [zone(25, 30)]),
    ],
}

# 8. Flat smart-display arc: thick rounded arc, huge centred numeral, small
#    grey caption above and unit below; the arc changes colour at the limit.
def arc_face(name, channel, lo, hi, unit, title, unit_text, decimals, zones_):
    return face(name, [
        ring(channel, lo, hi, unit=unit, r=233, thickness=34, start=135, sweep=270, rounded=True, color="accent", zones=zones_),
        label(title, 140, 24, font="sans", color="#8A8F98"),
        number(channel, unit=unit, font="sans", size=120, y=236, decimals=decimals, showUnit=False),
        label(unit_text, 330, 26, font="sans", color="#8A8F98")])


TEMPLATES["smart-arc"] = {
    "schema": 1, "name": "Smart arc",
    "theme": {"bg": "#000000", "fg": "#FFFFFF", "accent": "#00E5FF", "dim": "#1A1F24", "warn": "#FFB300", "alert": "#FF3D00"},
    "faces": [
        arc_face("Boost", "manifold_pressure", -15, 30, "psi_gauge", "BOOST", "psi", 1, [zone(25, 30)]),
        arc_face("Coolant", "coolant_temp", 40, 130, "C", "COOLANT", "°C", 0, [zone(100, 110, "warn"), zone(110, 130)]),
        arc_face("Battery", "battery_voltage", 8, 18, None, "BATTERY", "volts", 1, [zone(8, 12, "warn")]),
        arc_face("Oil pressure", "oil_pressure", 0, 100, "psi", "OIL PRESSURE", "psi", 0, [zone(0, 15)]),
    ],
}

# 9. White-face street dial: off-white face, black bold condensed numerals,
#    orange tapered pointer with hub, red redline arc.
def white_face(name, channel, lo, hi, major, unit, title, unit_text, zones_, **kw):
    widgets = [
        dial(channel, lo, hi, major, unit=unit, labelFont="condensed", labelSize=32, needle="tapered_cap", color="fg",
             zones=zones_, **kw),
        label(title, 150, 26, color="fg")]
    if unit_text:
        widgets.append(label(unit_text, 320, 22, color="#55555A"))
    return face(name, widgets)


TEMPLATES["street-white"] = {
    "schema": 1, "name": "Street white",
    "theme": {"bg": "#F5F5F0", "fg": "#111111", "accent": "#FF6A00", "dim": "#C9C9C4", "warn": "#C77700", "alert": "#D40000"},
    "faces": [
        white_face("Tach", "rpm", 0, 8000, 8, None, "RPM x1000", "", [zone(6500, 8000)], labelScale=1000),
        white_face("Oil pressure", "oil_pressure", 0, 100, 10, "psi", "OIL PRESS", "PSI", [zone(0, 15)]),
        white_face("Water temp", "coolant_temp", 40, 130, 9, "C", "WATER TEMP", "°C", [zone(110, 130)]),
        white_face("Volts", "battery_voltage", 8, 18, 10, None, "VOLTS", "V", [zone(8, 11), zone(15.5, 18)]),
    ],
}

# 10. Heritage chrome-bezel dial: black tach with green print and a thin white
#     needle, cream quarter-sweep small gauges with black print.
TEMPLATES["heritage"] = {
    "schema": 1, "name": "Heritage",
    "theme": {"bg": "#0A0A0A", "fg": "#7ED99A", "accent": "#F2F2F2", "dim": "#1E2A22", "warn": "#FFD27A", "alert": "#D62828"},
    "faces": [
        face("Tach", [
            dial("rpm", 0, 8000, 8, r=226, tickWidth=1, tickLen=14, labelFont="sans", labelSize=24, labelScale=1000,
                 needle="line", needleWidth=3, zoneWidth=5, zones=[zone(7000, 8000)]),
            label("RPM x 1000", 150, 18, font="sans"), label("HERITAGE", 320, 14, font="sans", color="#6B8F76")]),
        face("Oil pressure", [
            dial("oil_pressure", 0, 100, 4, unit="psi", x=C, y=318, r=232, start=210, sweep=120, tickWidth=1, tickLen=14,
                 labelFont="sans", labelSize=26, color="#111111", needle="line", needleWidth=3, needleColor="#111111",
                 zoneWidth=5, zones=[zone(0, 15)]),
            label("OIL", 200, 22, font="sans", color="#111111"), label("lb/in²", 228, 16, font="sans", color="#55554E")],
             bg="#EDE4CB"),
        face("Water temp", [
            dial("coolant_temp", 40, 130, 3, unit="C", x=C, y=318, r=232, start=210, sweep=120, tickWidth=1, tickLen=14,
                 labelFont="sans", labelSize=26, color="#111111", needle="line", needleWidth=3, needleColor="#111111",
                 zoneWidth=5, zones=[zone(110, 130)]),
            label("WATER", 200, 22, font="sans", color="#111111"), label("°C", 228, 16, font="sans", color="#55554E")],
             bg="#EDE4CB"),
    ],
}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for name, cfg in TEMPLATES.items():
        path = OUT / f"{name}.json"
        path.write_text(json.dumps(cfg, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        widgets = sum(len(f["widgets"]) for f in cfg["faces"])
        print(f"{path.name}: {len(cfg['faces'])} faces, {widgets} widgets, {path.stat().st_size} bytes")


if __name__ == "__main__":
    main()
