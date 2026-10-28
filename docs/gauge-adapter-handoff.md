# Gauge adapter: hand-off

For an agent taking over the gauge adapter board. Written 2026-10-10. The owner (James) does the
PCB layout in his own tools and the enclosures; the agent's side is the schematic, the parts
lists, the spec text and the firmware that touches the adapter's pins. Read this, then
`docs/electrical-spec.md` section 2.

## What it is

One small board per gauge. It plugs onto the rear 1x8 female header of a Waveshare
ESP32-S3-Touch-AMOLED-1.75 and carries the gauge chain in and out: 5 V, GND, CAN H, CAN L on
Micro-Fit 3.0 connectors. On it are a 3.3 V CAN transceiver for the gauge, a Schottky diode so
chain power reaches the gauge but USB power never reaches the chain, a jumper to bypass that
diode on the bench, and bus ESD protection. The chain passes straight through: the two
connectors are wired pin for pin in parallel, so neither is "in" or "out".

## Status

| | |
|---|---|
| Rev A | Designed and in the 2026-10-04 PCBWay order with the hub carrier (confirm with the owner that the adapters were included). Treat its files as the as-built record: change them only to record bench results. It needs a 120 R terminator plug in the last gauge's spare port. |
| Rev B | Decided 2026-10-09/10. Self-terminating: each adapter senses an empty port and switches its own 120 R in. Chain connector goes from 2x2 to 2x3. Spec section 2.5 and the rev B table in `docs/bom-jlcpcb.md` are complete. The owner drew the schematic in EasyEDA on 2026-10-10 (a 46 mm round board, Micro-Fits on the outer face); it was reviewed against the spec and matches apart from the items in "Open items". Layout not started. |
| Bench | Nothing on the adapter has been powered yet. The gauge board's header pin order for GPIO16/17/18 is still unknown (below). |

## Files

| File | What |
|---|---|
| `docs/electrical-spec.md` section 2 | The design: header pinout (2.1), net list (2.2), parts (2.3), 5 V budget (2.4), rev B (2.5). Section 1.5 has the chain pin order and connector parts; 3.1 the assembly notes. |
| `hardware/gauge-adapter.sch` | Rev A schematic, Eagle 7 XML, importable into EasyEDA and Fusion. Generated, do not hand-edit. |
| `scripts/gen_eagle_sch.py` | The generator. `gauge_adapter()` is the adapter; `python scripts/gen_eagle_sch.py` rewrites every `.sch` in `hardware/`. Add rev B as a new function writing `hardware/gauge-adapter-rev-b.sch` rather than changing `gauge_adapter()`. |
| `docs/gauge-adapter-schematic.svg` | Rev A schematic drawn out for reading. |
| `docs/bom-jlcpcb.md`, `hardware/jlcpcb/gauge-adapter-bom.csv` | Rev A BOM with manufacturer and LCSC numbers, and the rev B additions table. Keep the CSV and the markdown table in step. |
| `docs/bom.md` | Shopping list by store, including the Micro-Fit housings, terminals and crimp tool. |
| `docs/bringup-checklist.md` | Step 1 (bare gauge board) maps the header pins; step 4 is hub plus gauge through the adapter. |
| `src/gauge/board_pins.h` | `PIN_CAN_TX` 17, `PIN_CAN_RX` 18, `PIN_SPARE` 16 on the Waveshare build; set from the step 1 result. |
| `src/gauge/can_task.cpp` | The gauge's CAN driver and protocol node, which uses those pins. |

## Rev A, as designed

**Gauge header P1** (1x8, 2.54 mm, position 1 at the VBUS end): 1 VBUS, 2 GND, 3 3V3, 4 and 5
GPIO43/44 (UART0, unconnected), 6 to 8 GPIO16/17/18 **in an order Waveshare's documents
disagree on**. The schematic puts CAN TX on position 6, RX on 7 and the spare on 8; the firmware
is then set to whichever GPIOs those turn out to be, so any order works, but the order must be
known before the firmware's pin defines are final. Bring-up step 1 drives each GPIO in turn to
find out; record the answer in spec 2.1.

