# The gauge bus over Wi-Fi

A gauge can reach the hub over Wi-Fi instead of the CAN chain: for a gauge on the bench
with no transceiver (the M5Stack Dial on a USB lead), or one mounted too far away for the
chain. The hub treats it exactly like a gauge on the bus: it shows up in the roster,
gets its config, images, alerts, the intro and the time, and the page works on it.

> A gauge that is linked but hears nothing from the hub for 15 s drops the link and joins
> again on its own (seen once on the bench after a reset: joined, addressed, sending, unheard).
> While linked it prints a line every 10 s with packets sent, send failures, its address and
> the signal strength.

## How it works

- The hub listens on UDP port 4510 on both its networks, the access point and the home
  network. Every frame it puts on the gauge bus (beacons, commands, transfers, forwarded
  live data, the simulator) also goes to each linked gauge, a few frames to a datagram.
  Frames from linked gauges join the ones from the bus and go through the same code.
- A datagram is `H` `G`, a version byte, a count, then up to 64 frames of 13 bytes: the
  id (big-endian, bit 31 for an extended id), the length, eight data bytes. An empty
  datagram is a keepalive; the hub forgets a gauge it has not heard from for 10 s.
- The gauge joins the network, sends a keepalive every 2 s so the hub knows where it is,
  then waits for a beacon like any gauge. It keeps sending over CAN too, so a gauge with
  both works on either.
- With nothing on the wire to acknowledge, the hub's CAN controller cannot send; the hub
  leaves it alone once its queue fills and the linked gauges still get everything.
- The link needs no account, no token and no internet: the gauge joins the hub's own
  access point, `HaltechGauges` / `haltech123`, where the hub is `192.168.4.1`. It can
  join the home network instead and be given the hub's address there.

## Setting it up on a gauge

On the gauge's USB serial console (115200 baud; `?` lists the commands):

```
link HaltechGauges haltech123            # the hub's access point; the hub is 192.168.4.1
link MyHomeWifi secret 192.168.1.50      # the home network, naming the hub's address
link                                     # what it is doing
link off
```

The settings are kept in the gauge's flash, so it links again at every start. The
five-second report on the console shows the link's state.

## On the page

A linked gauge's card carries an "over Wi-Fi" chip. The hub's status reports the linked
gauges under `ip_bus`: `{port, peers: [{ip, node, age_s}], rx_frames, tx_packets, dropped}`.

## Limits

- Wi-Fi is slower to react than CAN and loses the odd datagram; the protocol's
  acknowledgements and repeats cover that, as they do for a noisy bus. Live data goes over
  the link as the newest frame of each id, twenty times a second (a datagram of up to 64
  frames), rather than every frame the bus carries: all a gauge can show, and a load a
  small board handles. The M5Stack Dial drew at 2 to 5 fps when fed every frame.
- Images and firmware over Wi-Fi (docs/images.md) are a separate mechanism: a linked gauge
  that is already on the hub's access point stays on it for those; one on the home network
  would be asked to switch to the hub's access point and back.
- This is for the bench and the odd remote gauge. The chain stays the design for the dash:
  it carries power, needs no joining, and is deterministic.
