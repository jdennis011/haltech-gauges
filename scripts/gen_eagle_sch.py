"""Generates Eagle 7 XML schematics for the gauge adapter and the hub carriers.

    python scripts/gen_eagle_sch.py

Writes hardware/gauge-adapter.sch, hardware/hub-carrier.sch (ESP32-C6 DevKit)
and hardware/hub-carrier-p4.sch (Olimex ESP32-P4-DevKit). They import
into EasyEDA (File > Import > Eagle). The circuits are the ones described in
docs/electrical-spec.md; this script is their machine-readable form.

Every pin gets a short wire and a net label; pins that share a label are
connected. Standard library only.
"""

import os
import xml.etree.ElementTree as ET

G = 2.54          # schematic grid, mm
PIN_PITCH = 2 * G  # spacing between pins on box symbols
STUB = G           # length of the wire drawn from each pin


def f(value):
    text = f"{value:.4f}".rstrip("0").rstrip(".")
    return text if text not in ("", "-0") else "0"


# ---------------------------------------------------------------- symbols

class Symbol:
    def __init__(self, name):
        self.name = name
        self.pins = {}        # pin name -> (x, y, rot)
        self.elements = []    # (tag, attrs, text)

    def wire(self, x1, y1, x2, y2):
        self.elements.append(("wire", dict(x1=f(x1), y1=f(y1), x2=f(x2), y2=f(y2),
                                           width="0.254", layer="94"), None))

    def circle(self, x, y, r):
        self.elements.append(("circle", dict(x=f(x), y=f(y), radius=f(r),
                                             width="0.254", layer="94"), None))

    def text(self, x, y, value, layer="94", size=1.778):
        self.elements.append(("text", dict(x=f(x), y=f(y), size=f(size), layer=layer), value))

    def pin(self, name, x, y, rot, length="short", visible="off"):
        self.pins[name] = (x, y, rot)
        attrs = dict(name=name, x=f(x), y=f(y), visible=visible, length=length, direction="pas")
        if rot != "R0":
            attrs["rot"] = rot
        self.elements.append(("pin", attrs, None))

    def box(self, x1, y1, x2, y2):
        self.wire(x1, y1, x2, y1)
        self.wire(x2, y1, x2, y2)
        self.wire(x2, y2, x1, y2)
        self.wire(x1, y2, x1, y1)

    def name_value(self, x, y_name, y_value):
        self.text(x, y_name, ">NAME", layer="95")
        self.text(x, y_value, ">VALUE", layer="96")


def two_pin(name, pin1, pin2, draw):
    s = Symbol(name)
    draw(s)
    s.pin(pin1, -2 * G, 0, "R0")
    s.pin(pin2, 2 * G, 0, "R180")
    s.name_value(-G, 2.2, -4.0)
    return s


def draw_resistor(s):
    s.box(-G, -1.016, G, 1.016)


def draw_capacitor(s):
    s.wire(-G, 0, -0.635, 0)
    s.wire(0.635, 0, G, 0)
    s.wire(-0.635, -1.778, -0.635, 1.778)
    s.wire(0.635, -1.778, 0.635, 1.778)


def draw_polarised(s):
    draw_capacitor(s)
    s.text(-2.4, 0.6, "+", size=1.27)


def draw_diode(s):
    s.wire(-G, 0, -1.27, 0)
    s.wire(1.27, 0, G, 0)
    s.wire(-1.27, 1.27, -1.27, -1.27)
    s.wire(-1.27, -1.27, 1.27, 0)
    s.wire(1.27, 0, -1.27, 1.27)
    s.wire(1.27, 1.27, 1.27, -1.27)


def draw_tvs(s):
    s.wire(-G, 0, -1.778, 0)
    s.wire(1.778, 0, G, 0)
    s.wire(-1.778, 1.27, -1.778, -1.27)
    s.wire(-1.778, -1.27, 0, 0)
    s.wire(0, 0, -1.778, 1.27)
    s.wire(1.778, 1.27, 1.778, -1.27)
    s.wire(1.778, -1.27, 0, 0)
    s.wire(0, 0, 1.778, 1.27)
    s.wire(0, 1.524, 0, -1.524)


def draw_fuse(s):
    s.box(-G, -0.889, G, 0.889)
    s.wire(-G, 0, G, 0)


def draw_battery(s):
    s.wire(-G, 0, -0.635, 0)
    s.wire(0.635, 0, G, 0)
    s.wire(-0.635, -2.286, -0.635, 2.286)
    s.wire(0.635, -1.016, 0.635, 1.016)
    s.text(-2.4, 0.9, "+", size=1.27)


def draw_jumper(s):
    s.circle(-1.27, 0, 0.635)
    s.circle(1.27, 0, 0.635)
    s.wire(-G, 0, -1.905, 0)
    s.wire(1.905, 0, G, 0)


def box_symbol(name, left, right, width, pitch=PIN_PITCH):
    """A rectangle with named pins down the left and right sides."""
    s = Symbol(name)
    rows = max(len(left), len(right), 1)
    top = (rows - 1) / 2 * pitch
    half_w = width / 2
    edge = top + pitch / 2 + G / 2
    s.box(-half_w, edge, half_w, -edge)
    for i, pin in enumerate(left):
        if pin:
            s.pin(pin, -half_w - 2 * G, top - i * pitch, "R0", length="middle", visible="pin")
    for i, pin in enumerate(right):
        if pin:
            s.pin(pin, half_w + 2 * G, top - i * pitch, "R180", length="middle", visible="pin")
    s.name_value(-half_w, edge + 1.0, -edge - 2.8)
    return s


def test_point():
    s = Symbol("TESTPOINT")
    s.circle(G, 0, 0.9)
    s.wire(0, 0, G - 0.9, 0)
    s.pin("TP", -G, 0, "R0")
    s.name_value(0, 1.6, -3.4)
    return s


# --------------------------------------------------------------- packages

class Package:
    def __init__(self, name, description):
        self.name = name
        self.description = description
        self.elements = []

    def smd(self, name, x, y, dx, dy, round_=False):
        attrs = dict(name=name, x=f(x), y=f(y), dx=f(dx), dy=f(dy), layer="1")
        if round_:
            attrs["roundness"] = "100"
        self.elements.append(("smd", attrs, None))

    def pad(self, name, x, y, drill=1.0, diameter=1.8, square=False):
        attrs = dict(name=name, x=f(x), y=f(y), drill=f(drill), diameter=f(diameter))
        if square:
            attrs["shape"] = "square"
        self.elements.append(("pad", attrs, None))

    def outline(self, x1, y1, x2, y2):
        for a, b, c, d in ((x1, y1, x2, y1), (x2, y1, x2, y2), (x2, y2, x1, y2), (x1, y2, x1, y1)):
            self.elements.append(("wire", dict(x1=f(a), y1=f(b), x2=f(c), y2=f(d),
                                               width="0.127", layer="21"), None))

    def labels(self, x, y):
        self.elements.append(("text", dict(x=f(x), y=f(y), size="1.016", layer="25"), ">NAME"))


