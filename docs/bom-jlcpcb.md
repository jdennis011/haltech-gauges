# Ordering from JLCPCB

JLCPCB makes the boards and fits the surface-mount parts from its own stock, which is
LCSC's: every placed part is named by its LCSC number. The files for the order come out
of EasyEDA, where each part on the schematic already carries that number:

| Upload | Where it comes from |
|---|---|
| Gerbers | EasyEDA: Fabrication > PCB Fabrication File (Gerber) |
| BOM | EasyEDA: Fabrication > BOM, in JLCPCB's layout (Comment, Designator, Footprint, LCSC Part #) |
| Pick-and-place (CPL) | EasyEDA: Fabrication > Pick and Place. The designators must match the BOM |

The CSVs in `hardware/jlcpcb/` are the same BOMs in JLCPCB's layout, kept here as the
record of which LCSC part each designator is, so the EasyEDA export can be checked against
them before the order:

- `hardware/jlcpcb/gauge-adapter-bom.csv`
- `hardware/jlcpcb/hub-carrier-bom.csv` (the ESP32-C6 hub)
- `hardware/jlcpcb/hub-carrier-p4-bom.csv` (the ESP32-P4 hub, electrical spec section 5)
- `hardware/jlcpcb/hub-carrier-p4-rev-b-bom.csv` (the next P4 hub, spec section 6.3; written by `scripts/gen_eagle_sch.py` from the schematic)

Rows with no LCSC number are the through-hole parts, the modules and the test pads:
JLCPCB shows them as unmatched, so leave them unselected and fit them yourself (below).
The tables further down list the same parts with their manufacturer numbers, which the
LCSC numbers were checked against.

## Basic and extended parts

JLCPCB's "basic" parts are loaded at no extra charge; an "extended" part costs a loading
fee per board design. The transceivers, the TVS, the PTC fuse and the clock are extended;
the passives were chosen from the basic range where one existed. If the order page shows
a part out of stock, any part of the same value, package and rating does for the
passives; for the transceivers, the TVS and the PTC ask first, since the spec explains
why each was chosen.

## Parts you fit yourself

The Pololu regulator module, the Waveshare or DevKit processor board, the Micro-Fit
headers, the pin headers, the GPS socket, the coin-cell holder and the jumper shunts are
hand-soldered when the boards arrive. JLCPCB's standard assembly service can fit the
through-hole headers at a per-joint charge if you would rather; the modules stay with you
either way. Mark them "do not place" in the order rather than deleting the lines, so the
designators stay in step with the schematic.

## Gauge adapter

Order 5 boards, assemble 3 to 5.

| Designators | Qty | Manufacturer | Part number | Description | Package | Fit | Notes |
|---|---|---|---|---|---|---|---|
| U1 | 1 | Texas Instruments | SN65HVD230DR | CAN transceiver, 3.3 V | SOIC-8 | SMT |  |
| D2 | 1 | Nexperia | PESD2CAN,215 | CAN bus ESD protection diode | SOT-23 | SMT |  |
| D1 | 1 | MDD | SS14 | Schottky diode 1 A 40 V, chain 5 V to gauge VBUS | SMA (DO-214AC) | SMT | Any SS14 in SMA; Vishay SS14-E3/61T from a distributor |
| C1 | 1 | Samsung | CL21B104KBCNNNC | 100 nF 50 V X7R | 0805 | SMT |  |
| R1 | 1 | Yageo | RC0805FR-0710KL | 10 k 1% | 0805 | SMT |  |
| J1, J2 | 2 | Molex | 43045-0412 | Micro-Fit 3.0 header, 2x2, vertical, chain in and out | THT, 4 pins + 2 pegs | THT | Both wired in parallel |
| P1 | 1 | Wurth Elektronik | 61300811121 | 1x8 male header 2.54 mm, mates with the gauge | THT | THT | Any plain 1x8 2.54 mm header |
| JP1 | 1 | Wurth Elektronik | 61300211121 | 1x2 male header 2.54 mm, chain-power jumper | THT | THT | Fit shunt 60900213421 |
| TP1 |  |  |  | Test pad on the spare GPIO | bare pad | none | No part |

## Gauge adapter, rev B (self-terminating)

Electrical spec section 2.5. Not laid out yet; the user does the layout. Everything from the rev A
list stays except J1 and J2, which go up one size, plus the parts below.

