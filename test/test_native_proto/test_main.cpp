#include <string.h>
#include <unity.h>

#include <deque>
#include <vector>

#include "crc32.h"
#include "proto.h"
#include "xfer.h"

using namespace hg;
using namespace hg::proto;

void setUp() {}
void tearDown() {}

// ------------------------------------------------------------ messages

void test_crc32_known_vector() {
    const char* s = "123456789";
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926, crc32((const uint8_t*)s, 9));
    // Continuing over two buffers gives the same result.
    uint32_t part = crc32((const uint8_t*)s, 4);
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926, crc32((const uint8_t*)s + 4, 5, part));
    TEST_ASSERT_EQUAL_HEX32(0, crc32(nullptr, 0));
}

void test_id_fields_round_trip() {
    const uint32_t id = makeId(Msg::XferAck, Dir::GaugeToHub, 0xABCDEF);
    TEST_ASSERT_TRUE(id <= 0x1FFFFFFF);
    TEST_ASSERT_EQUAL(int(Msg::XferAck), int(idMsg(id)));
    TEST_ASSERT_EQUAL(int(Dir::GaugeToHub), int(idDir(id)));
    TEST_ASSERT_EQUAL_HEX32(0xABCDEF, idNode(id));
    TEST_ASSERT_TRUE(makeId(Msg::XferDataLast, Dir::GaugeToHub, kBroadcastNode) <= 0x1FFFFFFF);
}

void test_messages_round_trip() {
    Beacon b;
    b.flags = kBeaconFlagSimulator;
    b.uptimeSec = 0x1234;
    Beacon b2;
    CanFrame f = pack(b);
    TEST_ASSERT_TRUE(f.extended);
    TEST_ASSERT_EQUAL_HEX32(kBroadcastNode, idNode(f.id));
    TEST_ASSERT_TRUE(unpack(f, b2));
    TEST_ASSERT_EQUAL(0x1234, b2.uptimeSec);
    TEST_ASSERT_EQUAL(kBeaconFlagSimulator, b2.flags);

    Hello h;
    const uint8_t mac[6] = {1, 2, 3, 4, 5, 6};
    memcpy(h.mac, mac, 6);
    h.schemaVersion = 7;
    Hello h2;
    TEST_ASSERT_TRUE(unpack(pack(h, 0x040506), h2));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(mac, h2.mac, 6);
    TEST_ASSERT_EQUAL(7, h2.schemaVersion);
    TEST_ASSERT_EQUAL(0, packHelloRequest(kBroadcastNode).len);

    Status s;
    s.configCrc = 0xDEADBEEF;
    s.activeFace = 2;
    s.faceCount = 5;
    s.vbusMillivolts = 4875;
    s.flags = kStatusFlagTrying;
    Status s2;
    TEST_ASSERT_TRUE(unpack(pack(s, 1), s2));
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, s2.configCrc);
    TEST_ASSERT_EQUAL(2, s2.activeFace);
    TEST_ASSERT_EQUAL(5, s2.faceCount);
    TEST_ASSERT_EQUAL(4875, s2.vbusMillivolts);
    TEST_ASSERT_EQUAL(kStatusFlagTrying, s2.flags);

    XferBegin xb;
    xb.session = 9;
    xb.kind = XferKind::TryConfig;
    xb.size = 0x123456;
    xb.blockFrames = 32;
    XferBegin xb2;
    TEST_ASSERT_TRUE(unpack(pack(xb, Dir::HubToGauge, 1), xb2));
    TEST_ASSERT_EQUAL(9, xb2.session);
    TEST_ASSERT_EQUAL(int(XferKind::TryConfig), int(xb2.kind));
    TEST_ASSERT_EQUAL_HEX32(0x123456, xb2.size);

    XferAck xa;
    xa.session = 3;
    xa.status = XferStatus::CrcFail;
    xa.nextFrame = 70000;
    XferAck xa2;
    TEST_ASSERT_TRUE(unpack(pack(xa, Dir::GaugeToHub, 1), xa2));
    TEST_ASSERT_EQUAL(int(XferStatus::CrcFail), int(xa2.status));
    TEST_ASSERT_EQUAL(70000, xa2.nextFrame);

    // Truncated frames are refused.
    CanFrame shortFrame = pack(s, 1);
    shortFrame.len = 3;
    TEST_ASSERT_FALSE(unpack(shortFrame, s2));
}

