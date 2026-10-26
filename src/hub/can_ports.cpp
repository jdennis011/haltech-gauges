#include "can_ports.h"

#include <string.h>

#include "board_pins.h"
#include "ip_bus.h"

namespace {

#if HG_HAS_ECU_PORT
// The driver assigns hardware controllers in begin() order: CAN1 then CAN2.
TwaiCAN& ecuCan = CAN1;
TwaiCAN& gaugeCan = CAN2;
#else
TwaiCAN& gaugeCan = ESP32Can;
#endif

const uint16_t kEcuRxQueue = 512;
const uint16_t kGaugeRxQueue = 128;
const uint16_t kGaugeTxSlots = 64;
// The gauge controller is bus-off (no transceiver on the bench, a shorted
// pair): nothing is handed to it until service() has it running again, or
// the driver logs a refusal for every one of the simulator's 1400 frames a
// second and that flood starves the rest of the hub.
bool gaugeBusOff = false;
#if HG_HAS_FG_PORT
TwaiCAN fgCan;  // the third controller, taken by begin() after the other two
const uint16_t kFgTxSlots = 16;
const uint16_t kFgRxQueue = 64;
#endif

bool readFrom(TwaiCAN& can, hg::CanFrame& out, uint32_t timeoutMs) {
    CanFrame in;
    if (!can.readFrame(in, timeoutMs)) return false;
    out.id = in.identifier;
    out.extended = in.extd;
    out.len = in.data_length_code > 8 ? 8 : in.data_length_code;
    memcpy(out.data, in.data, out.len);
    return true;
}

CanPortStats statsOf(TwaiCAN& can) {
    CanPortStats s = {};
#if HG_HAS_ECU_PORT
    TwaiDiagnostics d;
    if (can.getDiagnostics(&d)) {
        s.rxMissed = d.rxMissed;
        s.txRejected = d.rejected;
        s.busErrors = d.busErrors;
        s.txErrors = d.txErrors;
        s.rxErrors = d.rxErrors;
    }
#else
    twai_status_info_t info;
    if (can.getStatus(&info)) {
        s.rxMissed = info.rx_missed_count;
        s.txRejected = info.tx_failed_count;
        s.busErrors = info.bus_error_count;
        s.txErrors = uint16_t(info.tx_error_counter);
        s.rxErrors = uint16_t(info.rx_error_counter);
    }
#endif
    s.state = can.canState();
    return s;
}

void serviceCan(TwaiCAN& can) {
    const uint32_t state = can.canState();
    if (state == TWAI_STATE_BUS_OFF) {
        can.recover();
    } else if (state == TWAI_STATE_STOPPED) {
        can.restart();
    }
}

}  // namespace

#if HG_HAS_ECU_PORT

bool EcuPort::begin() {
    // Transceiver in normal mode so the controller acknowledges the ECU's frames.
    pinMode(PIN_ECU_CAN_SILENT, OUTPUT);
    digitalWrite(PIN_ECU_CAN_SILENT, LOW);
    // Transmit queue kept at the minimum: nothing is ever sent on this bus.
    return ecuCan.begin(TWAI_SPEED_1000KBPS, PIN_ECU_CAN_TX, PIN_ECU_CAN_RX, 1, kEcuRxQueue);
}

bool EcuPort::read(hg::CanFrame& frame, uint32_t timeoutMs) {
    return readFrom(ecuCan, frame, timeoutMs);
}

CanPortStats EcuPort::stats() { return statsOf(ecuCan); }

void EcuPort::service() { serviceCan(ecuCan); }

#endif

bool GaugePort::begin() {
    return gaugeCan.begin(TWAI_SPEED_1000KBPS, PIN_GAUGE_CAN_TX, PIN_GAUGE_CAN_RX, kGaugeTxSlots,
                          kGaugeRxQueue);
}

bool GaugePort::read(hg::CanFrame& frame, uint32_t timeoutMs) {
    return readFrom(gaugeCan, frame, timeoutMs);
}

bool GaugePort::write(const hg::CanFrame& frame) {
    // Gauges linked over Wi-Fi get every frame too (docs/ip-link.md). With
    // nothing on the wire acknowledging, the controller's queue fills and
    // stays full, so it is left alone then rather than refused for ever.
    const bool viaIp = ip_bus::send(frame);
    if (gaugeBusOff || gaugeCan.inTxQueue() + 2 >= kGaugeTxSlots) return viaIp;
    CanFrame out = {};
    out.identifier = frame.id;
    out.extd = frame.extended;
    out.data_length_code = frame.len;
    memcpy(out.data, frame.data, frame.len);
    return gaugeCan.writeFrame(out, 0) || viaIp;
}

uint32_t GaugePort::txPending() { return gaugeCan.inTxQueue(); }

CanPortStats GaugePort::stats() { return statsOf(gaugeCan); }

void GaugePort::service() {
    const uint32_t state = gaugeCan.canState();
    gaugeBusOff = state == TWAI_STATE_BUS_OFF || state == TWAI_STATE_RECOVERING || state == TWAI_STATE_STOPPED;
    serviceCan(gaugeCan);
}

#if HG_HAS_FG_PORT

bool FgPort::begin(TwaiSpeed speed) {
    pinMode(PIN_FG_CAN_SILENT, OUTPUT);
    digitalWrite(PIN_FG_CAN_SILENT, LOW);
    return fgCan.begin(speed, PIN_FG_CAN_TX, PIN_FG_CAN_RX, kFgTxSlots, kFgRxQueue);
}

bool FgPort::read(hg::CanFrame& frame, uint32_t timeoutMs) { return readFrom(fgCan, frame, timeoutMs); }

bool FgPort::write(const hg::CanFrame& frame) {
    if (fgCan.inTxQueue() + 2 >= kFgTxSlots) return false;  // nobody acknowledging: do not pile up
    CanFrame out = {};
    out.identifier = frame.id;
    out.extd = frame.extended;
    out.data_length_code = frame.len;
    memcpy(out.data, frame.data, frame.len);
    return fgCan.writeFrame(out, 0);
}

void FgPort::setListenOnly(bool on) { digitalWrite(PIN_FG_CAN_SILENT, on ? HIGH : LOW); }

CanPortStats FgPort::stats() { return statsOf(fgCan); }

void FgPort::service() { serviceCan(fgCan); }

#endif
