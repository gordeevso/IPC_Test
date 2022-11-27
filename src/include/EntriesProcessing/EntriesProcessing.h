#pragma once

#include <vector>

#include "Common.h"

namespace Processing {

/* Filters incoming entries (price and volume) by throwing volume away (every second byte)
 * assumes that message contains at least 3 bytes: price volume EOFMarker
 * */
bool filterEntries(const Common::Buffer& entries, std::vector<std::uint8_t>& outPrices, std::uint8_t eofMarker);

/* Filters prices with custom predicate */
template<typename Predicate>
void filterPrices(const Common::Buffer& allPrices, std::vector<std::vector<std::uint8_t>>& goodPrices, std::size_t messageLength, Predicate&& goodPricePredicate) {
    goodPrices.clear();
    const std::vector<std::uint8_t>& prices = allPrices.data;
    for(std::int32_t i = 0; i < allPrices.countBytes; ++i) {
        const std::uint8_t& byte = prices[i];
        if(goodPricePredicate(byte)) {
            std::vector<std::uint8_t> message(messageLength, byte);
            goodPrices.push_back(std::move(message));
        }
    }
}

}