#include "EntriesProcessing.h"

namespace Processing {

bool filterEntries(const Common::Buffer& entries, std::vector<std::uint8_t>& outPrices, std::uint8_t eofMarker) {
    if(entries.countBytes < 3) {
        return false;
    }

    outPrices.clear();
    const std::vector<std::uint8_t>& data = entries.data;
    bool foundEof = false;
    for(std::int32_t i = 0; i < entries.countBytes; i += 2) {
        const std::uint8_t byte = data[i];
        if(byte != eofMarker) {
            outPrices.push_back(byte);
        } else {
            // assumption that we can skip the rest of the packet
            foundEof = true;
            break;
        }
    }

    if(!foundEof) {
        outPrices.clear();
        return false;
    }
    return true;
}

}