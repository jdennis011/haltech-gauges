// Turns a Waveshare ESP32-P4-WIFI6 into a serial bridge to its own ESP32-C6
// Wi-Fi module, with the C6 held in download mode, so esptool on the board's
// USB-C port can reflash the C6 through the P4 (platformio.ini, env
// hub_p4_c6bridge; electrical-spec.md 5.4). For when there is no USB-serial
// adapter to put on the C6's 4-pin header.
//
// Wiring, three jumpers from the P4 header to the 4-pin C6 header:
//   P4 GPIO26 (header pin 29)  ->  C6 header RXD
//   P4 GPIO27 (header pin 27)  <-  C6 header TXD
//   P4 GPIO2  (header pin 15)  ->  C6 header IO9      (held low: download mode)
// GND is shared on the board. The C6's reset is the P4's own GPIO54.
//
// Then, with the baud pinned so the C6 is never asked to change it:
//   esptool --chip esp32c6 --port COM3 --baud 115200 --before no-reset --after no-reset
//       write-flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 network_adapter.bin
//
// If esptool cannot connect, the reset polarity is the other way round for
// this board: build with -DC6_RESET_ASSERTED=LOW.

#include <Arduino.h>
#include <HardwareSerial.h>

#ifndef C6_RESET_ASSERTED
#define C6_RESET_ASSERTED HIGH  // ESP-Hosted's default for the P4: reset asserted by driving GPIO54 high
#endif

namespace {

const int kPinToC6Rxd = 26;
const int kPinFromC6Txd = 27;
const int kPinC6Io9 = 2;
const int kPinC6Reset = 54;

HardwareSerial c6(1);

}  // namespace

void setup() {
    Serial.setRxBufferSize(8192);
    Serial.begin(115200);
    c6.setRxBufferSize(8192);
    c6.begin(115200, SERIAL_8N1, kPinFromC6Txd, kPinToC6Rxd);

    // Download mode: IO9 low while the C6 comes out of reset.
    pinMode(kPinC6Io9, OUTPUT);
    digitalWrite(kPinC6Io9, LOW);
    pinMode(kPinC6Reset, OUTPUT);
    digitalWrite(kPinC6Reset, C6_RESET_ASSERTED);
    delay(200);
    digitalWrite(kPinC6Reset, C6_RESET_ASSERTED == HIGH ? LOW : HIGH);
    delay(300);
    // Anything the C6 ROM printed on boot is thrown away so esptool starts clean.
    while (c6.available()) c6.read();
    Serial.println("C6 bridge: 115200, download mode. Run esptool on this port with --before no-reset --after no-reset");
}

void loop() {
    uint8_t buf[256];
    while (Serial.available()) {
        const size_t n = Serial.readBytes(buf, min(sizeof(buf), size_t(Serial.available())));
        c6.write(buf, n);
    }
    while (c6.available()) {
        const size_t n = c6.readBytes(buf, min(sizeof(buf), size_t(c6.available())));
        Serial.write(buf, n);
    }
}