// ------------------------------------------------------------ transfer

namespace {

const uint32_t kNode = 0x123456;

// A simulated CAN link in each direction, with loss, duplication and a
// bounded queue so send() sometimes reports "no room".
struct Wire {
    std::deque<CanFrame> queue;
    size_t capacity = 16;
    int dropPermille = 0;
    int dupPermille = 0;
    int corruptDataFrame = -1;  // index of the data frame to damage once
    int dataFramesSeen = 0;
    int dropAcksWithStatus = -1;  // drop the first ack carrying this status
    uint32_t rng = 1;
    uint32_t sent = 0;

    uint32_t next() {
        rng = rng * 1664525u + 1013904223u;
        return rng >> 8;
    }

    static bool send(const CanFrame& frame, void* ctx) {
        Wire* w = static_cast<Wire*>(ctx);
        if (w->queue.size() >= w->capacity) return false;
        w->sent++;
        CanFrame f = frame;
        const Msg msg = idMsg(f.id);
        if (msg == Msg::XferData || msg == Msg::XferDataLast) {
            if (w->dataFramesSeen++ == w->corruptDataFrame) f.data[1] ^= 0xFF;
        }
        if (msg == Msg::XferAck && w->dropAcksWithStatus >= 0 &&
            f.data[1] == uint8_t(w->dropAcksWithStatus)) {
            w->dropAcksWithStatus = -1;
            return true;
        }
        if (int(w->next() % 1000) < w->dropPermille) return true;  // lost on the wire
        w->queue.push_back(f);
        if (int(w->next() % 1000) < w->dupPermille) w->queue.push_back(f);
        return true;
    }
};

struct Sink {
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> received;
    int completed = 0;
    int dropped = 0;
    XferStatus acceptStatus = XferStatus::Ok;
    bool applyResult = true;
    XferKind kind = XferKind::Config;

    static XferStatus accept(XferKind kind, uint32_t size, uint8_t** buffer, void* ctx) {
        Sink* s = static_cast<Sink*>(ctx);
        if (s->acceptStatus != XferStatus::Ok) return s->acceptStatus;
        s->kind = kind;
        s->buffer.assign(size ? size : 1, 0xEE);
        *buffer = s->buffer.data();
        return XferStatus::Ok;
    }
    static bool complete(XferKind, const uint8_t* data, uint32_t size, void* ctx) {
        Sink* s = static_cast<Sink*>(ctx);
        s->received.assign(data, data + size);
        s->completed++;
        return s->applyResult;
    }
    static void droppedCb(void* ctx) { static_cast<Sink*>(ctx)->dropped++; }
};

struct Rig {
    Wire toReceiver, toSender;
    Sink sink;
    XferSender sender;
    XferReceiver receiver;
    uint32_t now = 1000;

    Rig() { initReceiver(); }

    void initReceiver() {
        XferReceiver::Handler h = {Sink::accept, Sink::complete, Sink::droppedCb, &sink};
        receiver.init({Wire::send, &toSender}, kNode, Dir::HubToGauge, h);
    }

    void start(const std::vector<uint8_t>& payload, uint8_t session = 1,
               XferKind kind = XferKind::Config) {
        sender.start({Wire::send, &toReceiver}, kNode, Dir::HubToGauge, session, kind,
                     payload.data(), uint32_t(payload.size()), now);
    }

    // One simulated millisecond. A 1 Mbit/s bus carries about 7 frames per ms.
    void step() {
        now++;
        sender.poll(now);
        for (int i = 0; i < 7 && !toReceiver.queue.empty(); i++) {
            receiver.onFrame(toReceiver.queue.front(), now);
            toReceiver.queue.pop_front();
        }
        receiver.poll(now);
        for (int i = 0; i < 7 && !toSender.queue.empty(); i++) {
            sender.onFrame(toSender.queue.front(), now);
            toSender.queue.pop_front();
        }
    }

