#ifndef MATCHING_ENGINE__ORDER_BOOK_H
#define MATCHING_ENGINE__ORDER_BOOK_H

#include "messages.h"
#include "order.h"
#include "price_level.h"

#include <functional>
#include <list>
#include <map>
#include <vector>

namespace gemini {

class OrderBook {
  public:
    using OrderMatchedFn = std::function<void(const Trade &)>;

    OrderBook(std::string symbol, OrderMatchedFn fn);

    // may result in matches, will call the callback for each match
    void AddOrder(Order order);

    // no cancel message, so no need for a CancelOrder

    std::vector<std::string> Dump() const;

  private:
    std::string m_symbol;

    OrderMatchedFn m_orderMatched;

    // primary index is by price time
    //
    // std::multimap has the requirement that subsequent insertions
    // at the same key are sorted in insertion order, so no need to
    // track the time manually since order modifications are not supported
    using PriceLevelIndex = std::multimap<PriceLevel, Order>;
    using OrderIterator = PriceLevelIndex::iterator;

    using SequenceNumberIndex = std::map<unsigned long, OrderIterator>;

    struct Indexes {
        PriceLevelIndex &byPriceLevel;
        SequenceNumberIndex &bySequenceNumber;
    };

    Indexes GetIndexesForSide(SideEnum::Type side) noexcept;

    bool OrdersMatch(const Order &inboundOrder, const Order &restingOrder);
    std::vector<Trade> GenerateTrades(Order &inboundOrder);

    // primary (owned) storage for orders
    PriceLevelIndex m_bids;
    PriceLevelIndex m_asks;

    // other indexes reference the primary storage
    SequenceNumberIndex m_bidsBySequenceNumber;
    SequenceNumberIndex m_asksBySequenceNumber;
};
} // namespace gemini

#endif // MATCHING_ENGINE__ORDER_BOOK_H