def chip(name, dx, half_pitch, dy, description):
    p = Package(name, description)
    p.smd("1", -half_pitch, 0, dx, dy)
    p.smd("2", half_pitch, 0, dx, dy)
    p.labels(-half_pitch, dy / 2 + 0.4)
    return p


def build_packages():
    generic = "Generic land pattern. Replace with the LCSC part's own footprint before layout."
    packages = [
        chip("0805", 1.0, 0.95, 1.3, generic),
        chip("1206", 1.2, 1.5, 1.8, generic),
        chip("1812", 1.6, 2.05, 3.5, generic),
        chip("SMA", 2.2, 2.0, 1.7, generic + " Pad 1 is the cathode."),
        chip("SMB", 2.3, 2.15, 2.3, generic),
        chip("CAP-SMD-8X10", 3.4, 3.2, 1.6, generic + " Pad 1 is positive."),
    ]

    sot = Package("SOT23", generic)
    sot.smd("1", -0.95, -1.1, 0.9, 1.0)
    sot.smd("2", 0.95, -1.1, 0.9, 1.0)
    sot.smd("3", 0, 1.1, 0.9, 1.0)
    sot.labels(-1.4, 2.0)
    packages.append(sot)

    soic = Package("SOIC8", generic)
    for i in range(4):
        soic.smd(str(i + 1), -2.7, 1.905 - i * 1.27, 1.55, 0.6)
        soic.smd(str(8 - i), 2.7, 1.905 - i * 1.27, 1.55, 0.6)
    soic.outline(-1.95, 2.45, 1.95, -2.45)
    soic.labels(-1.95, 2.9)
    packages.append(soic)

    soic16 = Package("SOIC16W", generic + " Wide body, 7.5 mm.")
    for i in range(8):
        soic16.smd(str(i + 1), -4.7, 4.445 - i * 1.27, 2.0, 0.6)
        soic16.smd(str(16 - i), 4.7, 4.445 - i * 1.27, 2.0, 0.6)
    soic16.outline(-3.75, 5.2, 3.75, -5.2)
    soic16.labels(-3.75, 5.7)
    packages.append(soic16)

    for count in (2, 3, 5, 8):
        hdr = Package(f"HDR-1X{count}", "Pin header, 2.54 mm pitch, 1.0 mm drill.")
        for i in range(count):
            hdr.pad(str(i + 1), (i - (count - 1) / 2) * G, 0, square=(i == 0))
        hdr.outline(-count * G / 2, 1.27, count * G / 2, -1.27)
        hdr.labels(-count * G / 2, 1.7)
        packages.append(hdr)

    # From Molex sales drawing SD-43045-005 (PCB layout, component side): four
    # 1.02 mm holes on a 3.00 mm square, circuits 1 and 2 along one row with 3
    # and 4 opposite them, and two polarising peg holes 9.00 mm apart, 0.94 mm
    # outside the circuit 3-4 row. Body 9.65 x 7.37 mm.
    mf = Package("MOLEX-43045-0412",
                 "Molex Micro-Fit 3.0 vertical header, 2x2, from sales drawing SD-43045-005. "
                 "Check against the current Molex drawing before ordering boards.")
    for name, x, y in (("1", 1.5, -1.5), ("2", -1.5, -1.5), ("3", 1.5, 1.5), ("4", -1.5, 1.5)):
        mf.pad(name, x, y, drill=1.02, diameter=1.9, square=(name == "1"))
    for x in (-4.5, 4.5):
        mf.elements.append(("hole", dict(x=f(x), y=f(2.44), drill="1.02"), None))
    mf.outline(-4.825, 3.685, 4.825, -3.685)
    mf.labels(-4.825, 4.2)
    packages.append(mf)

    tp = Package("TESTPAD", "1.5 mm round test pad.")
    tp.smd("1", 0, 0, 1.5, 1.5, round_=True)
    tp.labels(-1.0, 1.2)
    packages.append(tp)

    # Measured from Espressif's esp32-c6-devkitc-1 dimension drawing (v1.2):
    # two rows of 16 at 2.54 mm, rows 22.86 mm apart, pin 1 at the antenna end.
    dev = Package("ESP32-C6-DEVKITC-1",
                  "Top view, USB ports at the bottom. Board outline 25.4 x 51.8 mm; the antenna "
                  "overhangs the top edge by about 6.35 mm: keep copper clear of it.")
    for i in range(16):
        y = 19.05 - i * G
        dev.pad(f"J1_{i + 1}", -11.43, y, square=(i == 0))
        dev.pad(f"J3_{i + 1}", 11.43, y, square=(i == 0))
    dev.outline(-12.7, 20.625, 12.7, -31.175)
    dev.labels(-12.7, 21.2)
    packages.append(dev)

    # From Olimex's KiCad board file, ESP32-P4-DevKit rev C: EXT1 and EXT2 are 1x20
    # at 2.54 mm, 25.40 mm apart, pin 1 of each at the USB-C end. The board is
    # 30 x 72 mm with 3.3 mm holes on a 23 x 65 mm pattern; the header centre is
    # 1 mm towards the Ethernet end of the board centre.
    p4 = Package("OLIMEX-ESP32-P4-DEVKIT",
                 "Top view, USB-C at the bottom, Ethernet jack at the top (it overhangs the edge). "
                 "The DevKit carries parts on its underside: keep the carrier clear beneath it, "
                 "or seat it on sockets.")
    for i in range(20):
        y = -24.13 + i * G
        p4.pad(f"EXT1_{i + 1}", -12.7, y, square=(i == 0))
        p4.pad(f"EXT2_{i + 1}", 12.7, y, square=(i == 0))
    for x, y in ((-11.5, 31.5), (11.5, 31.5), (-11.5, -33.5), (11.5, -33.5)):
        p4.elements.append(("hole", dict(x=f(x), y=f(y), drill="3.3"), None))
    p4.outline(-15, 35, 15, -37)
    p4.labels(-15, 35.5)
    packages.append(p4)

    # Pad names follow the datasheet; the positions do not.
    mini = Package("ESP32-C6-MINI-1-PLACEHOLDER",
                   "PLACEHOLDER with the module's 53 pad names. Use the ESP32-C6-MINI-1 footprint "
                   "from the LCSC part (C5736265) or Espressif's library. 13.2 x 16.6 mm.")
    for i in range(12):
        mini.smd(str(i + 1), -6.2, 4.4 - i * 0.8, 0.8, 0.4)
        mini.smd(str(i + 13), -4.4 + i * 0.8, -7.9, 0.4, 0.8)
        mini.smd(str(i + 25), 6.2, -4.4 + i * 0.8, 0.8, 0.4)
        mini.smd(str(i + 37), 4.4 - i * 0.8, 7.9, 0.4, 0.8)
    for i in range(5):
        mini.smd(str(i + 49), -2.4 + i * 1.2, 0, 0.8, 0.8)
    mini.outline(-6.6, 8.3, 6.6, -8.3)
    mini.labels(-6.6, 8.8)
    packages.append(mini)

    cell = Package("CR2032-HOLDER-PLACEHOLDER",
                   "PLACEHOLDER. Replace with the footprint of the 2032 holder you buy.")
    cell.pad("POS", -10.25, 0, drill=1.2, diameter=2.4, square=True)
    cell.pad("NEG", 10.25, 0, drill=1.2, diameter=2.4)
    cell.outline(-11.5, 8, 11.5, -8)
    cell.labels(-11.5, 8.5)
    packages.append(cell)

    buck = Package("POLOLU-D36V28F-PLACEHOLDER",
                   "PLACEHOLDER. Replace with the hole pattern from Pololu's D36V28Fx drill guide.")
    for i, name in enumerate(("VIN", "GND1", "GND2", "VOUT", "EN")):
        buck.pad(name, (i - 2) * G, 0, square=(i == 0))
    buck.outline(-8.9, 10.2, 8.9, -10.2)
    buck.labels(-8.9, 10.7)
    packages.append(buck)
    return packages


