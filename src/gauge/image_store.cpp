#include "image_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <stdio.h>

#include <vector>

#include "crc32.h"

namespace image_store {

namespace {

const char* kDir = "/img";
const char* kTempFile = "/img/.tmp";
const uint32_t kReserveBytes = 64 * 1024;
const size_t kChunk = 4096;  // written a piece at a time, so other filesystem users get a turn

struct Image {
    uint32_t crc;
    uint32_t size;
    bool png;
    bool pending;  // handed to the writer, not on flash yet
};

struct Job {
    uint8_t* data;
    uint32_t size;
    uint32_t crc;
    bool png;
};

SemaphoreHandle_t mtx = nullptr;
std::vector<Image> images;  // hold a Guard
QueueHandle_t jobs = nullptr;
bool mounted = false;

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

std::string pathFor(uint32_t crc, bool png) {
    char buf[24];
    snprintf(buf, sizeof(buf), "%s/%08lX.%s", kDir, (unsigned long)crc, png ? "png" : "jpg");
    return buf;
}

int indexOf(uint32_t crc) {
    for (size_t i = 0; i < images.size(); i++) {
        if (images[i].crc == crc) return int(i);
    }
    return -1;
}

void forget(uint32_t crc) {
    Guard guard;
    const int i = indexOf(crc);
    if (i >= 0) images.erase(images.begin() + i);
}

bool write(const Job& job) {
    File f = LittleFS.open(kTempFile, "w");
    if (!f) return false;
    for (uint32_t pos = 0; pos < job.size; pos += kChunk) {
        const size_t n = job.size - pos < kChunk ? job.size - pos : kChunk;
        if (f.write(job.data + pos, n) != n) {
            f.close();
            LittleFS.remove(kTempFile);
            return false;
        }
        vTaskDelay(1);
    }
    f.close();
    const std::string path = pathFor(job.crc, job.png);
    LittleFS.remove(path.c_str());
    return LittleFS.rename(kTempFile, path.c_str());
}

void writerTask(void*) {
    Job job;
    for (;;) {
        if (xQueueReceive(jobs, &job, portMAX_DELAY) != pdTRUE) continue;
        const bool ok = write(job);
        free(job.data);
        if (ok) {
            {
                Guard guard;
                const int i = indexOf(job.crc);
                if (i >= 0) images[i].pending = false;
            }
            Serial.printf("Image %08lX stored, %lu bytes\n", (unsigned long)job.crc, (unsigned long)job.size);
        } else {
            // The hub finds out when it next asks.
            forget(job.crc);
            Serial.printf("Image %08lX could not be stored\n", (unsigned long)job.crc);
        }
    }
}

}  // namespace

bool begin() {
    mtx = xSemaphoreCreateMutex();
    mounted = LittleFS.begin(true);
    if (!mounted) return false;
    if (!LittleFS.exists(kDir)) LittleFS.mkdir(kDir);
    LittleFS.remove(kTempFile);

    File dir = LittleFS.open(kDir);
    for (File f = dir ? dir.openNextFile() : File(); f; f = dir.openNextFile()) {
        String base = f.name();
        const int slash = base.lastIndexOf('/');
        if (slash >= 0) base = base.substring(slash + 1);
        const bool png = base.endsWith(".png");
        if (base.length() != 12 || !(png || base.endsWith(".jpg"))) continue;
        char* end = nullptr;
        const uint32_t crc = uint32_t(strtoul(base.substring(0, 8).c_str(), &end, 16));
        if (!end || *end) continue;
        images.push_back({crc, uint32_t(f.size()), png, false});
    }
    jobs = xQueueCreate(4, sizeof(Job));
    xTaskCreatePinnedToCore(writerTask, "images", 4096, nullptr, 2, nullptr, 0);
    return true;
}

bool has(uint32_t crc) {
    if (!mtx) return false;
    Guard guard;
    return indexOf(crc) >= 0;
}

bool roomFor(uint32_t size) {
    if (!mounted) return false;
    uint32_t pending = 0;
    {
        Guard guard;
        for (const Image& im : images) {
            if (im.pending) pending += im.size;
        }
    }
    const size_t free = LittleFS.totalBytes() - LittleFS.usedBytes();
    return free >= size_t(size) + pending + kReserveBytes;
}

bool submit(uint8_t* data, uint32_t size) {
    if (!mounted || !jobs) {
        free(data);
        return false;
    }
    const Job job = {data, size, hg::crc32(data, size), size >= 4 && data[0] == 0x89 && data[1] == 'P'};
    bool known;
    {
        Guard guard;
        known = indexOf(job.crc) >= 0;
        if (!known) images.push_back({job.crc, size, job.png, true});
    }
    if (known) {  // sent again while still being written
        free(data);
        return true;
    }
    if (xQueueSend(jobs, &job, 0) != pdTRUE) {
        forget(job.crc);
        free(data);
        return false;
    }
    return true;
}

std::string path(uint32_t crc) {
    if (!mtx) return std::string();
    Guard guard;
    const int i = indexOf(crc);
    if (i < 0 || images[i].pending) return std::string();
    return pathFor(crc, images[i].png);
}

void keepOnly(const uint32_t* crcs, size_t count) {
    if (!mtx) return;
    std::vector<Image> drop;
    {
        Guard guard;
        for (size_t i = images.size(); i-- > 0;) {
            bool keep = images[i].pending;  // on its way: the config that wants it may follow
            for (size_t k = 0; k < count && !keep; k++) keep = images[i].crc == crcs[k];
            if (keep) continue;
            drop.push_back(images[i]);
            images.erase(images.begin() + i);
        }
    }
    for (const Image& im : drop) LittleFS.remove(pathFor(im.crc, im.png).c_str());
}

}  // namespace image_store
