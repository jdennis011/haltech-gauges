#include "image_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdio.h>

#include <vector>

#include "crc32.h"
#include "face_model.h"
#include "image_format.h"
#include "library_store.h"

using namespace hg;

namespace image_store {

namespace {

const char* kDir = "/img";
const char* kIndexFile = "/img/index.json";
const char* kUploadFile = "/img/.upload";

struct Entry {
    std::string name;
    Info info;
};

SemaphoreHandle_t mtx = nullptr;
std::vector<Entry> entries;
uint32_t changes = 0;
File sending;  // the image going out on the bus

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

std::string pathFor(const std::string& name, bool png) {
    return std::string(kDir) + "/" + name + (png ? ".png" : ".jpg");
}

bool readFile(uint32_t offset, uint8_t* out, uint32_t len, void* ctx) {
    File* f = static_cast<File*>(ctx);
    return f->seek(offset) && f->read(out, len) == len;
}

uint32_t crcOfFile(File& f) {
    uint8_t buf[512];
    uint32_t crc = 0;
    f.seek(0);
    for (;;) {
        const size_t n = f.read(buf, sizeof(buf));
        if (n == 0) break;
        crc = crc32(buf, n, crc);
    }
    return crc;
}

Entry* find(const char* name) {
    for (Entry& e : entries) {
        if (e.name == name) return &e;
    }
    return nullptr;
}

void saveIndex() {
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    for (const Entry& e : entries) {
        JsonObject o = root[e.name].to<JsonObject>();
        char crc[12];
        snprintf(crc, sizeof(crc), "%08lX", (unsigned long)e.info.crc);
        o["crc"] = crc;
        o["size"] = e.info.size;
        o["width"] = e.info.width;
        o["height"] = e.info.height;
        o["type"] = e.info.png ? "png" : "jpeg";
    }
    File f = LittleFS.open(kIndexFile, "w");
    if (!f) return;
    serializeJson(doc, f);
    f.close();
}

// Checks a file and fills in what is known about it.
bool examine(File& f, Info& out, std::string& error) {
    cfg::ImageInfo info;
    if (!cfg::probeImage(readFile, &f, uint32_t(f.size()), info, error)) return false;
    out.size = uint32_t(f.size());
    out.width = info.width;
    out.height = info.height;
    out.png = info.type == cfg::ImageType::Png;
    out.crc = crcOfFile(f);
    return true;
}

// The index, checked against the files: an image whose file has gone is
// dropped, and a file the index does not know is examined and added.
void load() {
    JsonDocument index;
    if (File f = LittleFS.open(kIndexFile, "r")) {
        if (deserializeJson(index, f)) index.clear();
        f.close();
    }
    bool changed = false;
    File dir = LittleFS.open(kDir);
    if (!dir) return;
    for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
        if (f.isDirectory()) continue;
        String base = f.name();
        const int slash = base.lastIndexOf('/');
        if (slash >= 0) base = base.substring(slash + 1);
        const bool png = base.endsWith(".png");
        if (base.startsWith(".") || !(png || base.endsWith(".jpg"))) continue;
        base.remove(base.length() - 4);
        if (!cfg::validImageName(base.c_str()) || entries.size() >= kMaxCount) continue;

        Entry e;
        e.name = base.c_str();
        JsonObjectConst known = index[e.name].as<JsonObjectConst>();
        const uint32_t size = uint32_t(f.size());
        if (!known.isNull() && known["size"] == size && (known["type"] == "png") == png) {
            e.info.crc = uint32_t(strtoul(known["crc"] | "0", nullptr, 16));
            e.info.size = size;
            e.info.width = known["width"] | 0;
            e.info.height = known["height"] | 0;
            e.info.png = png;
        } else {
            std::string error;
            if (!examine(f, e.info, error)) continue;
            changed = true;
        }
        entries.push_back(e);
    }
    if (changed || index.as<JsonObjectConst>().size() != entries.size()) saveIndex();
}

}  // namespace

void begin() {
    mtx = xSemaphoreCreateMutex();
    if (!LittleFS.exists(kDir)) LittleFS.mkdir(kDir);
    LittleFS.remove(kUploadFile);  // left from an upload cut short
    {
        Guard guard;
        load();
        changes++;
    }
    // The configs already loaded need the images' CRCs.
    library_store::imagesChanged();
}