# ------------------------------------------------------------- devicesets

DEVKIT_J1 = ["3V3", "RST", "IO4", "IO5", "IO6", "IO7", "IO0", "IO1", "IO8", "IO10", "IO11",
             "IO2", "IO3", "5V", "GND@1", "NC@1"]
DEVKIT_J3 = ["GND@2", "IO16_TX", "IO17_RX", "IO15", "IO23", "IO22", "IO21", "IO20", "IO19",
             "IO18", "IO9", "GND@3", "IO13_USB_DP", "IO12_USB_DM", "GND@4", "NC@2"]


# Olimex ESP32-P4-DevKit rev C headers, pin 1 first (from its KiCad files).
P4_EXT1 = ["3V3", "GND@1", "IO2_LED", "IO3_SD_DET", "IO4", "IO5", "IO6", "IO7_SDA", "IO8_SCL",
           "IO9", "IO10", "IO11", "IO12", "IO13", "IO14", "IO15", "IO16", "IO17", "IO18", "IO19"]
P4_EXT2 = ["5V", "GND@2", "IO54", "IO53", "IO48", "IO47", "IO46", "IO33", "IO32", "IO23", "IO22",
           "IO21", "IO20", "EN", "GND@3", "USB1_P", "USB1_N", "GND@4", "USB_DP", "USB_DN"]

# ESP32-C6-MINI-1 pads by number (datasheet, pin definitions).
C6_MINI_SIGNALS = {"3V3": 3, "EN": 8, "IO8": 22, "IO9": 23, "RXD0": 30, "TXD0": 31, "IO2": 5,
                   "IO18": 24, "IO19": 25, "IO20": 26, "IO21": 27, "IO22": 28, "IO23": 29,
                   "IO0": 12, "IO1": 13, "IO3": 6, "IO4": 9, "IO5": 10, "IO6": 15, "IO7": 16,
                   "IO12": 17, "IO13": 18, "IO14": 19, "IO15": 20}
C6_MINI_NC = [4, 7, 21, 32, 33, 34, 35]
C6_MINI_GND = [1, 2, 11, 14] + list(range(36, 54))