**Chain connectors J1, J2**: Micro-Fit 3.0 2x2 vertical headers, Molex 43045-0412, wired pin for
pin. Pin order everywhere on the chain, hub J11 included: 1 = 5 V, 2 = GND, 3 = CAN H, 4 = CAN L.
Cable ends are housing 43025-0400 with 43030-0007 terminals (20-24 AWG); 20 AWG for all four
cores is simplest, CAN pair twisted, white = CAN H, blue = CAN L.

**Circuit** (spec 2.2 has the full net list):

- U1 SN65HVD230DR, 3.3 V from P1-3. D to CAN_TX, R to CAN_RX, RS to GND (high-speed mode), Vref
  open. R1 10 k pulls D up to 3V3 so the bus stays recessive while the gauge is in reset.
- D1 SS14 from chain 5 V to the gauge's VBUS, through JP1. Pull JP1 to run the gauge from USB
  while it is still on the chain; otherwise the PC's USB would feed the whole chain.
- D2 PESD2CAN across CAN H and CAN L, next to the connectors.
- C1 100 nF at U1. TP1 is a test pad on the spare GPIO.
- No termination on the board. The hub is terminated; the last adapter's spare port takes a
  Micro-Fit plug with 120 R between pins 3 and 4. A terminator in a middle gauge's spare port
  would be a third one and load the bus to 40 R.

**Mechanical**: three M2 holes matching the gauge's standoffs at (-13.75, +14.7), (+13.75, +14.7)
and (0, -20.5) mm from the display centre, +y towards the header, taken from Waveshare's STEP
model and not yet checked against a board. The connectors are vertical, so the leads leave
straight back from the gauge and the enclosure needs depth behind the adapter for the plug and
the cable bend. Three gauges at about 0.3 A each on 20 AWG leave roughly 4.4 V at the last
gauge (spec 2.4); the gauge's PMIC is set to accept down to 4.04 V. The real current and the
lowest working voltage come from bring-up step 1.

## Rev B: self-terminating

The owner chose this over a terminator plug. Full description in spec 2.5; the parts with
manufacturer numbers are in `docs/bom-jlcpcb.md` under "Gauge adapter, rev B".

- **Connector**: J1 and J2 become 2x3, Molex 43045-0612 (housings 43025-0600, same terminals).
  Pins 1 to 4 unchanged; **pin 6 = GND, pin 5 = sense**. The hub's chain connector J11 goes 2x3
  too, pin 6 grounded; the hub's ECU input J10 stays 2x2, so the two plugs no longer
  interchange (a chain lead in J10 would put 12 V into a gauge).
- **Cable**: still 4-core. In every plug a short loop of wire with a terminal on each end
  shorts cavities 5 and 6, under the heat shrink. The owner chose the plug loop over a crossed
  6-core cable; the cost is that a lead plugged into the last gauge with nothing on its far end
  reads as connected, so that gauge does not terminate. Bench only; in the car leads are in at
  both ends or removed.
- **Sense**: each port's pin 5 has a 4.7 k pull-up to 3V3 (R2, R3). A plug pulls it low. The two
  lines are OR'd through BAT54 diodes (D3, D4) into one node with a 100 k pull-down (R4): high
  when either port is empty.
- **Switch**: that node drives IN of U2, a TS5A3166 SPST analogue switch (SOT-23-5) powered from
  the chain 5 V with C2 100 nF, which connects R5 120 R across CAN H and CAN L. Either port
  empty: terminated. Both occupied: open. Both empty (a gauge alone on the bench): terminated
  once, harmless. The node sits at about 2.9 V after the diode drop and R4, above the switch's
  2.4 V threshold at a 5 V supply; 10 k pull-ups would leave only 2.7 V, which is why they are
  4.7 k. The SN74LVC1G66 is not a substitute (its 5 V threshold is about 3.5 V). U2's
  SOT-23-5 pinout is 1 NO, 2 GND, 3 IN, 4 COM, 5 V+. No split capacitor on the adapter; the
  hub's termination carries the common-mode cap.
