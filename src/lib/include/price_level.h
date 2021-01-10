#ifndef MATCHING_ENGINE__PRICE_LEVEL_H
#define MATCHING_ENGINE__PRICE_LEVEL_H

#include "fields.h"

#include <cassert>

namespace gemini {
struct PriceLevel {
    unsigned long price;
    SideEnum::Type side;
};

constexpr bool operator<(const PriceLevel &lhs,
                         const PriceLevel &rhs) noexcept {
    assert(lhs.side == rhs.side);
    if (lhs.side == SideEnum::Buy) {
        return lhs.price > rhs.price;
    } else {
        return lhs.price < rhs.price;
    }
}

} // namespace gemini

#endif // MATCHING_ENGINE__PRICE_LEVEL_H
