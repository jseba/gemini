#include "order_book.h"

#include <cassert>

namespace gemini {

OrderBook::OrderBook(std::string symbol, OrderMatchedFn fn)
    : m_symbol(std::move(symbol)), m_orderMatched(fn) {}

void OrderBook::AddOrder(Order order) {
    // generate matches
    auto trades = GenerateTrades(order);

    // print the trades
    for (auto &trade : trades) {
        m_orderMatched(trade);
    }

    // if still quantity left, rest the order
    if (order.Quantity() > 0) {
        auto indexes = GetIndexesForSide(order.Side());

        PriceLevel priceLevel{order.Price(), order.Side()};
        auto element = std::make_pair(priceLevel, std::move(order));
        auto it = indexes.byPriceLevel.insert(std::move(element));

        // the book now owns the order

        indexes.bySequenceNumber.insert(
            std::make_pair(order.SequenceNumber(), it));
    }
}

std::vector<std::string> OrderBook::Dump() const {
    std::vector<std::string> result;

    // dump orders in sequence order, asks before bids
    for (auto const &it : m_asksBySequenceNumber) {
        result.push_back(it.second->second.ToString());
    }
    for (auto const &it : m_bidsBySequenceNumber) {
        result.push_back(it.second->second.ToString());
    }

    return result;
}

OrderBook::Indexes OrderBook::GetIndexesForSide(SideEnum::Type side) noexcept {
    if (side == SideEnum::Buy) {
        return {m_bids, m_bidsBySequenceNumber};
    }
    return {m_asks, m_asksBySequenceNumber};
}

bool OrderBook::OrdersMatch(const Order &inboundOrder,
                            const Order &restingOrder) {
    if (inboundOrder.Side() == SideEnum::Buy) {
        return restingOrder.Price() <= inboundOrder.Price();
    } else {
        return inboundOrder.Price() <= restingOrder.Price();
    }
}

std::vector<Trade> OrderBook::GenerateTrades(Order &inboundOrder) {
    std::vector<Trade> trades;
    std::vector<OrderIterator> filledRestingOrders;

    // scan the opposite side for matching resting orders
    auto contraSide =
        inboundOrder.Side() == SideEnum::Buy ? SideEnum::Sell : SideEnum::Buy;
    auto contraSideIndexes = GetIndexesForSide(contraSide);

    // run until we hit an order that doesn't match
    auto it = contraSideIndexes.byPriceLevel.begin();
    while (it != contraSideIndexes.byPriceLevel.end() &&
           OrdersMatch(inboundOrder, it->second)) {
        // calculate traded quantity
        auto tradeQuantity =
            std::min(inboundOrder.Quantity(), it->second.Quantity());
        auto tradePrice = it->second.Price();

        Trade trade;

        trade.symbol = m_symbol;
        trade.orderId = inboundOrder.OrderId();
        trade.contraOrderId = it->second.OrderId();
        trade.quantity = tradeQuantity;
        trade.price = tradePrice;

        trades.push_back(std::move(trade));

        // adjust quantity on each order
        inboundOrder.DecreaseQuantity(tradeQuantity);
        it->second.DecreaseQuantity(tradeQuantity);

        // queue resting order for removal from indexes if fully filled
        if (it->second.Quantity() == 0) {
            filledRestingOrders.push_back(it);
        }

        // break if no more quantity on inbound order
        if (inboundOrder.Quantity() == 0) {
            break;
        }

        ++it;
    }

    for (auto filledOrderIt : filledRestingOrders) {
        // remove from secondary indexes, then primary
        contraSideIndexes.bySequenceNumber.erase(
            filledOrderIt->second.SequenceNumber());
        contraSideIndexes.byPriceLevel.erase(filledOrderIt);
    }

    return trades;
}

} // namespace gemini