def build_library():
    """Returns (symbols, devicesets). A deviceset is (prefix, symbol, package, pin->pad)."""
    symbols = {}
    sets = {}

    def add(name, prefix, symbol, package, connects):
        symbols[symbol.name] = symbol
        sets[name] = (prefix, symbol.name, package, connects)

    resistor = two_pin("RESISTOR", "1", "2", draw_resistor)
    capacitor = two_pin("CAPACITOR", "1", "2", draw_capacitor)
    add("R0805", "R", resistor, "0805", {"1": "1", "2": "2"})
    add("R1206", "R", resistor, "1206", {"1": "1", "2": "2"})
    add("C0805", "C", capacitor, "0805", {"1": "1", "2": "2"})
    add("CPOL-8X10", "C", two_pin("CAPACITOR-POL", "+", "-", draw_polarised), "CAP-SMD-8X10",
        {"+": "1", "-": "2"})
    add("SCHOTTKY-SMA", "D", two_pin("DIODE", "A", "K", draw_diode), "SMA", {"K": "1", "A": "2"})
    add("TVS-SMB", "D", two_pin("TVS-BIDIR", "1", "2", draw_tvs), "SMB", {"1": "1", "2": "2"})
    add("POLYFUSE-1812", "F", two_pin("FUSE", "1", "2", draw_fuse), "1812", {"1": "1", "2": "2"})
    add("JUMPER-2", "JP", two_pin("JUMPER", "1", "2", draw_jumper), "HDR-1X2", {"1": "1", "2": "2"})
    add("TESTPOINT", "TP", test_point(), "TESTPAD", {"TP": "1"})

    add("PESD2CAN", "D", box_symbol("PESD2CAN", ["IO1", "IO2"], ["GND"], 4 * G), "SOT23",
        {"IO1": "1", "IO2": "2", "GND": "3"})
    add("SN65HVD230", "U",
        box_symbol("SN65HVD230", ["D", "R", "RS", "VREF", "VCC", "GND"], ["CANH", "CANL"], 8 * G),
        "SOIC8", {"D": "1", "GND": "2", "VCC": "3", "R": "4", "VREF": "5", "CANL": "6",
                  "CANH": "7", "RS": "8"})
    add("TJA1051T-3", "U",
        box_symbol("TJA1051T-3", ["TXD", "RXD", "S", "VIO", "VCC", "GND"], ["CANH", "CANL"], 8 * G),
        "SOIC8", {"TXD": "1", "GND": "2", "VCC": "3", "RXD": "4", "VIO": "5", "CANL": "6",
                  "CANH": "7", "S": "8"})

    add("MICROFIT-2X2", "J", box_symbol("MICROFIT-2X2", ["1", "2", "3", "4"], [], 4 * G),
        "MOLEX-43045-0412", {str(i): str(i) for i in range(1, 5)})
    gauge_pins = ["VBUS", "GND", "3V3", "UART_A", "UART_B", "POS6", "POS7", "POS8"]
    add("GAUGE-HEADER", "P", box_symbol("GAUGE-HEADER", [], gauge_pins, 6 * G), "HDR-1X8",
        {pin: str(i + 1) for i, pin in enumerate(gauge_pins)})

    connects = {pin: f"J1_{i + 1}" for i, pin in enumerate(DEVKIT_J1)}
    connects.update({pin: f"J3_{i + 1}" for i, pin in enumerate(DEVKIT_J3)})
    add("ESP32-C6-DEVKITC-1", "U", box_symbol("ESP32-C6-DEVKITC-1", DEVKIT_J1, DEVKIT_J3, 12 * G),
        "ESP32-C6-DEVKITC-1", connects)

    # The symbol reads like the board seen from above: Ethernet end (pin 20) at the top.
    connects = {pin: f"EXT1_{i + 1}" for i, pin in enumerate(P4_EXT1)}
    connects.update({pin: f"EXT2_{i + 1}" for i, pin in enumerate(P4_EXT2)})
    add("OLIMEX-ESP32-P4-DEVKIT", "U",
        box_symbol("OLIMEX-ESP32-P4-DEVKIT", P4_EXT1[::-1], P4_EXT2[::-1], 12 * G),
        "OLIMEX-ESP32-P4-DEVKIT", connects)

    # Every pad of the module is on the symbol, at half the usual pin spacing.
    connects = {name: str(pad) for name, pad in C6_MINI_SIGNALS.items()}
    connects.update({f"NC@{pad}": str(pad) for pad in C6_MINI_NC})
    connects.update({f"GND@{pad}": str(pad) for pad in C6_MINI_GND})
    left = list(C6_MINI_SIGNALS) + [f"NC@{pad}" for pad in C6_MINI_NC[:3]]
    right = [f"NC@{pad}" for pad in C6_MINI_NC[3:]] + [f"GND@{pad}" for pad in C6_MINI_GND]
    add("ESP32-C6-MINI-1", "U", box_symbol("ESP32-C6-MINI-1", left, right, 10 * G, pitch=G),
        "ESP32-C6-MINI-1-PLACEHOLDER", connects)

    # Pins 5 to 12 of the SO-16 have no function and must be grounded.
    rtc_pads = {"32KHZ": "1", "VCC": "2", "INT_SQW": "3", "RST": "4", "GND": "13", "VBAT": "14",
                "SDA": "15", "SCL": "16"}
    rtc_pads.update({f"NC@{n}": str(n) for n in range(5, 13)})
    add("DS3231SN", "U",
        box_symbol("DS3231SN", ["VCC", "VBAT", "SDA", "SCL", "INT_SQW", "RST", "32KHZ", "GND"],
                   [f"NC@{n}" for n in range(5, 13)], 10 * G),
        "SOIC16W", rtc_pads)
    add("BATTERY-2032", "BT", two_pin("BATTERY", "+", "-", draw_battery), "CR2032-HOLDER-PLACEHOLDER",
        {"+": "POS", "-": "NEG"})
    add("BAT54", "D", two_pin("DIODE", "A", "K", draw_diode), "SOT23", {"A": "1", "K": "3"})
    add("JUMPER-3", "JP", box_symbol("JUMPER-3", ["1", "2", "3"], [], 4 * G), "HDR-1X3",
        {str(i): str(i) for i in range(1, 4)})
    add("POWER-2", "J", box_symbol("POWER-2", ["+5V", "GND"], [], 6 * G), "HDR-1X2",
        {"+5V": "1", "GND": "2"})
    gps_pins = ["VCC", "GND", "TXD", "RXD", "PPS"]
    add("GPS-HEADER", "J", box_symbol("GPS-HEADER", gps_pins, [], 6 * G), "HDR-1X5",
        {pin: str(i + 1) for i, pin in enumerate(gps_pins)})

    add("POLOLU-D36V28F5", "U",
        box_symbol("BUCK-MODULE", ["VIN", "EN", "GND@1"], ["VOUT", "GND@2"], 8 * G),
        "POLOLU-D36V28F-PLACEHOLDER",
        {"VIN": "VIN", "EN": "EN", "GND@1": "GND1", "VOUT": "VOUT", "GND@2": "GND2"})
    return symbols, sets


# ------------------------------------------------------------- schematics

class Schematic:
    def __init__(self, title, notes):
        self.title = title
        self.notes = notes
        self.parts = []   # (ref, deviceset, value, x, y, attributes)
        self.nets = {}    # net name -> [(ref, pin)]

    def part(self, ref, deviceset, value, x, y, **attributes):
        self.parts.append((ref, deviceset, value, x * G, y * G, attributes))

    def net(self, name, *pins):
        for pin in pins:
            ref, pin_name = pin.split(".", 1)
            self.nets.setdefault(name, []).append((ref, pin_name))


def gauge_adapter():
    s = Schematic(
        "Haltech gauges: gauge adapter (one per gauge)",
        ["J1 and J2 are wired pin for pin; the chain passes straight through.",
         "P1 plugs into the gauge's rear 1x8 socket. POS6/POS7/POS8 carry GPIO16/17/18 in an",
         "order confirmed at bring-up; the firmware maps CAN TX/RX to whichever land on 6 and 7.",
         "No termination here: the last adapter's J2 takes a 120R terminator plug.",
         "Footprints are generic. Assign each part its LCSC number and use that footprint."])
    s.part("JP1", "JUMPER-2", "CHAIN PWR", 18, 44)
    s.part("D1", "SCHOTTKY-SMA", "SS14", 36, 44, LCSC="C2480")
    s.part("R1", "R0805", "10k", 54, 44, LCSC="C17414")
    s.part("C1", "C0805", "100n", 72, 44, LCSC="C49678")
    s.part("P1", "GAUGE-HEADER", "1x8 male 2.54", 14, 26)
    s.part("U1", "SN65HVD230", "SN65HVD230DR", 44, 26, LCSC="C12084")
    s.part("D2", "PESD2CAN", "PESD2CAN", 44, 8, LCSC="C75176")
    s.part("TP1", "TESTPOINT", "SPARE", 20, 8)
    s.part("J1", "MICROFIT-2X2", "43045-0412 IN", 76, 32)
    s.part("J2", "MICROFIT-2X2", "43045-0412 OUT", 76, 16)

    s.net("+5V_CHAIN", "J1.1", "J2.1", "D1.A")
    s.net("VBUS_D", "D1.K", "JP1.1")
    s.net("VBUS", "JP1.2", "P1.VBUS")
    s.net("GND", "J1.2", "J2.2", "P1.GND", "U1.GND", "U1.RS", "C1.2", "D2.GND")
    s.net("+3V3", "P1.3V3", "U1.VCC", "C1.1", "R1.1")
    s.net("CAN_TX", "P1.POS6", "U1.D", "R1.2")
    s.net("CAN_RX", "P1.POS7", "U1.R")
    s.net("SPARE", "P1.POS8", "TP1.TP")
    s.net("CANH", "J1.3", "J2.3", "U1.CANH", "D2.IO1")
    s.net("CANL", "J1.4", "J2.4", "U1.CANL", "D2.IO2")
    return s


