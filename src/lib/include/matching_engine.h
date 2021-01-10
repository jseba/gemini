#ifndef MATCHING_ENGINE__MATCHING_ENGINE_H
#define MATCHING_ENGINE__MATCHING_ENGINE_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "messages.h"
#include "order_book.h"

namespace gemini {
class MatchingEngine {
   public:
    using SendMessageFn = std::function<void(const MessageHeader &msg)>;

    MatchingEngine(SendMessageFn fn);

    void OnMessage(const MessageHeader &msg);

    std::vector<std::string> Dump() const;

   private:
    void OnNewOrder(const NewOrder &newOrder);

    void HandleOrderMatched(const Trade &trade);

    OrderBook &FindOrCreateSymbolOrderBook(const std::string &symbol);

    SendMessageFn m_sendMessage;

    // sequence number increments on receipt of each message
    unsigned long m_sequenceNumber;

    // one order book per symbol (instrument)
    std::map<std::string, OrderBook> m_orderBooks;
};
}  // namespace gemini

#endif  // MATCHING_ENGINE__MATCHING_ENGINE_H
