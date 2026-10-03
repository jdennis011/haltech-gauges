// Hub CAN self-test (bring-up step 5a).
//
// Wire the two transceivers together (H to H, L to L) with 120 ohm at each
// end. Each controller sends 2000 numbered frames a second to the other; any
// gap in the numbers received is a lost frame. Shorting H to L should show
// bus-off and then recovery once the short is removed.

#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

#include "../hub/board_pins.h"

#ifndef TWAI_CAN_NEW_DRIVER
#error "The hub needs a chip with two TWAI controllers (ESP32-C6) on ESP-IDF 5.5 or later"
#endif

namespace {

const uint32_t kFramesPerMs = 2;
const uint32_t kTestSeconds = 60;

struct Side {
    TwaiCAN* can;
    const char* name;
    uint32_t txId;
    volatile uint32_t sent;
    volatile uint32_t sendRejected;
    volatile uint32_t received;
    volatile uint32_t gaps;
    volatile uint32_t recoveries;
    uint32_t nextExpected;
    bool haveFirst;
};

Side sides[2] = {
    {&CAN1, "CAN1", 0x101, 0, 0, 0, 0, 0, 0, false},
    {&CAN2, "CAN2", 0x102, 0, 0, 0, 0, 0, 0, false},
};

void txTask(void* arg) {
    Side* side = static_cast<Side*>(arg);
    uint32_t seq = 0;
    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        for (uint32_t i = 0; i < kFramesPerMs; i++) {
            CanFrame f = {};
            f.identifier = side->txId;
            f.data_length_code = 8;
            f.data[0] = uint8_t(seq >> 24);
            f.data[1] = uint8_t(seq >> 16);
            f.data[2] = uint8_t(seq >> 8);
            f.data[3] = uint8_t(seq);
            if (side->can->writeFrame(f, 0)) {
                side->sent = side->sent + 1;
                seq++;
            } else {
                side->sendRejected = side->sendRejected + 1;
            }
        }
        vTaskDelayUntil(&wake, 1);
    }
}

void rxTask(void* arg) {
    Side* side = static_cast<Side*>(arg);
    CanFrame f;
    for (;;) {
        if (!side->can->readFrame(f, 100)) continue;
        const uint32_t seq = (uint32_t(f.data[0]) << 24) | (uint32_t(f.data[1]) << 16) |
                             (uint32_t(f.data[2]) << 8) | f.data[3];
        if (side->haveFirst && seq != side->nextExpected) {
            side->gaps = side->gaps + (seq - side->nextExpected);
        }
        side->haveFirst = true;
        side->nextExpected = seq + 1;
        side->received = side->received + 1;
    }
}

void service(Side& side) {
    const uint32_t state = side.can->canState();
    if (state == TWAI_STATE_BUS_OFF) {
        side.can->recover();
        side.recoveries = side.recoveries + 1;
    } else if (state == TWAI_STATE_STOPPED) {
        side.can->restart();
    }
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("Hub CAN self-test: CAN1 <-> CAN2 at 1 Mbit/s");

    const bool ok1 = CAN1.begin(TWAI_SPEED_1000KBPS, PIN_ECU_CAN_TX, PIN_ECU_CAN_RX, 64, 512);
    const bool ok2 = CAN2.begin(TWAI_SPEED_1000KBPS, PIN_GAUGE_CAN_TX, PIN_GAUGE_CAN_RX, 64, 512);
    Serial.printf("CAN1 %s, CAN2 %s\n", ok1 ? "started" : "FAILED", ok2 ? "started" : "FAILED");

    for (Side& side : sides) {
        xTaskCreate(rxTask, "rx", 4096, &side, 20, nullptr);
        xTaskCreate(txTask, "tx", 4096, &side, 10, nullptr);
    }
}

void loop() {
    static uint32_t seconds = 0;
    delay(1000);
    seconds++;

    for (Side& side : sides) {
        service(side);
        TwaiDiagnostics d = {};
        side.can->getDiagnostics(&d);
        Serial.printf("%s: sent %lu (rejected %lu)  received %lu  gaps %lu  queue-missed %lu  "
                      "bus-errors %lu  state %lu  recoveries %lu\n",
                      side.name, (unsigned long)side.sent, (unsigned long)side.sendRejected,
                      (unsigned long)side.received, (unsigned long)side.gaps,
                      (unsigned long)d.rxMissed, (unsigned long)d.busErrors,
                      (unsigned long)side.can->canState(), (unsigned long)side.recoveries);
    }

    if (seconds == kTestSeconds) {
        const uint32_t gaps = sides[0].gaps + sides[1].gaps;
        const bool enough = sides[0].received > kTestSeconds * 1900 && sides[1].received > kTestSeconds * 1900;
        Serial.printf("=== %lu s result: %s (gaps %lu) ===\n", (unsigned long)kTestSeconds,
                      (gaps == 0 && enough) ? "PASS" : "FAIL", (unsigned long)gaps);
    }
}