def hub_carrier():
    s = Schematic(
        "Haltech gauges: hub carrier",
        ["J10 is the ECU side: 1 +12V switched, 2 GND, 3 CAN H, 4 CAN L (from the DTM-4 loom,",
         "which carries the inline 2 A fuse). J11 is the gauge chain: 1 +5V, 2 GND, 3 CAN H, 4 CAN L.",
         "U4 is the ESP32-C6-DevKitC-1-N8, soldered in. U1 is the Pololu D36V28F5.",
         "JP1 fitted = 120R across the ECU bus (leave open if the Haltech bus is already terminated).",
         "R4-R7 and C9 are the gauge bus's split termination (two 120R in parallel per leg).",
         "Footprints are generic; the Pololu one is a placeholder."])

    # 12 V input and the 5 V rail
    s.part("J10", "MICROFIT-2X2", "43045-0412 ECU", 14, 74)
    s.part("D1", "TVS-SMB", "SMBJ26CA", 34, 74, LCSC="C515606")
    s.part("D2", "SCHOTTKY-SMA", "SS34", 54, 74, LCSC="C8678")
    s.part("C1", "CPOL-8X10", "100u 50V", 74, 74, LCSC="C116241")
    s.part("C2", "C0805", "1u 50V", 94, 74, LCSC="C28323")
    s.part("U1", "POLOLU-D36V28F5", "D36V28F5", 18, 60)
    s.part("C3", "CPOL-8X10", "100u 50V", 46, 62, LCSC="C116241")
    s.part("C4", "C0805", "10u 25V", 66, 62, LCSC="C15850")
    s.part("C5", "C0805", "10u 25V", 86, 62, LCSC="C15850")
    s.part("D3", "SCHOTTKY-SMA", "SS14", 46, 56, LCSC="C2480")
    s.part("F2", "POLYFUSE-1812", "1812L200/16DR", 66, 56, LCSC="C439873")
    s.part("J11", "MICROFIT-2X2", "43045-0412 CHAIN", 110, 60)

    # MCU and transceivers
    s.part("U4", "ESP32-C6-DEVKITC-1", "ESP32-C6-DevKitC-1-N8", 24, 30)
    s.part("U2", "TJA1051T-3", "TJA1051T/3", 58, 40, LCSC="C38695")
    s.part("U3", "SN65HVD230", "SN65HVD230DR", 58, 22, LCSC="C12084")
    s.part("D4", "PESD2CAN", "PESD2CAN", 86, 44, LCSC="C75176")
    s.part("R3", "R1206", "120R", 86, 37, LCSC="C17909")
    s.part("JP1", "JUMPER-2", "ECU TERM", 106, 37)
    s.part("D5", "PESD2CAN", "PESD2CAN", 86, 28, LCSC="C75176")
    s.part("R4", "R1206", "120R", 86, 21, LCSC="C17909")
    s.part("R5", "R1206", "120R", 106, 21, LCSC="C17909")
    s.part("R6", "R1206", "120R", 86, 16, LCSC="C17909")
    s.part("R7", "R1206", "120R", 106, 16, LCSC="C17909")
    s.part("C9", "C0805", "4n7", 86, 11, LCSC="C1744")

    # Decoupling, pulls and the 12 V sense divider
    s.part("C6", "C0805", "100n", 12, 8, LCSC="C49678")
    s.part("C7", "C0805", "100n", 34, 8, LCSC="C49678")
    s.part("C8", "C0805", "100n", 56, 8, LCSC="C49678")
    s.part("C10", "C0805", "100n", 78, 6, LCSC="C49678")
    s.part("R1", "R0805", "10k", 12, 3, LCSC="C17414")
    s.part("R2", "R0805", "10k", 34, 3, LCSC="C17414")
    s.part("R8", "R0805", "100k", 56, 3, LCSC="C149504")
    s.part("R9", "R0805", "10k", 78, 1, LCSC="C17414")

    for i, name in enumerate(("GND", "+5V", "+3V3", "VBAT", "RST")):
        s.part(f"TP{i + 1}", "TESTPOINT", name, 132, 74 - 4 * i)

    s.net("+12V_IN", "J10.1", "D1.1", "D2.A", "R8.1")
    s.net("+12V_PROT", "D2.K", "C1.+", "C2.1", "U1.VIN")
    s.net("+5V", "U1.VOUT", "C3.+", "C4.1", "C5.1", "D3.A", "F2.1", "U2.VCC", "C6.1", "TP2.TP")
    s.net("+5V_MCU", "D3.K", "U4.5V")
    s.net("+5V_CHAIN", "F2.2", "J11.1")
    s.net("+3V3", "U4.3V3", "U2.VIO", "C7.1", "U3.VCC", "C8.1", "R2.1", "TP3.TP")
    s.net("GND", "J10.2", "D1.2", "C1.-", "C2.2", "U1.GND@1", "U1.GND@2", "C3.-", "C4.2", "C5.2",
          "J11.2", "U4.GND@1", "U4.GND@2", "U4.GND@3", "U4.GND@4", "U2.GND", "U3.GND", "U3.RS",
          "D4.GND", "D5.GND", "C9.2", "C6.2", "C7.2", "C8.2", "C10.2", "R1.2", "R9.2", "TP1.TP")
    s.net("ECU_TX", "U4.IO18", "U2.TXD")
    s.net("ECU_RX", "U4.IO19", "U2.RXD")
    s.net("ECU_SILENT", "U4.IO23", "U2.S", "R1.1")
    s.net("GAUGE_TX", "U4.IO20", "U3.D", "R2.2")
    s.net("GAUGE_RX", "U4.IO21", "U3.R")
    s.net("VBAT_SENSE", "U4.IO2", "R8.2", "R9.1", "C10.1", "TP4.TP")
    s.net("RST", "U4.RST", "TP5.TP")
    s.net("ECU_CANH", "J10.3", "U2.CANH", "D4.IO1", "R3.1")
    s.net("ECU_CANL", "J10.4", "U2.CANL", "D4.IO2", "JP1.2")
    s.net("ECU_TERM", "R3.2", "JP1.1")
    s.net("CANH", "J11.3", "U3.CANH", "D5.IO1", "R4.1", "R5.1")
    s.net("CANL", "J11.4", "U3.CANL", "D5.IO2", "R6.2", "R7.2")
    s.net("TERM_MID", "R4.2", "R5.2", "R6.1", "R7.1", "C9.1")
    return s