const char* uploadPath() { return kUploadFile; }

bool roomFor(uint32_t size, std::string& error) {
    if (size == 0 || size > kMaxBytes) {
        error = "an image can be at most 256 KB";
        return false;
    }
    const size_t free = LittleFS.totalBytes() - LittleFS.usedBytes();
    if (free < size + kReserveBytes) {
        char buf[96];
        snprintf(buf, sizeof(buf), "not enough room on the hub: %u KB free, and %u KB is kept for configs",
                 unsigned(free / 1024), unsigned(kReserveBytes / 1024));
        error = buf;
        return false;
    }
    return true;
}

bool accept(const char* name, std::string& error) {
    if (!cfg::validImageName(name)) {
        LittleFS.remove(kUploadFile);
        error = "name must be 1-32 characters: letters, digits, - and _";
        return false;
    }
    Info info;
    {
        File f = LittleFS.open(kUploadFile, "r");
        if (!f) {
            error = "the upload did not arrive";
            return false;
        }
        const bool ok = f.size() > 0 && f.size() <= kMaxBytes && examine(f, info, error);
        f.close();
        if (!ok) {
            if (error.empty()) error = "an image can be at most 256 KB";
            LittleFS.remove(kUploadFile);
            return false;
        }
    }
    {
        Guard guard;
        Entry* e = find(name);
        if (!e && entries.size() >= kMaxCount) {
            LittleFS.remove(kUploadFile);
            error = "the hub holds at most 32 images; delete one first";
            return false;
        }
        const std::string path = pathFor(name, info.png);
        LittleFS.remove(path.c_str());
        if (!LittleFS.rename(kUploadFile, path.c_str())) {
            LittleFS.remove(kUploadFile);
            error = "could not store the image";
            return false;
        }
        // A JPEG replaced by a PNG of the same name, or the other way round.
        if (e && e->info.png != info.png) LittleFS.remove(pathFor(name, e->info.png).c_str());
        if (!e) {
            entries.push_back(Entry());
            e = &entries.back();
            e->name = name;
        }
        e->info = info;
        saveIndex();
        changes++;
    }
    library_store::imagesChanged();
    return true;
}

bool remove(const char* name) {
    {
        Guard guard;
        Entry* e = find(name);
        if (!e) return false;
        LittleFS.remove(pathFor(e->name, e->info.png).c_str());
        entries.erase(entries.begin() + (e - entries.data()));
        saveIndex();
        changes++;
    }
    library_store::imagesChanged();
    return true;
}

bool lookup(const char* name, Info& out) {
    Guard guard;
    const Entry* e = find(name);
    if (!e) return false;
    out = e->info;
    return true;
}

bool file(const char* name, std::string& path, const char*& contentType) {
    Guard guard;
    const Entry* e = find(name);
    if (!e) return false;
    path = pathFor(e->name, e->info.png);
    contentType = e->info.png ? "image/png" : "image/jpeg";
    return true;
}

void listJson(JsonArray out) {
    Guard guard;
    for (const Entry& e : entries) {
        JsonObject o = out.add<JsonObject>();
        o["name"] = e.name;
        o["size"] = e.info.size;
        char crc[12];
        snprintf(crc, sizeof(crc), "%08lX", (unsigned long)e.info.crc);
        o["crc"] = crc;
        o["width"] = e.info.width;
        o["height"] = e.info.height;
        o["type"] = e.info.png ? "png" : "jpeg";
    }
}

uint32_t version() { return changes; }

bool openForSend(uint32_t crc, hg::proto::XferSource& source, uint32_t& size, void*) {
    Guard guard;
    for (const Entry& e : entries) {
        if (e.info.crc != crc) continue;
        if (sending) sending.close();
        sending = LittleFS.open(pathFor(e.name, e.info.png).c_str(), "r");
        if (!sending) return false;
        source = {readFile, &sending};
        size = uint32_t(sending.size());
        return true;
    }
    return false;
}

void closeSend(void*) {
    Guard guard;
    if (sending) sending.close();
}

}  // namespace image_store