- **Spare GPIO**: the same node goes through R6 1 k to the spare GPIO, so the gauge can read "I
  am the end of the chain". TP1 moves to this node: high with the ports empty, low with a looped
  plug in. Nothing in the firmware reads it yet; see the firmware note below.
- **Designators**: keep rev A's (U1, D1, D2, C1, R1, J1, J2, P1, JP1, TP1) and add U2, R2 to R6,
  D3, D4, C2 as in the BOM table, so the two revisions read side by side.

## Open items, in order

0. **From the 2026-10-10 schematic review**: the sheet's note gives U2's pins as "2 COM, 3 GND",
   which contradicts the datasheet (1 NO, 2 GND, 3 IN, 4 COM, 5 V+); check the symbol's pin
   numbers and fix the note. R2/R3 are drawn as 10 k; the spec now says 4.7 k. Confirm D1's
   orientation (anode on +5V_CHAIN), that LCSC C85084 is a single BAT54 (pin 1 anode, pin 3
   cathode), that TP1 is on the outer face where it can be reached with the adapter plugged
   on, and the standoff hole positions against a real board before fixing the 46 mm outline.
1. **Header pin order** (bring-up step 1): which of P1 positions 6, 7, 8 is GPIO16, 17, 18.
   Record in spec 2.1 and set `src/gauge/board_pins.h` to match. Everything on the adapter
   waits on this.
2. **Whether the gauge starts by itself on header 5 V** with no button press, and the standoff
   hole positions checked against a real board.
3. **Full-white current and the lowest voltage the gauge runs at** (step 1), which decide
   whether D1's 0.3 V drop is acceptable at the third gauge or a P-FET ideal diode is needed.
4. **Rev B schematic**: add `gauge_adapter_rev_b()` to the generator, regenerate, and draw it
   out as `docs/gauge-adapter-rev-b-schematic.svg` if the owner wants a readable copy. Then the
   rev B BOM CSV alongside the markdown table.
5. **After the first rev B boards**: check TP1 reads as above with and without a looped plug,
   measure 60 R across H and L at the hub with one gauge on the chain and power off, and check
   TERM_ENABLE reads about 2.9 V with a port empty and under 0.1 V with both looped plugs in.

## Firmware note

The gauge firmware (`src/gauge/can_task.cpp`) talks to the hub through the protocol in
`lib/hg_proto/`. The gauge's periodic `Status` message has a `flags` byte with two bits used
(`kStatusFlagTrying`, `kStatusFlagDemo`). Reporting "end of chain" is a third flag read from
`PIN_SPARE` on rev B adapters; the hub's roster could then warn when two gauges claim it or none
does. That is a small, self-contained change and has not been started. On rev A adapters the
spare GPIO floats, so the flag must only be trusted on rev B hardware; there is no hardware
revision field on the adapter, so the hub cannot tell them apart unless one is added (the
protocol's `Info` message already carries `hwRevision` for the gauge board).

## Conventions

- Spec text is the source of truth; the schematic generator, BOMs and firmware pin defines follow
  it. When something changes, change the spec first.
- Parts carry LCSC numbers where they exist (JLCPCB places from them); add one
  for every new part, and give a distributor alternative in the notes where the LCSC part is a
  generic.
- Molex part numbers: board headers 43045-xxxx, cable housings 43025-xxxx, terminals 43030-0007.
  Molex calls the board part the header and the cable part the receptacle.
- The owner commits and pushes only when he asks; do not commit on your own.
- Haltech's protocol document is not in the repo (`docs/ref/README.md` says where it is) and
  nothing here needs it.
