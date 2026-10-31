#include "runs.h"

#include <Arduino.h>
#include <stdio.h>
#include <time.h>

#include <atomic>
#include <string>

#include "datalog.h"
#include "drag_timer.h"

using hg::proto::HubManager;
using hg::proto::RunReport;

namespace runs {

namespace {

const char* kFile = "runs.csv";
const char* kHeader = "time,uptime_s,gauge,mac,run,aborted,splits,s1_name,s1_s,s1_kmh,s2_name,s2_s,s2_kmh,s3_name,s3_s,s3_kmh,s4_name,s4_s,s4_kmh,total_s";

struct Entry {
    uint32_t node;
    uint8_t mac[6];
    uint32_t uptimeMs;
    time_t epoch;  // 0 when the hub has no time
    RunReport report;
};
Entry ring[kKeep];
size_t head = 0;   // next slot
size_t count = 0;
std::atomic<uint32_t> ver{1};

std::string macText(const uint8_t* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}

std::string timeText(time_t epoch) {
    if (!epoch) return "";
    char buf[24];
    struct tm lt;
    localtime_r(&epoch, &lt);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &lt);
    return buf;
}

std::string seconds(uint32_t ms) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu.%03lu", (unsigned long)(ms / 1000), (unsigned long)(ms % 1000));
    return buf;
}

void appendCsv(const Entry& e) {
    const hg::motion::Run run = hg::motion::Run(e.report.run < hg::motion::kRunCount ? e.report.run : 0);
    size_t n = 0;
    const hg::motion::Split* sp = hg::motion::splits(run, n);
    std::string line = timeText(e.epoch);
    char buf[32];
    snprintf(buf, sizeof(buf), ",%lu,%06lX,", (unsigned long)(e.uptimeMs / 1000), (unsigned long)e.node);
    line += buf;
    line += macText(e.mac);
    line += ',';
    line += hg::motion::runKey(run);
    line += e.report.aborted ? ",1," : ",0,";
    line += std::to_string(e.report.count);
    for (size_t i = 0; i < hg::proto::kRunSplits; i++) {
        line += ',';
        if (i < n) line += sp[i].name;
        line += ',';
        if (i < e.report.count) line += seconds(e.report.ms[i]);
        line += ',';
        if (i < e.report.count && e.report.speed10[i]) {
            snprintf(buf, sizeof(buf), "%u.%u", unsigned(e.report.speed10[i] / 10), unsigned(e.report.speed10[i] % 10));
            line += buf;
        }
    }
    line += ',';
    if (e.report.count) line += seconds(e.report.ms[e.report.count - 1]);
    datalog::appendLine(kFile, line, kHeader);
}

}  // namespace

void onRun(const HubManager::Gauge& gauge, const RunReport& report, void*) {
    Entry& e = ring[head];
    e.node = gauge.node;
    memcpy(e.mac, gauge.mac, 6);
    e.uptimeMs = millis();
    const time_t now = time(nullptr);
    e.epoch = now > 1700000000 ? now : 0;
    e.report = report;
    head = (head + 1) % kKeep;
    if (count < kKeep) count++;
    ver++;
    appendCsv(e);
    Serial.printf("run: %06lX %s %s%s\n", (unsigned long)gauge.node,
                  hg::motion::runName(hg::motion::Run(report.run < hg::motion::kRunCount ? report.run : 0)),
                  report.count ? seconds(report.ms[report.count - 1]).c_str() : "-", report.aborted ? " aborted" : "");
}

uint32_t version() { return ver; }

void listJson(JsonArray out) {
    for (size_t i = 0; i < count; i++) {
        const Entry& e = ring[(head + kKeep - 1 - i) % kKeep];
        const hg::motion::Run run = hg::motion::Run(e.report.run < hg::motion::kRunCount ? e.report.run : 0);
        size_t n = 0;
        const hg::motion::Split* sp = hg::motion::splits(run, n);
        JsonObject o = out.add<JsonObject>();
        o["seq"] = e.report.seq;
        o["at_s"] = e.uptimeMs / 1000;
        o["time"] = timeText(e.epoch);
        char node[8];
        snprintf(node, sizeof(node), "%06lX", (unsigned long)e.node);
        o["node"] = node;
        o["mac"] = macText(e.mac);
        o["run"] = hg::motion::runKey(run);
        o["name"] = hg::motion::runName(run);
        o["aborted"] = e.report.aborted;
        o["complete"] = !e.report.aborted && e.report.count == n;
        o["total_ms"] = e.report.count ? e.report.ms[e.report.count - 1] : 0;
        JsonArray splits = o["splits"].to<JsonArray>();
        for (size_t s = 0; s < e.report.count && s < n; s++) {
            JsonObject so = splits.add<JsonObject>();
            so["name"] = sp[s].name;
            so["ms"] = e.report.ms[s];
            so["kmh"] = e.report.speed10[s] / 10.0f;
        }
    }
}

void clear() {
    head = 0;
    count = 0;
    ver++;
}

}  // namespace runs
