# Ordering from PCBWay instead of JLCPCB

`docs/bom.md` assumes JLCPCB, which fits parts from its own stock and is quoted by LCSC
number. PCBWay works the other way round: it has no fixed catalogue and buys whatever
your BOM names, from Digi-Key, Mouser, LCSC, Arrow and the like, or fits parts you send
("consigned"). So the BOM for PCBWay is written with **manufacturer part numbers**, one
line per value, every designator listed. That is what the two CSV files here are:

- `hardware/pcbway/gauge-adapter-bom.csv`
- `hardware/pcbway/hub-carrier-bom.csv`
- `hardware/pcbway/hub-carrier-p4-bom.csv` (the ESP32-P4 variant of the hub, electrical spec section 5)

Both use PCBWay's column layout (item, designator, quantity, manufacturer, part number,
description, package, type). If their quote page insists on its own spreadsheet
template, copy the columns across; they map one to one. The LCSC column is a hint for
their buyer, since PCBWay also sources from LCSC, and it ties each line back to the
JLCPCB table.

## What PCBWay needs that this repo cannot give you

PCBWay assembles from three uploads, and two of them come out of your layout tool, not
from here:

| Upload | Where it comes from |
|---|---|
| Gerbers and drill files | Your layout (EasyEDA: Fabrication > PCB Fabrication File; Fusion: CAM processor) |
| Pick-and-place (centroid) file: designator, X, Y, rotation, side | Your layout (EasyEDA: Fabrication > Pick and Place; Fusion/Eagle: `mountsmd.ulp`). **The designators must match the BOM exactly**, so keep the schematic's names (U1, D2, J10 ...) when you lay out, or edit both files together |
| BOM | The CSVs here |

Choose "turnkey" on the quote so PCBWay buys the parts. Unlike JLCPCB, PCBWay fits
through-hole parts too, so the Micro-Fit headers and pin headers can go on the boards
there if you would rather not solder them.

## Two parts to keep for yourself

The Pololu regulator module and the ESP32-C6 DevKit are modules, not catalogue parts.
PCBWay can fit them if you post them in as consigned parts, but it is simpler to leave
them off the order (mark them "DNP" or delete the lines) and solder them on when the
boards arrive. Both only have 2.54 mm pins. The same goes for the jumper shunts.

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
| U1 | 1 | Pololu | D36V28F5 (Pololu item 3782) | 5 V 3.2 A step-down regulator module | THT module, 1x4 pins | THT | Consign, or solder it yourself |
| U4 | 1 | Espressif | ESP32-C6-DevKitC-1-N8 | ESP32-C6 development board, 8 MB flash | 2 x 1x16 2.54 mm | THT | Consign, or solder it yourself. Optional sockets: 2 x Sullins PPTC161LFBN-RC |
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
| U1 | 1 | Pololu | D36V28F5 (Pololu item 3782) | 5 V 3.2 A step-down regulator module | THT module, 1x4 pins | THT | Consign, or solder it yourself |
| U4 | 1 | Waveshare | ESP32-P4-WIFI6 (SKU 32020) | ESP32-P4 board with ESP32-C6 Wi-Fi | 2 x 20, Raspberry Pi Pico pattern | THT | Consign, or fit it yourself on 2 x Sullins PPTC201LFBN-RC sockets (1x20, 8.5 mm) |
| TP1-TP5 |  |  |  | Test pads: GND, 5 V, 3V3, 12 V sense, RST | bare pads | none | No part |

## Before you submit

- **Footprints must match the part numbers.** The two Schottky diodes are specified in
  SMA; if your layout used SMB or SMC pads, either change the pads or pick a part in that
  package and change the BOM line. The 100 uF capacitors are an 8 x 10 mm SMD can.
- **Polarity.** D1 to D3, C1, C3 and the PESD2CAN parts are polarised. PCBWay places from
  your centroid file's rotation, so check the silkscreen marks against the datasheets.
- **Alternatives.** If their buyer reports a part out of stock, any part with the same
  value, package and rating is fine for the passives. For the two transceivers, the TVS
  and the PTC fuse, ask before substituting; the spec explains why each was chosen.
- **Quantity.** A turnkey order charges a setup and stencil fee per board design plus
  the parts at distributor prices and a sourcing fee, so assembling 3 adapters costs
  little more than 1. Get the live quote from their site; expect the pair of boards
  assembled and shipped to land in the same bracket as the JLCPCB estimate in
  `docs/bom.md`, give or take the exchange rate and shipping.

Everything else in `docs/bom.md` (connectors and crimps, cable, the Pololu module, the
DevKit, the Waveshare gauges) is bought the same way whichever board house you use.