    // Runs until the sender stops; returns elapsed simulated ms.
    uint32_t run(uint32_t limitMs = 120000) {
        const uint32_t begin = now;
        while (sender.busy() && now - begin < limitMs) step();
        return now - begin;
    }
};

std::vector<uint8_t> makePayload(size_t size, uint32_t seed = 7) {
    std::vector<uint8_t> p(size);
    uint32_t x = seed;
    for (size_t i = 0; i < size; i++) {
        x = x * 1103515245u + 12345u;
        p[i] = uint8_t(x >> 16);
    }
    return p;
}

void assertDelivered(Rig& rig, const std::vector<uint8_t>& payload) {
    TEST_ASSERT_EQUAL(int(XferSender::State::Done), int(rig.sender.state()));
    TEST_ASSERT_EQUAL(int(XferStatus::Done), int(rig.sender.result()));
    TEST_ASSERT_EQUAL(1, rig.sink.completed);
    TEST_ASSERT_EQUAL(payload.size(), rig.sink.received.size());
    if (!payload.empty()) {
        TEST_ASSERT_EQUAL_UINT8_ARRAY(payload.data(), rig.sink.received.data(), payload.size());
    }
    TEST_ASSERT_FALSE(rig.receiver.active());
}

}  // namespace

void test_request_round_trip() {
    Request rq;
    rq.cmd = Cmd::SetBrightness;
    rq.arg = 180;
    rq.all = true;
    const CanFrame f = pack(rq, 0x040506);
    TEST_ASSERT_EQUAL(int(Dir::GaugeToHub), int(idDir(f.id)));
    TEST_ASSERT_EQUAL(int(Msg::Command), int(idMsg(f.id)));
    Request rq2;
    TEST_ASSERT_TRUE(unpack(f, rq2));
    TEST_ASSERT_EQUAL(int(Cmd::SetBrightness), int(rq2.cmd));
    TEST_ASSERT_EQUAL(180, rq2.arg);
    TEST_ASSERT_TRUE(rq2.all);
    // A plain two-byte command is not a request.
    Command c;
    c.cmd = Cmd::SetDisplay;
    c.arg = 0;
    TEST_ASSERT_FALSE(unpack(pack(c, 0x040506), rq2));
}

void test_alert_round_trip() {
    Alert a;
    a.show = true;
    a.color = 0x12AB34;
    strcpy(a.text, "COOLANT HOT: PULL OVER NOW 12345");  // the full 32 characters
    CanFrame frames[kAlertMaxFrames];
    size_t count = packAlert(a, 5, 0x040506, frames);
    TEST_ASSERT_EQUAL(6, count);
    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_TRUE(frames[i].extended);
        TEST_ASSERT_EQUAL(int(Msg::Alert), int(idMsg(frames[i].id)));
        TEST_ASSERT_EQUAL(int(Dir::HubToGauge), int(idDir(frames[i].id)));
        TEST_ASSERT_EQUAL_HEX32(0x040506, idNode(frames[i].id));
    }

    AlertReceiver rx;
    TEST_ASSERT_FALSE(rx.current().show);
    for (size_t i = 0; i + 1 < count; i++) TEST_ASSERT_FALSE(rx.onFrame(frames[i], 1000));
    TEST_ASSERT_TRUE(rx.onFrame(frames[count - 1], 1000));
    TEST_ASSERT_TRUE(rx.current().show);
    TEST_ASSERT_EQUAL_STRING(a.text, rx.current().text);
    TEST_ASSERT_EQUAL_HEX32(0x12AB34, rx.current().color);
    // The repeats that keep it up are not a change.
    for (size_t i = 0; i < count; i++) TEST_ASSERT_FALSE(rx.onFrame(frames[i], 1500));

    // A different, shorter message replaces it.
    Alert b;
    b.show = true;
    strcpy(b.text, "SHIFT");
    count = packAlert(b, 6, 0x040506, frames);
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_FALSE(rx.onFrame(frames[0], 2000));
    TEST_ASSERT_TRUE(rx.onFrame(frames[1], 2000));
    TEST_ASSERT_EQUAL_STRING("SHIFT", rx.current().text);

    // A clear is one frame and takes it down at once.
    Alert clear;
    count = packAlert(clear, 6, 0x040506, frames);
    TEST_ASSERT_EQUAL(1, count);
    TEST_ASSERT_TRUE(rx.onFrame(frames[0], 2100));
    TEST_ASSERT_FALSE(rx.current().show);
    TEST_ASSERT_FALSE(rx.onFrame(frames[0], 2200));
}