| Designators | Qty | Manufacturer | Part number | Description | Package | Fit | Notes |
|---|---|---|---|---|---|---|---|
| J1, J2 | 2 | Molex | 43045-0612 | Micro-Fit 3.0 header, 2x3, vertical: pins 1-4 as rev A, 5 = sense, 6 = GND | THT, 6 pins + 2 pegs | THT | Replaces 43045-0412. Cable housings become 43025-0600; terminals stay 43030-0007 |
| U2 | 1 | Texas Instruments | TS5A3166DBVR | SPST analogue switch, 1 ohm, 0 to V+ signal range, switches the 120 R in | SOT-23-5 | SMT | V+ from the chain 5 V. Pins: 1 NO, 2 GND, 3 IN, 4 COM, 5 V+. Not the SN74LVC1G66: its 5 V threshold (about 3.5 V) is above the enable node's 2.9 V |
| R5 | 1 | Yageo | RC0805FR-07120RL | 120 R 1%, termination, in series with U2 across CAN H and CAN L | 0805 | SMT | About 35 mW while dominant |
| D3, D4 | 2 | Nexperia | BAT54,215 | Schottky, one per port: ORs the two sense lines into U2's IN | SOT-23 | SMT | Pin 1 anode, pin 3 cathode |
| R2, R3 | 2 | Yageo | RC0805FR-074K7L | 4.7 k 1%, pull-up on each port's pin 5 to 3V3 | 0805 | SMT | 4.7 k, not 10 k, so the enable node reaches about 2.9 V |
| R4 | 1 | Yageo | RC0805FR-07100KL | 100 k 1%, pull-down on U2's IN | 0805 | SMT |  |
| R6 | 1 | Yageo | RC0805FR-071KL | 1 k, U2's IN node to the spare GPIO ("I am the end of the chain") | 0805 | SMT | Protects the pin if the firmware ever drives it |
| C2 | 1 | Samsung | CL21B104KBCNNNC | 100 nF 50 V X7R, on U2's V+ | 0805 | SMT | Same part as C1 |

TP1 stays, now on the sense node. No terminator plug is needed with these boards.

## Hub carrier

Order 5 boards, assemble 1 or 2.

| Designators | Qty | Manufacturer | Part number | Description | Package | Fit | Notes |
|---|---|---|---|---|---|---|---|
| U2 | 1 | NXP | TJA1051T/3/1J | CAN transceiver, 5 V supply, 3.3 V logic, ECU bus | SOIC-8 | SMT |  |
| U3 | 1 | Texas Instruments | SN65HVD230DR | CAN transceiver, 3.3 V, gauge bus | SOIC-8 | SMT |  |
| D4, D5 | 2 | Nexperia | PESD2CAN,215 | CAN bus ESD protection diode, one per connector | SOT-23 | SMT |  |
| D1 | 1 | Vishay | SMBJ26CA-E3/52 | TVS diode, bidirectional, 26 V standoff, 12 V input | SMB (DO-214AA) | SMT |  |
| D2 | 1 | MDD | SS34 | Schottky diode 3 A 40 V, reverse-polarity block | SMA (DO-214AC) | SMT | Distributor alternative in the same SMA package: Diodes Inc B340A-13-F |
| D3 | 1 | MDD | SS14 | Schottky diode 1 A 40 V, 5 V rail to DevKit | SMA (DO-214AC) | SMT | Any SS14 in SMA; Vishay SS14-E3/61T from a distributor |
| F2 | 1 | Littelfuse | 1812L200/16DR | PTC resettable fuse, 2 A hold, 16 V, chain output | 1812 | SMT |  |
| C1, C3 | 2 | Nichicon | UUD1H101MNL1GS | 100 uF 50 V aluminium electrolytic, SMD | 8 x 10 mm | SMT | Polarised |
| C2 | 1 | Samsung | CL21B105KBFNNNE | 1 uF 50 V X7R, buck input | 0805 | SMT |  |
| C4, C5 | 2 | Samsung | CL21A106KAYNNNE | 10 uF 25 V X5R, 5 V rail | 0805 | SMT |  |
| C6, C7, C8, C10 | 4 | Samsung | CL21B104KBCNNNC | 100 nF 50 V X7R, decoupling | 0805 | SMT |  |
| C9 | 1 | Samsung | CL21B472KBANNNC | 4.7 nF 50 V X7R, split termination | 0805 | SMT |  |
| R3, R4, R5, R6, R7 | 5 | Yageo | RC1206FR-07120RL | 120 R 1%, termination | 1206 | SMT |  |
| R1, R2, R9 | 3 | Yageo | RC0805FR-0710KL | 10 k 1% | 0805 | SMT |  |
| R8 | 1 | Yageo | RC0805FR-07100KL | 100 k 1%, 12 V sense divider | 0805 | SMT |  |
| J10, J11 | 2 | Molex | 43045-0412 | Micro-Fit 3.0 header, 2x2, vertical: ECU in, chain out | THT, 4 pins + 2 pegs | THT |  |
| JP1 | 1 | Wurth Elektronik | 61300211121 | 1x2 male header 2.54 mm, ECU termination jumper | THT | THT | Shunt 60900213421 supplied loose, fitted only if the ECU bus needs it |
| U1 | 1 | Pololu | D36V28F5 (Pololu item 3782) | 5 V 3.2 A step-down regulator module | THT module, 1x5 pins, in the order EN, VIN, GND, GND, VOUT (checked against a module 2026-10-27; a footprint with VIN and VOUT swapped killed the first P4) | THT | Not placed by JLCPCB; solder it yourself |
| U4 | 1 | Espressif | ESP32-C6-DevKitC-1-N8 | ESP32-C6 development board, 8 MB flash | 2 x 1x16 2.54 mm | THT | Not placed by JLCPCB; solder it yourself. Optional sockets: 2 x Sullins PPTC161LFBN-RC |
| TP1-TP5 |  |  |  | Test pads: GND, 5 V, 3V3, 12 V sense, RST | bare pads | none | No part |

