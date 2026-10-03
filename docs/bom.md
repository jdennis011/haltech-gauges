# Bill of materials, by store

Sized for three gauges, a hub and spares, with **JLCPCB making and assembling the
two boards**. Buy one Waveshare board first: bring-up step 1 settles the adapter's
header pin order and the 5 V budget before the PCB order. Prices are AUD,
approximate, October 2026.

Already on hand: M5Stack Dial, ESP32-C3 DevKitM-1, DTM connector kit.

## 1. JLCPCB (PCBs with surface-mount assembly)

Prefer PCBWay? See `docs/bom-pcbway.md`: it needs manufacturer part numbers rather
than LCSC numbers, and `hardware/pcbway/` has the BOM in its format.

Order after layout. JLCPCB fits every surface-mount part from its own stock; nothing
needs to be bought for these separately. LCSC numbers were checked against JLCPCB's
catalogue on 1 October 2026.

Gauge adapter, order 5 assembled (3 used):

| Ref | Part | Package | LCSC | Class |
|---|---|---|---|---|
| U1 | SN65HVD230DR | SOIC-8 | C12084 | extended |
| D2 | PESD2CAN,215 | SOT-23 | C75176 | extended |
| D1 | SS14 | SMA | C2480 | basic |
| C1 | 100 nF 50 V X7R | 0805 | C49678 | basic |
| R1 | 10 k 1% | 0805 | C17414 | basic |

Hub carrier, order 5 boards, 2 assembled (1 used):

| Ref | Part | Package | LCSC | Class |
|---|---|---|---|---|
| U2 | TJA1051T/3/1J | SOIC-8 | C38695 | extended |
| U3 | SN65HVD230DR | SOIC-8 | C12084 | extended |
| D4, D5 | PESD2CAN,215 | SOT-23 | C75176 | extended |
| D1 | SMBJ26CA-E3/52 | SMB | C515606 | extended |
| F2 | 1812L200/16DR | 1812 | C439873 | extended |
| C bulk x2 | 100 uF 50 V, UUD1H101MNL1GS | SMD 8 x 10 mm | C116241 | extended |
| D2 | SS34 | SMA | C8678 | basic |
| D3 | SS14 | SMA | C2480 | basic |
| R x5 | 120 R 1% | 1206 | C17909 | basic |
| R x3 | 10 k 1% | 0805 | C17414 | basic |
| R x1 | 100 k 1% | 0805 | C149504 | basic |
| C x4 | 100 nF 50 V X7R | 0805 | C49678 | basic |
| C x1 | 1 uF 50 V X7R | 0805 | C28323 | basic |
| C x1 | 4.7 nF 50 V X7R | 0805 | C1744 | basic |
| C x2 | 10 uF 25 V | 0805 | C15850 | basic |

Rough cost for both orders, boards plus assembly and shipping: $90-120. Each extended
part adds a loading fee of a few dollars per order (two on the adapter, six on the hub).

## 2. Mouser AU or DigiKey AU (through-hole parts you solder, and the cable ends)

| Qty | Manufacturer part number | Part | For |
|---|---|---|---|
| 10 | 43045-0412 (Molex) | Micro-Fit 3.0 header, 2x2, vertical | 2 on the hub, 2 per adapter, 2 spare |
| 10 | 43025-0400 (Molex) | Micro-Fit 3.0 receptacle housing, 2x2 | cable ends, ECU loom, terminator plug |
| 50 | 43030-0007 (Molex) | Micro-Fit female crimp terminal, 20-24 AWG, tin | |
| 4 | 61300811121 (Wurth), or any plain 1x8 2.54 mm male header | | gauge side of each adapter |
| 6 | 61300211121 (Wurth), or any 1x2 2.54 mm male header | | jumpers |
| 6 | 60900213421 (Wurth), or any 2.54 mm jumper shunt | | |
| 6 | MFR-25FBF52-120R (Yageo), or any 120 R 1/4 W through-hole resistor | | terminator plug, bench buses |

About $50-60.

## 3. Core Electronics

| Qty | SKU / item | Price | For |
|---|---|---|---|
| 1 | POLOLU-3782: Pololu 5 V, 3.2 A step-down regulator D36V28F5 | ~$30 | hub 12 V to 5 V |
| 1 | ESP32-C6-DevKitC-1-N8 (8 MB flash) | $27.05 | hub |

Little Bird Electronics lists the same DevKit at $23.65.

## 4. Waveshare (waveshare.com), or Amazon AU for the gauge board

| Qty | Item | Price | For |
|---|---|---|---|
| 3 (1 first) | ESP32-S3-Touch-AMOLED-1.75, plain version (SKU 31261, no case). Not the -G GPS version (it uses GPIO17/18) and not the separate 1.75C product | US$30-40 each | gauges |
| 3 | SN65HVD230 CAN Board (3.3 V transceiver breakout) | a few US$ each | bench steps 3-4 and the M5Dial |

Amazon AU has the gauge board with case at about $92, and generic SN65HVD230 modules.

## 5. Jaycar, Repco or any auto-electrical counter

| Qty | Item | For |
|---|---|---|
| 1 | Inline mini-blade fuse holder | ECU loom, 12 V lead |
| 3 | 2 A mini-blade fuses | |
| 5 m | 20 AWG (0.5 mm2) twin-core cable, red/black | chain 5 V / GND, loom 12 V / GND |
| 5 m | Twisted pair, 22 AWG | chain and loom CAN H / L |
| - | Braided sleeve or loom tape | |
| 10 | M2 x 6 screws | gauges to adapters |
| 10 | M3 x 8 screws | hub enclosure |

## 6. Amazon AU or eBay

| Qty | Item | For |
|---|---|---|
| 1 | Engineer PA-09 crimp tool (~$60), or skip it and buy Molex pre-crimped Micro-Fit leads from Mouser | Micro-Fit terminals |
| 10 | M3 heat-set inserts | printed hub enclosure |
| 1 | AMS1117 3.3 V regulator module, only if not powering the Dial's transceiver from the C3's 3V3 pin | M5Dial bench setup |
| - | Jumper wires, breadboard, USB-C cables | bench |

A 5 V bench supply with a current readout and a multimeter are needed for bring-up
if you do not already have them.

## Rough totals

| Store | Approx. |
|---|---|
| JLCPCB (both boards, assembled, shipped) | $90-120 |
| Mouser / DigiKey | $50-60 |
| Core Electronics | $57 |
| Waveshare (3 gauges + 3 transceiver boards, shipped) | $170-200 |
| Auto-electrical | $40 |
| Amazon / eBay | $80 with the crimp tool |
| **Total** | **about $490-560** |