void test_alert_survives_lost_frames_and_times_out() {
    Alert a;
    a.show = true;
    strcpy(a.text, "OIL PRESSURE LOW");  // three frames of text
    CanFrame frames[kAlertMaxFrames];
    const size_t count = packAlert(a, 1, 0x040506, frames);
    TEST_ASSERT_EQUAL(4, count);

    // The middle text frame is lost: nothing shows until a repeat fills it in.
    AlertReceiver rx;
    rx.onFrame(frames[0], 0);
    rx.onFrame(frames[1], 0);
    rx.onFrame(frames[3], 0);
    TEST_ASSERT_FALSE(rx.current().show);
    TEST_ASSERT_FALSE(rx.onFrame(frames[0], 1000));
    TEST_ASSERT_FALSE(rx.onFrame(frames[1], 1000));
    TEST_ASSERT_TRUE(rx.onFrame(frames[2], 1000));
    TEST_ASSERT_EQUAL_STRING("OIL PRESSURE LOW", rx.current().text);

    // Repeats keep it up; three seconds after the last one it comes down.
    TEST_ASSERT_FALSE(rx.poll(3900));
    for (size_t i = 0; i < count; i++) rx.onFrame(frames[i], 3900);
    TEST_ASSERT_FALSE(rx.poll(6800));
    TEST_ASSERT_TRUE(rx.poll(6900));
    TEST_ASSERT_FALSE(rx.current().show);
    TEST_ASSERT_FALSE(rx.poll(7000));

    // Text frames of a message whose header never arrived show nothing.
    AlertReceiver deaf;
    for (size_t i = 1; i < count; i++) TEST_ASSERT_FALSE(deaf.onFrame(frames[i], 0));
    TEST_ASSERT_FALSE(deaf.current().show);
}

void test_theme_colours_round_trip() {
    const uint32_t colours[kThemeColourCount] = {0xFFFFFF, 0xFF8C00, 0x123456, 0xABCDEF,
                                                 0x000001, 0x800000, 0x00FF00, 0x000000};
    CanFrame frames[kThemeColourFrames];
    TEST_ASSERT_EQUAL(4, packThemeColours(colours, kBroadcastNode, frames));
    uint32_t got[kThemeColourCount] = {};
    for (const CanFrame& f : frames) {
        TEST_ASSERT_EQUAL(int(Msg::ThemeColours), int(idMsg(f.id)));
        TEST_ASSERT_EQUAL_HEX32(kBroadcastNode, idNode(f.id));
        TEST_ASSERT_TRUE(unpackThemeColours(f, got));
    }
    TEST_ASSERT_EQUAL_HEX32_ARRAY(colours, got, kThemeColourCount);

    CanFrame bad = frames[0];
    bad.data[0] = 4;
    TEST_ASSERT_FALSE(unpackThemeColours(bad, got));
    bad = frames[0];
    bad.len = 6;
    TEST_ASSERT_FALSE(unpackThemeColours(bad, got));
}

void test_xfer_small_payload() {
    Rig rig;
    auto payload = makePayload(10);
    rig.start(payload, 1, XferKind::TryConfig);
    rig.run();
    assertDelivered(rig, payload);
    TEST_ASSERT_EQUAL(int(XferKind::TryConfig), int(rig.sink.kind));
}