## Hub carrier, ESP32-P4 variant

Instead of the hub carrier above, not as well as it. The Waveshare board and the Pololu
module are yours to fit; the GPS, its SMA lead and the antenna are bought separately
(electrical spec section 5.7).

| Designators | Qty | Manufacturer | Part number | Description | Package | Fit | Notes |
|---|---|---|---|---|---|---|---|
| U2 | 1 | NXP | TJA1051T/3/1J | CAN transceiver, 5 V supply, 3.3 V logic, ECU bus | SOIC-8 | SMT |  |
| U3 | 1 | Texas Instruments | SN65HVD230DR | CAN transceiver, 3.3 V, gauge bus | SOIC-8 | SMT |  |
| U5 | 1 | Analog Devices | DS3231SN#T&R | Real-time clock, temperature compensated, I2C | SOIC-16 wide | SMT |  |
| D4, D5 | 2 | Nexperia | PESD2CAN,215 | CAN bus ESD protection diode, one per connector | SOT-23 | SMT |  |
| D1 | 1 | Vishay | SMBJ26CA-E3/52 | TVS diode, bidirectional, 26 V standoff, 12 V input | SMB (DO-214AA) | SMT |  |
| D2 | 1 | MDD | SS34 | Schottky diode 3 A 40 V, reverse-polarity block | SMA (DO-214AC) | SMT | Distributor alternative in the same SMA package: Diodes Inc B340A-13-F |
| D3 | 1 | MDD | SS14 | Schottky diode 1 A 40 V, 5 V rail to the board's VSYS | SMA (DO-214AC) | SMT | Any SS14 in SMA; Vishay SS14-E3/61T from a distributor |
| D6 | 1 | Nexperia | BAT54,215 | Schottky diode 30 V 200 mA, cell trickle charge | SOT-23 | SMT | Any single BAT54 in SOT-23 (pin 1 anode, pin 3 cathode) |
| Q1 | 1 | Nexperia | 2N7002,215 | N-channel MOSFET 60 V, turns the GPS off | SOT-23 | SMT | Any 2N7002 in SOT-23 (pin 1 gate, 2 source, 3 drain); LCSC C8545 is Changjiang's |
| F2 | 1 | Littelfuse | 1812L200/16DR | PTC resettable fuse, 2 A hold, 16 V, chain output | 1812 | SMT |  |
| C1, C3 | 2 | Nichicon | UUD1H101MNL1GS | 100 uF 50 V aluminium electrolytic, SMD | 8 x 10 mm | SMT | Polarised |
| C2 | 1 | Samsung | CL21B105KBFNNNE | 1 uF 50 V X7R, buck input | 0805 | SMT |  |
| C4, C5, C16 | 3 | Samsung | CL21A106KAYNNNE | 10 uF 25 V X5R | 0805 | SMT |  |
| C6, C7, C8, C10, C14, C15 | 6 | Samsung | CL21B104KBCNNNC | 100 nF 50 V X7R, decoupling | 0805 | SMT |  |
| C9 | 1 | Samsung | CL21B472KBANNNC | 4.7 nF 50 V X7R, split termination | 0805 | SMT |  |
| R3, R4, R5, R6, R7 | 5 | Yageo | RC1206FR-07120RL | 120 R 1%, termination | 1206 | SMT |  |
| R1, R2, R9, R20 | 4 | Yageo | RC0805FR-0710KL | 10 k 1% | 0805 | SMT |  |
| R8, R24 | 2 | Yageo | RC0805FR-07100KL | 100 k 1%: 12 V sense divider, Q1 gate pull-down | 0805 | SMT |  |
| R21, R22, R23 | 3 | Yageo | RC0805FR-071KL | 1 k 1% | 0805 | SMT |  |
| J10, J11 | 2 | Molex | 43045-0412 | Micro-Fit 3.0 header, 2x2, vertical: ECU in, chain out | THT, 4 pins + 2 pegs | THT |  |
| J12 | 1 | Sullins | PPTC091LFBN-RC | 1x9 female socket 2.54 mm, 8.5 mm tall, for the Adafruit Ultimate GPS | THT | THT |  |
| JP1, JP2 | 2 | Wurth Elektronik | 61300211121 | 1x2 male header 2.54 mm: ECU termination, ML2032 charge | THT | THT | Shunts 60900213421 supplied loose. JP2 is fitted only with a rechargeable ML2032 |
| BT1 | 1 | Keystone | 3034 | Holder for a 2032 coin cell | SMT | SMT | Or any 2032 holder; the footprint in the schematic is a placeholder |
| U1 | 1 | Pololu | D36V28F5 (Pololu item 3782) | 5 V 3.2 A step-down regulator module | THT module, 1x5 pins, in the order EN, VIN, GND, GND, VOUT (checked against a module 2026-10-27; a footprint with VIN and VOUT swapped killed the first P4) | THT | Not placed by JLCPCB; solder it yourself |
| U4 | 1 | Waveshare | ESP32-P4-WIFI6 (SKU 32020) | ESP32-P4 board with ESP32-C6 Wi-Fi | 2 x 20, Raspberry Pi Pico pattern | THT | Not placed by JLCPCB; fit it yourself on 2 x Sullins PPTC201LFBN-RC sockets (1x20, 8.5 mm) |
| TP1-TP5 |  |  |  | Test pads: GND, 5 V, 3V3, 12 V sense, RST | bare pads | none | No part |

