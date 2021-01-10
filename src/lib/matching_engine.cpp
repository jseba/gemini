#include "matching_engine.h"

#include <cassert>

namespace gemini {

MatchingEngine::MatchingEngine(SendMessageFn fn)
    : m_sendMessage(fn), m_sequenceNumber(0) {}

void MatchingEngine::OnMessage(const MessageHeader &msg) {
    m_sequenceNumber++;

    switch (msg.messageType) {
    case MessageTypeEnum::NewOrder:
        OnNewOrder(static_cast<const NewOrder &>(msg));
        break;
    default:
        assert(!"Unexpected message type");
    }
}

void MatchingEngine::OnNewOrder(const NewOrder &msg) {
    Order order{m_sequenceNumber, msg};

    auto &orderBook = FindOrCreateSymbolOrderBook(order.Symbol());
    orderBook.AddOrder(std::move(order)); // may result in trades
}

std::vector<std::string> MatchingEngine::Dump() const {
    std::vector<std::string> result;

    for (auto const &it : m_orderBooks) {
        auto orders = it.second.Dump();
        result.insert(result.end(), orders.begin(), orders.end());
    }

    return result;
}

OrderBook &
MatchingEngine::FindOrCreateSymbolOrderBook(const std::string &symbol) {
    auto handler = [this](const Trade &trade) { m_sendMessage(trade); };

    auto [it, result] = m_orderBooks.try_emplace(symbol, symbol, handler);
    return it->second;
}

} // namespace gemini