def hub_carrier_p4():
    """The hub on an Olimex ESP32-P4-DevKit: the C6 carrier's power and CAN circuits,
    plus a Wi-Fi module, a real-time clock and an optional GPS header."""
    s = Schematic(
        "Haltech gauges: hub carrier, ESP32-P4 variant (Olimex ESP32-P4-DevKit)",
        ["J10 is the ECU side: 1 +12V switched, 2 GND, 3 CAN H, 4 CAN L. J11 is the gauge chain: 1 +5V, 2 GND, 3 CAN H, 4 CAN L.",
         "U4 is the Olimex ESP32-P4-DevKit (rev C), pins down, USB-C end at EXT pin 1. It takes its 3.3 V rail to the carrier on EXT1-1.",
         "J13 feeds the DevKit: wire J13-1 to POE_PWR1 pin 4 (+5VP) and J13-2 to pin 3 (GND). Leave POE_PWR1 pins 1 and 2 open.",
         "Do not feed the DevKit through EXT2-1 (+5V): with USB plugged in, that pin back-feeds the PC.",
         "U6 is the Wi-Fi radio (ESP-Hosted over SDIO, the pins Espressif's own P4 board uses). Antenna at a board edge, no copper under it.",
         "U5 is the clock. BT1 is a CR2032; fit JP2 only with a rechargeable ML2032, never with a CR2032.",
         "J12 is the optional GPS header: 1 VCC (JP3 picks 3.3 V or 5 V), 2 GND, 3 TXD from the module, 4 RXD to it, 5 PPS.",
         "The DevKit already has 2.2 k pull-ups on SDA and SCL (GPIO7, GPIO8). Footprints are generic or placeholders."])

    # 12 V input and the 5 V rail: as on the C6 carrier, less its D3 (the DevKit has its own diode).
    s.part("J10", "MICROFIT-2X2", "43045-0412 ECU", 14, 150)
    s.part("D1", "TVS-SMB", "SMBJ26CA", 34, 150, LCSC="C515606")
    s.part("D2", "SCHOTTKY-SMA", "SS34", 54, 150, LCSC="C8678")
    s.part("C1", "CPOL-8X10", "100u 50V", 74, 150, LCSC="C116241")
    s.part("C2", "C0805", "1u 50V", 94, 150, LCSC="C28323")
    s.part("U1", "POLOLU-D36V28F5", "D36V28F5", 18, 136)
    s.part("C3", "CPOL-8X10", "100u 50V", 46, 138, LCSC="C116241")
    s.part("C4", "C0805", "10u 25V", 66, 138, LCSC="C15850")
    s.part("C5", "C0805", "10u 25V", 86, 138, LCSC="C15850")
    s.part("J13", "POWER-2", "TO DEVKIT POE_PWR1", 48, 130)
    s.part("F2", "POLYFUSE-1812", "1812L200/16DR", 70, 131, LCSC="C439873")
    s.part("J11", "MICROFIT-2X2", "43045-0412 CHAIN", 110, 136)

    # MCU and the two CAN transceivers
    s.part("U4", "OLIMEX-ESP32-P4-DEVKIT", "ESP32-P4-DevKit", 30, 100)
    s.part("U2", "TJA1051T-3", "TJA1051T/3", 72, 112, LCSC="C38695")
    s.part("U3", "SN65HVD230", "SN65HVD230DR", 72, 94, LCSC="C12084")
    s.part("D4", "PESD2CAN", "PESD2CAN", 100, 116, LCSC="C75176")
    s.part("R3", "R1206", "120R", 100, 109, LCSC="C17909")
    s.part("JP1", "JUMPER-2", "ECU TERM", 120, 109)
    s.part("D5", "PESD2CAN", "PESD2CAN", 100, 100, LCSC="C75176")
    s.part("R4", "R1206", "120R", 100, 93, LCSC="C17909")
    s.part("R5", "R1206", "120R", 120, 93, LCSC="C17909")
    s.part("R6", "R1206", "120R", 100, 88, LCSC="C17909")
    s.part("R7", "R1206", "120R", 120, 88, LCSC="C17909")
    s.part("C9", "C0805", "4n7", 100, 83, LCSC="C1744")

    # GPS header (optional)
    s.part("J12", "GPS-HEADER", "GPS 1x5 2.54", 152, 114)
    s.part("JP3", "JUMPER-3", "GPS 3V3/5V", 152, 101)
    s.part("R22", "R0805", "1k", 150, 92, LCSC="C17513")
    s.part("R23", "R0805", "1k", 150, 87, LCSC="C17513")
    s.part("C16", "C0805", "10u 25V", 150, 82, LCSC="C15850")

    # Wi-Fi module and what it needs
    s.part("U6", "ESP32-C6-MINI-1", "ESP32-C6-MINI-1-N4", 30, 50, LCSC="C5736265")
    s.part("R10", "R0805", "10k", 62, 62, LCSC="C17414")
    s.part("C13", "C0805", "100n", 84, 62, LCSC="C49678")
    s.part("C11", "C0805", "10u 25V", 106, 62, LCSC="C15850")
    s.part("C12", "C0805", "100n", 128, 62, LCSC="C49678")
    s.part("R11", "R0805", "10k", 62, 56, LCSC="C17414")
    s.part("R12", "R0805", "10k", 84, 56, LCSC="C17414")
    s.part("R13", "R0805", "10k", 106, 56, LCSC="C17414")
    s.part("R14", "R0805", "10k", 128, 56, LCSC="C17414")
    s.part("R15", "R0805", "10k", 62, 50, LCSC="C17414")
    s.part("R16", "R0805", "10k", 84, 50, LCSC="C17414")
    s.part("R17", "R0805", "10k", 106, 50, LCSC="C17414")
    for i, name in enumerate(("C6 EN", "C6 TXD", "C6 RXD", "C6 BOOT")):
        s.part(f"TP{i + 6}", "TESTPOINT", name, 64 + 22 * i, 44)

    # Decoupling, pulls and the 12 V sense divider
    s.part("C6", "C0805", "100n", 62, 38, LCSC="C49678")
    s.part("C7", "C0805", "100n", 84, 38, LCSC="C49678")
    s.part("C8", "C0805", "100n", 106, 38, LCSC="C49678")
    s.part("C10", "C0805", "100n", 128, 38, LCSC="C49678")
    s.part("R1", "R0805", "10k", 62, 33, LCSC="C17414")
    s.part("R2", "R0805", "10k", 84, 33, LCSC="C17414")
    s.part("R8", "R0805", "100k", 106, 33, LCSC="C149504")
    s.part("R9", "R0805", "10k", 128, 33, LCSC="C17414")

    # Real-time clock
    s.part("U5", "DS3231SN", "DS3231SN#", 30, 16, LCSC="C9866")
    s.part("C14", "C0805", "100n", 62, 26, LCSC="C49678")
    s.part("R20", "R0805", "10k", 84, 26, LCSC="C17414")
    s.part("BT1", "BATTERY-2032", "CR2032 holder", 62, 19)
    s.part("C15", "C0805", "100n", 84, 19, LCSC="C49678")
    s.part("D6", "BAT54", "BAT54", 62, 12, LCSC="C8590")
    s.part("R21", "R0805", "1k", 84, 12, LCSC="C17513")
    s.part("JP2", "JUMPER-2", "ML2032 CHARGE", 106, 12)

    for i, name in enumerate(("GND", "+5V", "+3V3", "VBAT", "RST")):
        s.part(f"TP{i + 1}", "TESTPOINT", name, 152, 150 - 4 * i)

    c6_grounds = [f"U6.GND@{n}" for n in C6_MINI_GND]
    rtc_unused = [f"U5.NC@{n}" for n in range(5, 13)]  # the datasheet wants these on ground

    s.net("+12V_IN", "J10.1", "D1.1", "D2.A", "R8.1")
    s.net("+12V_PROT", "D2.K", "C1.+", "C2.1", "U1.VIN")
    s.net("+5V", "U1.VOUT", "C3.+", "C4.1", "C5.1", "F2.1", "J13.+5V", "U2.VCC", "C6.1", "JP3.3", "TP2.TP")
    s.net("+5V_CHAIN", "F2.2", "J11.1")
    s.net("+3V3", "U4.3V3", "U2.VIO", "C7.1", "U3.VCC", "C8.1", "R2.1", "TP3.TP",
          "U6.3V3", "C11.1", "C12.1", "R10.1", "R11.1", "R12.1", "R13.1", "R14.1", "R15.1", "R16.1", "R17.1",
          "U5.VCC", "C14.1", "R20.1", "D6.A", "JP3.1")
    s.net("GND", "J10.2", "D1.2", "C1.-", "C2.2", "U1.GND@1", "U1.GND@2", "C3.-", "C4.2", "C5.2",
          "J11.2", "J13.GND", "U4.GND@1", "U4.GND@2", "U4.GND@3", "U4.GND@4", "U2.GND", "U3.GND", "U3.RS",
          "D4.GND", "D5.GND", "C9.2", "C6.2", "C7.2", "C8.2", "C10.2", "R1.2", "R9.2", "TP1.TP",
          *c6_grounds, "C11.2", "C12.2", "C13.2", "U5.GND", *rtc_unused, "C14.2", "BT1.-", "C15.2",
          "J12.GND", "C16.2")

    # CAN and the 12 V sense, on pins of the P4's always-3.3 V domains
    s.net("ECU_TX", "U4.IO21", "U2.TXD")
    s.net("ECU_RX", "U4.IO22", "U2.RXD")
    s.net("ECU_SILENT", "U4.IO23", "U2.S", "R1.1")
    s.net("GAUGE_TX", "U4.IO32", "U3.D", "R2.2")
    s.net("GAUGE_RX", "U4.IO33", "U3.R")
    s.net("VBAT_SENSE", "U4.IO20", "R8.2", "R9.1", "C10.1", "TP4.TP")
    s.net("RST", "U4.EN", "TP5.TP")
    s.net("ECU_CANH", "J10.3", "U2.CANH", "D4.IO1", "R3.1")
    s.net("ECU_CANL", "J10.4", "U2.CANL", "D4.IO2", "JP1.2")
    s.net("ECU_TERM", "R3.2", "JP1.1")
    s.net("CANH", "J11.3", "U3.CANH", "D5.IO1", "R4.1", "R5.1")
    s.net("CANL", "J11.4", "U3.CANL", "D5.IO2", "R6.2", "R7.2")
    s.net("TERM_MID", "R4.2", "R5.2", "R6.1", "R7.1", "C9.1")

    # Wi-Fi: SDIO to the module, with the pull-ups SDIO needs
    s.net("SDIO_CLK", "U4.IO18", "U6.IO19")
    s.net("SDIO_CMD", "U4.IO19", "U6.IO18", "R11.2")
    s.net("SDIO_D0", "U4.IO14", "U6.IO20", "R12.2")
    s.net("SDIO_D1", "U4.IO15", "U6.IO21", "R13.2")
    s.net("SDIO_D2", "U4.IO16", "U6.IO22", "R14.2")
    s.net("SDIO_D3", "U4.IO17", "U6.IO23", "R15.2")
    s.net("C6_EN", "U4.IO54", "U6.EN", "R10.2", "C13.1", "TP6.TP")
    s.net("C6_WAKE", "U4.IO6", "U6.IO2")
    # The module's serial port and boot pin, so the P4 can load its firmware.
    s.net("C6_TXD", "U6.TXD0", "U4.IO5", "TP7.TP")
    s.net("C6_RXD", "U6.RXD0", "U4.IO4", "TP8.TP")
    s.net("C6_BOOT", "U6.IO9", "U4.IO53", "R16.2", "TP9.TP")
    s.net("C6_IO8", "U6.IO8", "R17.2")

    # Clock: I2C on the DevKit's own bus, a 1 Hz tick, and the cell
    s.net("I2C_SDA", "U4.IO7_SDA", "U5.SDA")
    s.net("I2C_SCL", "U4.IO8_SCL", "U5.SCL")
    s.net("RTC_INT", "U4.IO9", "U5.INT_SQW", "R20.2")
    s.net("RTC_VBAT", "U5.VBAT", "BT1.+", "C15.1", "JP2.2")
    s.net("RTC_CHG_A", "D6.K", "R21.1")
    s.net("RTC_CHG_B", "R21.2", "JP2.1")

    # GPS: 1 k in each line coming from the module
    s.net("GPS_VCC", "JP3.2", "J12.VCC", "C16.1")
    s.net("GPS_TXD", "J12.TXD", "R22.1")
    s.net("GPS_RX", "R22.2", "U4.IO10")
    s.net("GPS_RXD", "J12.RXD", "U4.IO11")
    s.net("GPS_PPS", "J12.PPS", "R23.1")
    s.net("GPS_PPS_IN", "R23.2", "U4.IO12")
    return s