## Hub carrier, ESP32-P4 rev B

Rev A as built plus the rows below (electrical spec section 6.3); everything not listed is the
rev A table's. `hardware/jlcpcb/hub-carrier-p4-rev-b-bom.csv` is the complete list in JLCPCB's
layout, generated from the schematic, so it and this table cannot drift apart on designators.
LCSC numbers marked **unchecked** were not looked up on LCSC: confirm each in EasyEDA when the
part is placed, and fill in the blank ones.

| Designators | Qty | Manufacturer | Part number | Description | Package | Fit | LCSC | Notes |
|---|---|---|---|---|---|---|---|---|
| U7 | 1 | NXP | TJA1051T/3/1J | CAN transceiver, 5 V supply, 3.3 V logic, vehicle CAN | SOIC-8 | SMT | C38695 | |
| D7 | 1 | Nexperia | PESD2CAN,215 | CAN bus ESD protection, vehicle CAN pair | SOT-23 | SMT | C75176 | |
| R46 | 1 | Yageo | RC1206FR-07120RL | 120 R 1%, vehicle bus termination through JP3 | 1206 | SMT | C17909 | |
| R45 | 1 | Yageo | RC0805FR-0710KL | 10 k, pull-down on U7's silent pin | 0805 | SMT | C17414 | |
| C20, C23, C24 | 3 | Samsung | CL21B104KBCNNNC | 100 nF 50 V X7R, decoupling | 0805 | SMT | C49678 | |
| J11, J14 | 2 | Molex | 43045-0612 | Micro-Fit 3.0 header, 2x3, vertical: chain, vehicle CAN | THT, 6 pins + 2 pegs | THT | | Housings 43025-0600, terminals 43030-0007 |
| R44 | 1 | Yageo | RC0805FR-074K7L | 4.7 k, chain sense pull-up | 0805 | SMT | C17673 | unchecked |
| Q7 | 1 | Alpha & Omega | AO3401A | P-channel MOSFET 30 V 4 A, 5 V rail cutoff | SOT-23 | SMT | C15127 | unchecked |
| Q8 | 1 | onsemi | MMBT3906 | PNP, lifts Q7's gate on over-voltage | SOT-23 | SMT | | Any SOT-23 PNP (S8550) with pins B, E, C |
| U9 | 1 | Texas Instruments | TL431AIDBZR | 2.495 V shunt reference; trips the cutoff at 6.24 V | SOT-23 | SMT | C3113 | unchecked; the generic footprint assumes pins REF, A, K: check the part chosen |
| R40, R43 | 2 | Yageo | RC0805FR-0710KL | 10 k: Q7 gate pull-down, divider bottom | 0805 | SMT | C17414 | |
| R41 | 1 | Yageo | RC0805FR-072K2L | 2.2 k, TL431 to Q8 base | 0805 | SMT | C17520 | unchecked |
| R42 | 1 | Yageo | RC0805FR-0715KL | 15 k, divider top | 0805 | SMT | | |
| C19 | 1 | Samsung | CL21B104KBCNNNC | 100 nF across the divider bottom | 0805 | SMT | C49678 | |
| Q2 | 1 | Alpha & Omega | AO3400A | N-channel MOSFET, logic level, horn relay input | SOT-23 | SMT | C20917 | unchecked. A 2N7002 works only with JP4 fitted |
| D8 | 1 | Nexperia | BAT54S,215 | Dual series Schottky, the charge pump | SOT-23 | SMT | C47546 | unchecked; pins A1, K2, common |
| C21 | 1 | Samsung | CL21B105KBFNNNE | 1 uF 50 V X7R, pump capacitor | 0805 | SMT | C28323 | |
| C22 | 1 | Samsung | CL21A106KAYNNNE | 10 uF 25 V X5R, gate reservoir | 0805 | SMT | C15850 | |
| R47 | 1 | Yageo | RC0805FR-074K7L | 4.7 k, reservoir bleed (about 50 ms) | 0805 | SMT | C17673 | unchecked |
| R48, R50, R51, R52, R53 | 5 | Yageo | RC0805FR-07100KL | 100 k pull-downs: GPIO49 and the four remote gates | 0805 | SMT | C149504 | |
| R49 | 1 | Yageo | RC0805FR-071KL | 1 k, direct horn drive through JP4 | 0805 | SMT | C17513 | |
| JP3, JP4 | 2 | Wurth Elektronik | 61300211121 | 1x2 headers: vehicle termination (fitted), horn direct (open) | THT | THT | | Shunts 60900213421 |
| J15 | 1 | Wurth Elektronik | 61300311121 | 1x3 header: 5 V, GND, IN to the relay module | THT | THT | | |
| J13 | 1 | Wurth Elektronik | 61300611121 | 1x6 header: GND, REM, SW1-SW4 | THT | THT | | |
| Q3, Q4, Q5, Q6 | 4 | Nexperia | 2N7002,215 | N-channel MOSFET, wired-remote switches | SOT-23 | SMT | C8545 | |
| R30, R31, R32, R33 | 4 | | | Axial through-hole resistors, values from the head unit | 10.16 mm | DNP | | Fit when the head unit is chosen |
| U8 | 1 | Microchip | MCP4531T-103E/MS | 10 k digital pot, I2C, the alternative to R30-R33 | MSOP-8 | DNP | | Leave off the order |
| U6 | 1 | Torex | XC6206P302MR | 3.0 V LDO, ML2032 charge limit (rev A as built) | SOT-23 | SMT | | |
| C17, C18 | 2 | Samsung | CL21B105KBFNNNE | 1 uF 50 V X7R, U6 in and out | 0805 | SMT | C28323 | |
| D6 | 1 | Nexperia | BAS70,215 | Schottky 70 V, ML2032 charge (replaces rev A's BAT54 line) | SOT-23 | SMT | | |
| R8 | 1 | Yageo | RC0805FR-07150KL | 150 k, 12 V sense divider top (rev A as built) | 0805 | SMT | | Replaces the 100 k line of the rev A table |
| BT1 | 1 | Keystone | 103 | Holder for the ML2032 | THT | THT | | ML2032 only, with JP2 |
| TP6 | | | | Test pad: the buck's output before the cutoff | bare pad | none | | No part |

## Before you submit

- **Footprints must match the LCSC parts.** EasyEDA's JLCPCB library ties each LCSC
  number to its footprint, which avoids the mismatch; a part placed from any other library
  needs its package checked against the LCSC listing (the two Schottky diodes are SMA, the
  100 uF capacitors an 8 x 10 mm SMD can).
- **Pin order of module sockets.** Check a library footprint for a module against the
  module in hand before the order: the rev A carrier's Pololu socket had VIN and VOUT
  swapped and took the first P4 with it (`docs/bringup-checklist.md`, 2026-10-27).
- **Polarity.** D1 to D3, C1, C3 and the PESD2CAN parts are polarised. JLCPCB places from
  the CPL rotation; check the order page's placement preview against the datasheets and
  turn a rotated part there before paying.
- **Quantity.** The PCB minimum is five; assembly is charged as a setup and stencil fee
  per design plus the parts, so assembling five costs little more than two. Take the live
  quote from the order page.

Everything else in `docs/bom.md` (connectors and crimps, cable, the Pololu module, the
processor boards, the Waveshare gauges) is bought the same way.
