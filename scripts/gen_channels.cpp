// Prints the channel list exactly as the hub's /api/channels/meta does, from the
// same tables, so the online copy of the page can carry it as a static file.
// Built and run by scripts/gen_channels.ps1 into web/data/channels.json.
#include <cstdio>

#include "haltech.h"
#include "units.h"

using namespace hg;

int main() {
    printf("{\"units\":[");
    for (int u = 0; u <= int(Unit::Metres); u++) {
        size_t count = 0;
        const DisplayUnit* options = displayUnits(Unit(u), count);
        printf("%s{\"s\":\"%s\",\"o\":[", u ? "," : "", unitSymbol(Unit(u)));
        for (size_t k = 0; k < count; k++) {
            printf("%s{\"n\":\"%s\",\"s\":\"%s\",\"m\":%.7g,\"a\":%.7g}", k ? "," : "", options[k].name,
                   options[k].symbol, double(options[k].mul), double(options[k].add));
        }
        printf("]}");
    }
    printf("],\"channels\":[");
    for (int i = 0; i < CH_COUNT; i++) {
        const ChannelDef& d = channelDef(ChannelId(i));
        printf("%s{\"n\":\"%s\",\"l\":\"%s\",\"u\":%u,\"f\":%u}", i ? "," : "", d.name, d.label,
               unsigned(d.unit), unsigned(d.canId));
    }
    printf("]}\n");
    return 0;
}