void test_xfer_edge_sizes() {
    for (size_t size : {size_t(0), size_t(1), size_t(7), size_t(8), size_t(14), size_t(224), size_t(225)}) {
        Rig rig;
        auto payload = makePayload(size);
        rig.start(payload);
        rig.run();
        assertDelivered(rig, payload);
    }
}

void test_xfer_4k_config_is_fast_on_a_clean_link() {
    Rig rig;
    auto payload = makePayload(4096);
    rig.start(payload);
    const uint32_t ms = rig.run();
    assertDelivered(rig, payload);
    TEST_ASSERT_LESS_THAN(1000, ms);
}

void test_xfer_survives_loss_and_duplicates() {
    for (uint32_t seed = 1; seed <= 25; seed++) {
        Rig rig;
        rig.toReceiver.rng = seed;
        rig.toSender.rng = seed * 977;
        rig.toReceiver.dropPermille = rig.toSender.dropPermille = 10;
        rig.toReceiver.dupPermille = rig.toSender.dupPermille = 10;
        auto payload = makePayload(4096, seed);
        rig.start(payload, uint8_t(seed));
        rig.run();
        assertDelivered(rig, payload);
    }
}

void test_xfer_large_payload_with_loss() {
    Rig rig;
    rig.toReceiver.dropPermille = rig.toSender.dropPermille = 10;
    auto payload = makePayload(100 * 1024);
    rig.start(payload);
    rig.run(600000);
    assertDelivered(rig, payload);
}

void test_xfer_heavy_loss_never_delivers_bad_data() {
    int delivered = 0;
    for (uint32_t seed = 1; seed <= 20; seed++) {
        Rig rig;
        rig.toReceiver.rng = seed;
        rig.toSender.rng = seed + 100;
        rig.toReceiver.dropPermille = rig.toSender.dropPermille = 200;
        auto payload = makePayload(2000, seed);
        rig.start(payload);
        rig.run(600000);
        TEST_ASSERT_FALSE(rig.sender.busy());
        if (rig.sink.completed) {
            delivered++;
            TEST_ASSERT_EQUAL(1, rig.sink.completed);
            TEST_ASSERT_EQUAL_UINT8_ARRAY(payload.data(), rig.sink.received.data(), payload.size());
        } else {
            TEST_ASSERT_EQUAL(int(XferSender::State::Failed), int(rig.sender.state()));
        }
    }
    // 20% loss each way is far beyond a real bus; most attempts should still get through.
    TEST_ASSERT_GREATER_THAN(10, delivered);
}

void test_xfer_corrupted_frame_fails_crc() {
    Rig rig;
    rig.toReceiver.corruptDataFrame = 5;
    auto payload = makePayload(500);
    rig.start(payload);
    rig.run();
    TEST_ASSERT_EQUAL(int(XferSender::State::Failed), int(rig.sender.state()));
    TEST_ASSERT_EQUAL(int(XferStatus::CrcFail), int(rig.sender.result()));
    TEST_ASSERT_EQUAL(0, rig.sink.completed);
    TEST_ASSERT_EQUAL(1, rig.sink.dropped);
}

void test_xfer_receiver_refuses() {
    Rig rig;
    rig.sink.acceptStatus = XferStatus::TooBig;
    auto payload = makePayload(500);
    rig.start(payload);
    rig.run();
    TEST_ASSERT_EQUAL(int(XferStatus::TooBig), int(rig.sender.result()));
    TEST_ASSERT_EQUAL(0, rig.sink.completed);
}

void test_xfer_payload_rejected_after_arrival() {
    Rig rig;
    rig.sink.applyResult = false;
    auto payload = makePayload(500);
    rig.start(payload);
    rig.run();
    TEST_ASSERT_EQUAL(int(XferStatus::Rejected), int(rig.sender.result()));
    TEST_ASSERT_EQUAL(1, rig.sink.completed);
}

