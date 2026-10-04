# Electrical spec: hub carrier and gauge adapter

Draft for layout. Two items wait on bench results and are marked **[bench]**:
the gauge header pin order (bring-up step 1) and the 5 V budget (steps 1 and 4).

## 1. Hub carrier board

A carrier that the ESP32-C6-DevKitC-1-N8 plugs into, carrying the two CAN
transceivers, the protected 12 V input, the connectors and the mounting holes.
The DevKit has no mounting holes of its own, so the carrier is what gets screwed
into the enclosure.

### 1.1 DevKit footprint

Measured from Espressif's dimension drawing (`esp32-c6-devkitc-1-dimensions_v1.2.dxf`)
and layout PDF. Top view, USB ports at the bottom, antenna at the top:

```
                antenna overhang, ~6.35 mm past the board edge
             +-------------------------------------------+   <- top edge
   J1 pin 1  o   3V3                              G   o  J3 pin 1     1.575 mm from top edge
             o   RST                             TX   o
             o   4                               RX   o
             o   5                               15   o
             o   6                               23   o
             o   7                               22   o
             o   0                               21   o  J3-7   GPIO21  gauge-bus RXD
             o   1                               20   o  J3-8   GPIO20  gauge-bus TXD
             o   8  (RGB LED, strapping)         19   o  J3-9   GPIO19  ECU-bus RXD
             o   10                              18   o  J3-10  GPIO18  ECU-bus TXD
             o   11                               9   o
   J1-12     o   2   12 V sense ADC               G   o
             o   3                               13   o  (USB D+)
   J1-14     o   5V  carrier 5 V in              12   o  (USB D-)
   J1-15     o   G                                G   o
   J1-16     o   NC                              NC   o  J3-16               12.125 mm from bottom edge
             +-----[USB-C]---------[USB-C]---------------+   <- bottom edge
                   centre 5.5 mm     centre 19.9 mm from the left edge
             |<-------------- 25.4 mm ----------------->|
```