# ------------------------------------------------------------------ output

LAYERS = [
    (1, "Top", 4), (16, "Bottom", 1), (17, "Pads", 2), (18, "Vias", 2), (19, "Unrouted", 6),
    (20, "Dimension", 15), (21, "tPlace", 7), (22, "bPlace", 7), (25, "tNames", 7),
    (26, "bNames", 7), (27, "tValues", 7), (28, "bValues", 7), (29, "tStop", 7), (30, "bStop", 7),
    (31, "tCream", 7), (32, "bCream", 7), (39, "tKeepout", 4), (40, "bKeepout", 1),
    (41, "tRestrict", 4), (42, "bRestrict", 1), (43, "vRestrict", 2), (44, "Drills", 7),
    (45, "Holes", 7), (46, "Milling", 3), (47, "Measures", 7), (48, "Document", 7),
    (49, "Reference", 7), (51, "tDocu", 7), (52, "bDocu", 7), (91, "Nets", 2), (92, "Busses", 1),
    (93, "Pins", 2), (94, "Symbols", 4), (95, "Names", 7), (96, "Values", 7), (97, "Info", 7),
    (98, "Guide", 6),
]


def emit(parent, elements):
    for tag, attrs, text in elements:
        e = ET.SubElement(parent, tag, attrs)
        if text is not None:
            e.text = text


