# Haltech gauges

Round digital gauges for a car running a Haltech Nexus ECU. A hub listens to the
Haltech CAN broadcast, bridges it to a private gauge bus, and hosts a web page for
setting the gauges up. One to three Waveshare ESP32-S3-Touch-AMOLED-1.75 gauges hang off
the hub on a daisy chain of 5 V and CAN, each on a small adapter board.

- Hub: ESP32-C6 (two CAN controllers plus Wi-Fi) on a carrier board. A Lolin C3 Mini runs
  the hub firmware for development, minus the ECU bridge.
- Gauge: the Waveshare board with an adapter that supplies power and CAN. An M5Stack Dial
  stands in on the bench.
- Faces are JSON configs: dials, rings, bars, numbers, labels, warning lights and shift
  rims, in a choice of fonts and themes. Fifteen starter templates are built in.
- The hub's page previews faces, pushes configs to gauges, and lets gauges be planned
  before the hardware exists. A visual designer is planned (`docs/designer-plan.md`).

## Building

PlatformIO with the pioarduino platform (installed on first build into its own core
directory, leaving a stock PlatformIO install alone).

```
pio run -e hub            # ESP32-C6 hub
pio run -e hub_dev_c3     # Lolin C3 Mini development hub
pio run -e gauge          # Waveshare gauge
pio run -e gauge_m5dial   # M5Stack Dial as a gauge
pio test -e native        # host unit tests (scripts/test.ps1 finds a g++ on Windows)
```

The hub's web page, templates and fonts are embedded into the firmware by
`scripts/embed_web.py` before each hub build.

## Documents

| | |
|---|---|
| `docs/web-ui.md` | The hub's page and its REST and WebSocket API |
| `docs/config-schema.md` | The face config format |
| `docs/templates.md` | The built-in templates, with a gallery |
| `docs/electrical-spec.md` | Hub carrier and gauge adapter: circuits, parts, connectors |
| `docs/bom.md`, `docs/bom-pcbway.md` | Shopping lists by store, and a PCBWay-ready BOM |
| `docs/bringup-checklist.md` | Bench steps in order, each with what to expect |
| `docs/designer-plan.md` | Plan for the online gauge designer and the `.gauge` file |
| `hardware/` | Schematics (Eagle XML, importable into EasyEDA) and the PCBWay BOMs |

Haltech's CAN protocol document is not included; `docs/ref/README.md` says where to get it.

## Fonts

The five bundled fonts are under the SIL Open Font License; each has its licence file
next to it in `assets/fonts`.