| Item | Value |
|---|---|
| DevKit outline | 25.4 x 51.8 mm, 0.5 mm corner radius; antenna adds ~6.35 mm past the top edge |
| Headers | 2 rows x 16 positions, 2.54 mm pitch |
| Row spacing | 22.86 mm centre to centre (0.9 in); each row 1.27 mm in from its board edge |
| Pin 1 | Antenna end, both rows; J1 is the left row, J3 the right row (top view, USB down) |
| Carrier holes | 1.0 mm drill, 1.8 mm pad, 32 positions (Espressif's own pads are 1.98 x 1.35 mm oblong) |
| USB-C ports | Both on the bottom edge; bodies reach about 8 mm up the board. Keep the enclosure open at that edge so the UART port can be reached without unplugging anything |
| Antenna keep-out | No copper on any layer under or within 10 mm of the antenna; easiest is to let the DevKit's top edge hang past the carrier's edge |
| Retention | Solder the DevKit's male headers straight into the carrier. Female headers alone will walk out with vibration; if you want it removable, add a printed clamp bar over the DevKit's bottom (USB) end |
| Height above carrier | About 8 mm with the DevKit soldered down; about 15 mm on female headers |

### 1.2 DevKit pins used

| DevKit pin | GPIO | Carrier connection |
|---|---|---|
| J1-1 | 3V3 | Supply for the gauge-bus transceiver, and the logic-level (VIO) pin of the ECU transceiver |
| J1-12 | GPIO2 (ADC1_CH2) | 12 V sense divider |
| J1-14 | 5V | Carrier 5 V rail, through D3 |
| J1-15, J3-1, J3-12, J3-15 | GND | Ground |
| J3-10 | GPIO18 | ECU transceiver TXD |
| J3-9 | GPIO19 | ECU transceiver RXD |
| J3-8 | GPIO20 | Gauge transceiver TXD |
| J3-7 | GPIO21 | Gauge transceiver RXD |
| J3-5 | GPIO23 | ECU transceiver S (silent) pin, with a 10 k pull-down so it is in normal mode by default. Driven high, the hub keeps listening to the Haltech bus but can neither transmit nor acknowledge |
| J1-2 | RST | Test point only |
| J1-5 | GPIO6 | I2C SDA for the real-time clock (rev B on the board; rev A as a wired module, section 1.8) |
| J1-6 | GPIO7 | I2C SCL for the real-time clock |

Do not use GPIO4, 5, 8, 9, 15 (strapping), 12/13 (USB) or 16/17 (UART console).

### 1.3 Power input (from the Haltech DTM-4)

```
DTM-4 pin 1 (+12 V switched) --F1--+--D1--+--D2--+--C1--+--[ U1 buck 12 -> 5.15 V ]--+-- 5 V rail
                                   |      |             |                            |
                                  TVS   reverse        bulk                         C2
                                   |    block           |
DTM-4 pin 2 (GND) -----------------+------+-------------+----------------------------+-- GND
DTM-4 pin 3 (CAN H) --> ECU transceiver
DTM-4 pin 4 (CAN L) --> ECU transceiver
```

| Ref | Part | Notes |
|---|---|---|
| F1 | 2 A fuse, ATO/mini blade holder or 2 A slow PTC | Resettable is fine at this current |
| D1 | SMBJ26CA bidirectional TVS | Clamps load dump; does not conduct on a 24 V jump start |
| D2 | Reverse-blocking Schottky, >= 3 A, >= 60 V (e.g. SS34, or a P-FET ideal-diode circuit for lower drop) | |
| C1 | 100 uF, 50 V electrolytic + 1 uF ceramic | Buck input |
| U1 | 5 V, 3 A buck, input rated to at least 40 V, minimum input 6 V or lower, output trimmed to 5.15 - 5.2 V | Module options: Pololu D36V28F5 (5.3-50 V in, 3.2 A), or a TI TPS54360 / LMR33630 design. MP1584 (28 V) modules are not acceptable |
| C2 | 100 uF electrolytic + 10 uF ceramic | Buck output |
| D3 | Schottky, 1 A, from the 5 V rail to DevKit J1-14 | Stops a PC's USB feeding the carrier; the DevKit's own LDO makes 3.3 V |
| F2 | 2 A-hold polyfuse, 1812 (Littelfuse 1812L200/16DR, or /12DR), in the 5 V feed to the chain connector | A shorted chain cable takes out the fuse, not the buck |

Draw on the Haltech's switched 12 V (DTM-4 pin 1): three gauges at an estimated 0.3 A each
plus the hub is about 1.1 A at 5 V, so roughly 0.5 A at 13.8 V and up to about 1 A while the
battery sags during cranking. **[bench]** for the gauge current. The CAN +12 V supply must be
enabled in NSP, and if widebands or a dash share it, this adds to their total.

Why the buck's input range matters more than its current rating (the load is only about 1 A):

- **Upper limit.** D1 lets a transient reach about 42 V before it clamps fully. A module rated
  to 30 V (the DFRobot DFR1015 multi-output buck, 7.5-30 V, is the usual cheap option) would
  see more than its maximum. It can still be used if D1 becomes an SMBJ18CA, which clamps
  at about 29 V; the price is that a 24 V jump start blows F1.
- **Lower limit.** After D2's drop, a 7.5 V minimum means the 5 V rail collapses when the
  battery sags below about 8 V during cranking, and every gauge reboots. A 5.3 V minimum
  rides through.
- **Heat.** The DFR1015's protection is listed as acting at 70 C. Behind a dash in summer
  that is not much margin.

12 V sense: 100 k from +12 V (after F1, before D2) to GPIO2, 10 k from GPIO2 to GND, 100 nF across
the 10 k. 12 V reads about 1.09 V, 30 V about 2.7 V. The firmware logs it to show cranking dips.

### 1.4 CAN transceivers

Chosen from what JLCPCB stocks for assembly (it does not carry the TCAN3414 family). The
two buses get different parts because they face different hazards.

| Ref | Part | Notes |
|---|---|---|
| U2, ECU bus | NXP **TJA1051T/3** (SOIC-8) | The side wired into the car. Bus pins survive +/-58 V, TXD dominant time-out so a hung ESP32 cannot hold the Haltech bus dominant, automotive qualified. Needs 5 V on VCC (4.5-5.5 V, from the carrier's 5 V rail) and takes 3.3 V on VIO for the logic pins |
| | Pins: 1 TXD, 2 GND, 3 VCC (5 V), 4 RXD, 5 VIO (3V3), 6 CANL, 7 CANH, 8 S | S low = normal, S high = silent. GPIO23 with a 10 k pull-down. 100 nF at VCC and at VIO |
| U3, gauge bus | TI **SN65HVD230DR** (SOIC-8) | 3.3 V single supply, the same part as on every adapter and on the bench breakouts. Bus pins tolerate -4 to +16 V and there is no dominant time-out, which is acceptable on a private bus that only ever sees 5 V |
| | Pins: 1 D (TXD), 2 GND, 3 VCC (3V3), 4 R (RXD), 5 Vref (n/c), 6 CANL, 7 CANH, 8 RS | RS to GND selects high-speed mode, needed for 1 Mbit/s. 100 nF at VCC. 10 k pull-up from D to 3V3 keeps the bus recessive while the ESP32 is in reset |
| D4, D5 | PESD2CAN (SOT-23) across each CAN pair: pins 1 and 2 to the lines, pin 3 to GND | ESD on the connectors |
| L1 (optional) | Common-mode choke, 51 uH, on the ECU pair | Only if the car proves noisy |

If you would rather have the TCAN3414 on every node (3.3 V everywhere, +/-58 V everywhere),
it has to be bought from Mouser or DigiKey and hand-soldered or consigned; its STB pin must
be tied low and SHDN to ground.

Termination:

| Bus | Termination on the carrier |
|---|---|
| Gauge bus | Fixed split termination: 60 R + 60 R in series across H/L, 4.7 nF from the midpoint to GND. Each 60 R is two 120 R 1206 resistors in parallel, so the board uses one resistor value. The hub is one end of the chain; the terminator plug is the other |
| ECU bus | 120 R behind a 2-pin jumper, open by default. Nexus ECUs have selectable termination; with the car off, 60 R measured across H/L means the bus is already terminated at both ends and the jumper stays open |

### 1.5 Connectors

| Ref | Function | Part |
|---|---|---|
| J10 | ECU (12 V, GND, CAN H, CAN L) | 4-way Micro-Fit 3.0 2x2 on the board (Molex 43045-0412, vertical), to a home-made DTM-4 loom (section 1.6) |
| J11 | Gauge chain out (5 V, GND, CAN H, CAN L) | 4-way Micro-Fit 3.0 2x2, same family as the adapters. Never use DTM-4 on the 5 V side, so the ECU's 12 V lead can never be plugged into a gauge |
| JP1 | ECU termination jumper | 2-pin 2.54 mm header |
| TP | GND, 5 V, 3V3, 12 V sense, RST | Test points |

Chain pin order (both hub J11 and every adapter): 1 = 5 V, 2 = GND, 3 = CAN H, 4 = CAN L. CAN H
and CAN L on a twisted pair; 20 AWG for 5 V and GND.

### 1.6 ECU loom (DTM-4 to the hub)

Made from a DTM kit. Build the gender that mates with the end of the Haltech CAN cable
(or CAN hub port) it will plug into.

| DTM-4 pin | Haltech convention | Wire | Hub J10 pin |
|---|---|---|---|
| 1 | +12 V switched | 20 AWG red | 1 |
| 2 | Ground | 20 AWG black | 2 |
| 3 | CAN H | 22 AWG white, twisted with pin 4 | 3 |
| 4 | CAN L | 22 AWG blue, twisted with pin 3 | 4 |

- Size-20 solid contacts (DTM standard), wedge lock fitted, 500 mm is plenty.
- The pinout is Haltech's documented one for its CAN devices, not something in the broadcast
  protocol PDF. Before the first plug-in, confirm pin 1 is +12 V with the key on and pin 2 is
  ground. CAN H and L cannot be told apart with a meter at idle (both sit at about 2.5 V); if
  they are swapped nothing is damaged, the hub simply sees no frames, so swap and retry.
- The inline 2 A fuse goes in the red lead, close to the DTM end.

### 1.7 Board and mounting

- Suggested size about 70 x 50 mm, four M3 holes on a 60 x 40 mm pattern, 3.2 mm drill.
- The DevKit sits along one long edge with its antenna hanging past the board edge.
- The buck and the 12 V input stage go at the far end from the antenna; keep the switching
  node small and away from the CAN pairs.
- Connectors on one edge so the enclosure has a single cable face.
- All parts on the top side apart from the DevKit's header pins.

### 1.8 Real-time clock

The Haltech broadcast carries no time of day and neither ESP32 keeps time without power,
so the hub carries a battery-backed clock, sets it from the internet when it joins the
home network or from a phone's clock on the page, and sends the time to the gauges once
a second on the private bus.

**Rev A (the board as ordered):** a DS3231 breakout module wired in. Use the small
"DS3231 for Raspberry Pi" type with a CR2032 holder and no charger; it carries its own
pull-ups. VCC to the 3V3 test point, GND to the GND test point, SDA to GPIO6 and SCL to
GPIO7, soldered to the DevKit socket pins where they come through the carrier. For a
rechargeable cell, fit an ML2032 and charge it from 3V3 through a Schottky diode (BAT54)
and 1 k to the holder's positive terminal. Do not use the ZS-042 module: its charger
needs 5 V, and at 5 V the chip's data pins expect 3.5 V highs the C6 cannot give.

**Rev B (built in):**

| Ref | Part | Notes |
|---|---|---|
| U5 | DS3231SN (SOIC-16, +/-2 ppm) or DS3231M (SOIC-8, MEMS, +/-5 ppm, no crystal, cheaper and better stocked) | VCC from 3V3 with 100 nF; SDA, SCL to GPIO6, GPIO7 with 4.7 k pull-ups to 3V3; INT/SQW optional to a spare GPIO for a 1 Hz tick; 32K out unconnected |
| BT1 | ML2032 in a 2032 holder (rechargeable), or a CR2032 (5-10 years, no charger) | To VBAT with 100 nF. Charger for the ML2032: BAT54 from 3V3, 1 k in series, to the cell |
| alternative | 1 F to 5 F, 5.5 V supercapacitor on VBAT, charged through a diode from 3V3 | About a week of hold per farad; no cell to replace |

Time of day is then a `clock` widget on the faces; the hub keeps the time zone as a
setting.

## 2. Gauge adapter board

One per gauge. Plugs onto the gauge's rear 1x8 female header and carries the chain in and out.

### 2.1 Gauge header

1x8, 2.54 mm, female, on the back of the Waveshare board. Position 1 is the VBUS end.

| Position | Signal | Adapter use |
|---|---|---|
| 1 | VBUS (5 V in, same net as USB) | From chain 5 V through D1 |
| 2 | GND | GND |
| 3 | 3V3 | Transceiver supply |
| 4, 5 | GPIO43/44 (UART0) | Not connected. Order of these two is disputed and does not matter |
| 6, 7, 8 | GPIO16, 17, 18 in some order **[bench]** | Two go to transceiver TXD/RXD, one to a spare pad |

Waveshare's hardware reference and its schematic disagree on which of positions 6-8 is which
GPIO. Bring-up step 1 drives them one at a time; write the result here before laying out. The
firmware's `PIN_CAN_TX`/`PIN_CAN_RX` in `src/gauge/board_pins.h` are then set to match, so any
assignment of the three positions works.

### 2.2 Schematic

Drawn in `docs/gauge-adapter-schematic.svg`. The same circuit as a net list, for entering
into Fusion:

| Net | Connections |
|---|---|
| +5V_CHAIN | J1-1, J2-1, D1 anode |
| VBUS_D | D1 cathode, JP1-1 |
| VBUS | JP1-2, P1-1 |
| GND | J1-2, J2-2, P1-2, U1-2 (GND), U1-8 (RS), C1-2, D2-3 |
| +3V3 | P1-3, U1-3 (VCC), C1-1, R1-1 |
| CAN_TX | P1-6, U1-1 (D), R1-2 |
| CAN_RX | P1-7, U1-4 (R) |
| SPARE | P1-8, TP1 |
| CANH | J1-3, J2-3, U1-7, D2-1 |
| CANL | J1-4, J2-4, U1-6, D2-2 |

U1-5 (Vref) is left unconnected.
P1-4 and P1-5 (the gauge's UART0) are left unconnected. P1 positions 6-8 carry GPIO16/17/18
in an order confirmed at bring-up; whichever two GPIOs land on 6 and 7 become `PIN_CAN_TX`
and `PIN_CAN_RX` in the firmware.

### 2.3 Parts

| Ref | Part | Notes |
|---|---|---|
| J1, J2 | 4-way Micro-Fit 3.0 2x2, vertical (Molex 43045-0412), wired pin-for-pin in parallel | Chain in and chain out. Vertical means the cables leave straight back from the gauge, so the enclosure needs depth behind the adapter for the plug and the cable bend |
| P1 | 1x8 2.54 mm male header, mates with the gauge | |
| U1 | SN65HVD230DR (SOIC-8) | Powered from header position 3 (3V3). RS (pin 8) to GND for high-speed mode; Vref (pin 5) unconnected |
| C1 | 100 nF, within 2 mm of U1 pin 3 | |
| R1 | 10 k, D (pin 1) to 3V3 | Keeps the bus recessive while the gauge's ESP32 is in reset |
| D1 | Low-drop Schottky (<= 0.3 V at 0.5 A, e.g. SS14 or a PMEG series part), chain 5 V to header position 1 | Stops USB power reaching the chain when a gauge is plugged into a PC |
| JP1 | 2-pin jumper in series with D1 | Pull it to run a gauge from USB while it is still on the chain |
| D2 | PESD2CAN (SOT-23): pin 1 and pin 2 to the two bus lines, pin 3 to GND | Next to the connectors |
| TP1 | Test pad on the spare GPIO | |
| Holes | Three M2, matching the gauge's standoffs at (-13.75, +14.7), (+13.75, +14.7), (0, -20.5) mm from the display centre, +y towards the header | From Waveshare's STEP model; check against the board |

Termination: none on the adapter. The last adapter's J2 takes a terminator plug: a Micro-Fit
receptacle with 120 R between pins 3 and 4.

### 2.4 5 V budget **[bench]**

With a 5.15 V rail, 20 AWG wire, three gauges at roughly 0.3 A each and D1's drop, the last
gauge sees about 4.4 V. The AXP2101 runs down to 3.9 V and the firmware lowers its input
voltage limit to 4.04 V. Bring-up step 1 gives the real current and the lowest working voltage;
if the margin is thin, the fix is a lower-drop D1 (or a P-FET) before thicker wire.

## 3. JLCPCB assembly

JLCPCB places the surface-mount parts; the through-hole parts (Micro-Fit headers, pin
headers, the Pololu module and the DevKit) are hand-soldered afterwards. LCSC numbers were
checked against JLCPCB's catalogue on 1 October 2026; "basic" parts carry no per-part
loading fee, "extended" parts cost a few dollars each per order.

### 3.1 Gauge adapter (per board)

| Ref | Part | Package | LCSC | Class |
|---|---|---|---|---|
| U1 | SN65HVD230DR | SOIC-8 | C12084 | extended |
| D2 | PESD2CAN,215 | SOT-23 | C75176 | extended |
| D1 | SS14 | SMA | C2480 | basic |
| C1 | 100 nF 50 V X7R | 0805 | C49678 | basic |
| R1 | 10 k 1% | 0805 | C17414 | basic |

Hand-soldered: J1, J2 (Molex 43045-0412), P1 (1x8 male header), JP1 (1x2 header).

### 3.2 Hub carrier

| Ref | Part | Package | LCSC | Class |
|---|---|---|---|---|
| U2 | TJA1051T/3/1J | SOIC-8 | C38695 | extended |
| U3 | SN65HVD230DR | SOIC-8 | C12084 | extended |
| D4, D5 | PESD2CAN,215 | SOT-23 | C75176 | extended |
| D1 | SMBJ26CA-E3/52 (Vishay) | SMB | C515606 | extended |
| F2 | 1812L200/16DR (Littelfuse) | 1812 | C439873 | extended |
| C (bulk) x2 | 100 uF 50 V, Nichicon UUD1H101MNL1GS | SMD 8 x 10 mm | C116241 | extended |
| D2 | SS34 | SMA | C8678 | basic |
| D3 | SS14 | SMA | C2480 | basic |
| R x5 | 120 R 1% (four for the split termination, one behind the ECU jumper) | 1206 | C17909 | basic |
| R x3 | 10 k 1% (S pull-down, sense divider, U3 pull-up) | 0805 | C17414 | basic |
| R x1 | 100 k 1% (sense divider) | 0805 | C149504 | basic |
| C x4 | 100 nF 50 V X7R | 0805 | C49678 | basic |
| C x1 | 1 uF 50 V X7R (buck input) | 0805 | C28323 | basic |
| C x1 | 4.7 nF 50 V X7R (split termination) | 0805 | C1744 | basic |
| C x2 | 10 uF 25 V (5 V rail) | 0805 | C15850 | basic |

Hand-soldered: J10, J11 (Molex 43045-0412), JP1 (1x2 header), the Pololu D36V28F5 and the
ESP32-C6 DevKit.

The store-by-store shopping list is in `docs/bom.md`.
For PCBWay assembly, which sources by manufacturer part number, see `docs/bom-pcbway.md`.

## 4. Schematic files for EasyEDA

`hardware/gauge-adapter.sch` and `hardware/hub-carrier.sch` are Eagle 7 XML schematics,
generated by `scripts/gen_eagle_sch.py` from the net lists above. Import them in EasyEDA
with File > Import > Eagle. Every pin has a short wire and a net label; pins sharing a
label are connected.

- Each surface-mount part carries an `LCSC` attribute with its part number. In EasyEDA,
  set that as the part's supplier number and take the footprint from the LCSC library.
- The footprints inside the files are generic land patterns, there so pin numbers survive
  the import. Do not lay out on them as they are.
- The ESP32-C6-DevKitC-1 footprint is dimensionally correct (from Espressif's drawing).
- The Micro-Fit footprint follows Molex sales drawing SD-43045-005 for the vertical
  43045-0412: four 1.02 mm holes on a 3.00 mm square, circuits 1 and 2 along one row with
  3 and 4 opposite, two 1.02 mm polarising peg holes 9.00 mm apart and 0.94 mm outside the
  3-4 row, body 9.65 x 7.37 mm, 9.9 mm tall. Check it against the current drawing before
  ordering boards.
- The right-angle 43045-0400 (drawing SD-43045-001) shares the same four pin holes but
  uses one 3.00 mm snap-in peg hole on the centreline, 4.32 mm from the nearer pin row, and
  its body is 12.2 mm deep. Swapping between them only moves the retention holes.
- The Pololu footprint is a placeholder: replace it with Pololu's drill guide for the
  D36V28Fx.
- Diodes use pad 1 = cathode and the polarised capacitors pad 1 = positive. Check both
  against the footprint EasyEDA assigns.

To change a circuit, edit the part and net lists in the script and run it again.