def build_xml(sch, symbols, sets, packages):
    used_sets = {deviceset for _, deviceset, *_ in sch.parts}
    used_symbols = {sets[d][1] for d in used_sets}
    used_packages = {sets[d][2] for d in used_sets}

    eagle = ET.Element("eagle", version="7.7.0")
    drawing = ET.SubElement(eagle, "drawing")
    settings = ET.SubElement(drawing, "settings")
    ET.SubElement(settings, "setting", alwaysvectorfont="no")
    ET.SubElement(settings, "setting", verticaltext="up")
    ET.SubElement(drawing, "grid", distance="0.1", unitdist="inch", unit="inch", style="lines",
                  multiple="1", display="no", altdistance="0.01", altunitdist="inch", altunit="inch")
    layers = ET.SubElement(drawing, "layers")
    for number, name, color in LAYERS:
        schematic_layer = number >= 91
        ET.SubElement(layers, "layer", number=str(number), name=name, color=str(color), fill="1",
                      visible="yes" if schematic_layer and number != 93 else "no",
                      active="yes" if schematic_layer else "no")

    schematic = ET.SubElement(drawing, "schematic", xreflabel="%F%N/%S.%C%R", xrefpart="/%S.%C%R")
    libraries = ET.SubElement(schematic, "libraries")
    library = ET.SubElement(libraries, "library", name="haltech-gauges")
    packages_el = ET.SubElement(library, "packages")
    for package in packages:
        if package.name in used_packages:
            p = ET.SubElement(packages_el, "package", name=package.name)
            ET.SubElement(p, "description").text = package.description
            emit(p, package.elements)
    symbols_el = ET.SubElement(library, "symbols")
    for name in sorted(used_symbols):
        emit(ET.SubElement(symbols_el, "symbol", name=name), symbols[name].elements)
    sets_el = ET.SubElement(library, "devicesets")
    for name in sorted(used_sets):
        prefix, symbol, package, connects = sets[name]
        ds = ET.SubElement(sets_el, "deviceset", name=name, prefix=prefix, uservalue="yes")
        gates = ET.SubElement(ds, "gates")
        ET.SubElement(gates, "gate", name="G$1", symbol=symbol, x="0", y="0")
        devices = ET.SubElement(ds, "devices")
        device = ET.SubElement(devices, "device", name="", package=package)
        connects_el = ET.SubElement(device, "connects")
        for pin, pad in connects.items():
            ET.SubElement(connects_el, "connect", gate="G$1", pin=pin, pad=pad)
        technologies = ET.SubElement(device, "technologies")
        ET.SubElement(technologies, "technology", name="")

    ET.SubElement(schematic, "attributes")
    ET.SubElement(schematic, "variantdefs")
    classes = ET.SubElement(schematic, "classes")
    ET.SubElement(classes, "class", number="0", name="default", width="0", drill="0")

    parts_el = ET.SubElement(schematic, "parts")
    for ref, deviceset, value, _, _, attributes in sch.parts:
        part = ET.SubElement(parts_el, "part", name=ref, library="haltech-gauges",
                             deviceset=deviceset, device="", value=value)
        for key, val in attributes.items():
            ET.SubElement(part, "attribute", name=key, value=val)

    sheets = ET.SubElement(schematic, "sheets")
    sheet = ET.SubElement(sheets, "sheet")
    plain = ET.SubElement(sheet, "plain")
    top = max(y for *_, y, _ in sch.parts) + 12 * G
    ET.SubElement(plain, "text", x=f(4 * G), y=f(top + (len(sch.notes) + 1) * 3.0),
                  size="2.54", layer="97").text = sch.title
    for i, line in enumerate(sch.notes):
        ET.SubElement(plain, "text", x=f(4 * G), y=f(top + (len(sch.notes) - 1 - i) * 3.0),
                      size="1.778", layer="97").text = line

    instances = ET.SubElement(sheet, "instances")
    positions = {}
    for ref, deviceset, _, x, y, _ in sch.parts:
        ET.SubElement(instances, "instance", part=ref, gate="G$1", x=f(x), y=f(y))
        positions[ref] = (deviceset, x, y)
    ET.SubElement(sheet, "busses")

    nets = ET.SubElement(sheet, "nets")
    for name, pins in sch.nets.items():
        net = ET.SubElement(nets, "net", {"name": name, "class": "0"})
        for ref, pin in pins:
            deviceset, x, y = positions[ref]
            px, py, rot = symbols[sets[deviceset][1]].pins[pin]
            hot_x, hot_y = x + px, y + py
            # Left-hand pins (R0) get a wire to the left, right-hand pins (R180) to the right.
            end_x = hot_x - STUB if rot == "R0" else hot_x + STUB
            segment = ET.SubElement(net, "segment")
            ET.SubElement(segment, "pinref", part=ref, gate="G$1", pin=pin)
            ET.SubElement(segment, "wire", x1=f(hot_x), y1=f(hot_y), x2=f(end_x), y2=f(hot_y),
                          width="0.1524", layer="91")
            label = dict(x=f(end_x), y=f(hot_y), size="1.778", layer="95")
            if rot == "R0":
                label["rot"] = "R180"
            ET.SubElement(segment, "label", label)
    return eagle


def write(path, root):
    ET.indent(root)
    body = ET.tostring(root, encoding="unicode")
    with open(path, "w", encoding="utf-8", newline="\n") as out:
        out.write('<?xml version="1.0" encoding="utf-8"?>\n')
        out.write('<!DOCTYPE eagle SYSTEM "eagle.dtd">\n')
        out.write(body)
        out.write("\n")


def main():
    symbols, sets = build_library()
    packages = build_packages()
    out_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "hardware")
    os.makedirs(out_dir, exist_ok=True)
    for filename, schematic in (("gauge-adapter.sch", gauge_adapter()),
                                ("hub-carrier.sch", hub_carrier()),
                                ("hub-carrier-p4.sch", hub_carrier_p4())):
        path = os.path.join(out_dir, filename)
        write(path, build_xml(schematic, symbols, sets, packages))
        pins = sum(len(v) for v in schematic.nets.values())
        print(f"{filename}: {len(schematic.parts)} parts, {len(schematic.nets)} nets, {pins} pins")


if __name__ == "__main__":
    main()