void test_xfer_lost_final_ack_does_not_apply_twice() {
    Rig rig;
    rig.toSender.dropAcksWithStatus = int(XferStatus::Done);
    auto payload = makePayload(500);
    rig.start(payload);
    rig.run();
    assertDelivered(rig, payload);  // completed exactly once despite the repeated XferEnd
}

void test_xfer_receiver_reset_mid_transfer() {
    Rig rig;
    auto payload = makePayload(20000);
    rig.start(payload);
    for (int i = 0; i < 100; i++) rig.step();
    TEST_ASSERT_TRUE(rig.sender.busy());
    TEST_ASSERT_TRUE(rig.receiver.active());

    rig.receiver = XferReceiver();  // the gauge rebooted
    rig.initReceiver();
    rig.run();
    TEST_ASSERT_EQUAL(int(XferSender::State::Failed), int(rig.sender.state()));
    TEST_ASSERT_EQUAL(int(XferStatus::NoSession), int(rig.sender.result()));
    TEST_ASSERT_EQUAL(0, rig.sink.completed);
}

void test_xfer_sender_vanishes_mid_transfer() {
    Rig rig;
    auto payload = makePayload(20000);
    rig.start(payload);
    for (int i = 0; i < 100; i++) rig.step();
    TEST_ASSERT_TRUE(rig.receiver.active());

    // The hub rebooted: nothing more arrives.
    rig.toReceiver.queue.clear();
    for (int i = 0; i < 4000; i++) {
        rig.now++;
        rig.receiver.poll(rig.now);
    }
    TEST_ASSERT_FALSE(rig.receiver.active());
    TEST_ASSERT_EQUAL(1, rig.sink.dropped);
    TEST_ASSERT_EQUAL(0, rig.sink.completed);
}

void test_xfer_new_session_replaces_stale_one() {
    Rig rig;
    auto first = makePayload(20000, 1);
    rig.start(first, 1);
    for (int i = 0; i < 100; i++) rig.step();

    // The hub restarts the push with a new session while the gauge still holds the old one.
    rig.toReceiver.queue.clear();
    rig.toSender.queue.clear();
    auto second = makePayload(3000, 2);
    rig.start(second, 2);
    rig.run();
    assertDelivered(rig, second);
    TEST_ASSERT_EQUAL(1, rig.sink.dropped);
}

void test_xfer_no_receiver_times_out() {
    Rig rig;
    rig.toReceiver.dropPermille = 1000;  // nothing is listening
    auto payload = makePayload(100);
    rig.start(payload);
    const uint32_t ms = rig.run();
    TEST_ASSERT_EQUAL(int(XferStatus::Timeout), int(rig.sender.result()));
    TEST_ASSERT_LESS_THAN(5000, ms);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_crc32_known_vector);
    RUN_TEST(test_id_fields_round_trip);
    RUN_TEST(test_messages_round_trip);
    RUN_TEST(test_request_round_trip);
    RUN_TEST(test_alert_round_trip);
    RUN_TEST(test_alert_survives_lost_frames_and_times_out);
    RUN_TEST(test_theme_colours_round_trip);
    RUN_TEST(test_xfer_small_payload);
    RUN_TEST(test_xfer_edge_sizes);
    RUN_TEST(test_xfer_4k_config_is_fast_on_a_clean_link);
    RUN_TEST(test_xfer_survives_loss_and_duplicates);
    RUN_TEST(test_xfer_large_payload_with_loss);
    RUN_TEST(test_xfer_heavy_loss_never_delivers_bad_data);
    RUN_TEST(test_xfer_corrupted_frame_fails_crc);
    RUN_TEST(test_xfer_receiver_refuses);
    RUN_TEST(test_xfer_payload_rejected_after_arrival);
    RUN_TEST(test_xfer_lost_final_ack_does_not_apply_twice);
    RUN_TEST(test_xfer_receiver_reset_mid_transfer);
    RUN_TEST(test_xfer_sender_vanishes_mid_transfer);
    RUN_TEST(test_xfer_new_session_replaces_stale_one);
    RUN_TEST(test_xfer_no_receiver_times_out);
    return UNITY_END();
}
