#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "matching_engine.h"
#include "messages.h"

using namespace gemini;

enum NewOrderFieldIndex {
    OrderId = 0,
    Side = 1,
    Symbol = 2,
    Quantity = 3,
    Price = 4,
};

// breaks the line into substrings
std::vector<std::string> ParseLine(const std::string &line) {
    std::vector<std::string> result;

    std::string field;
    for (auto c : line) {
        if (isspace(c)) {
            if (!field.empty()) {
                result.push_back(std::move(field));
                field.clear();
            }
        } else {
            field.push_back(c);
        }
    }

    // push final field (if any)
    if (!field.empty()) {
        result.push_back(std::move(field));
    }

    return result;
}

// Constructs a new order from an exploded line
NewOrder ConstructNewOrderFromFields(const std::vector<std::string> &fields) {
    NewOrder result;

    if (fields.size() != 5) {
        return result;
    }

    result.orderId = fields[NewOrderFieldIndex::OrderId];
    result.side = SideEnum::FromString(fields[NewOrderFieldIndex::Side]);
    result.symbol = fields[NewOrderFieldIndex::Symbol];
    result.quantity = std::stoul(fields[NewOrderFieldIndex::Quantity]);
    result.price = std::stoul(fields[NewOrderFieldIndex::Price]);

    return result;
}

void PrintTrade(const Trade &trade) {
    std::cout << "TRADE " << trade.symbol << ' ' << trade.orderId << ' ' << trade.contraOrderId << ' ' << trade.quantity
              << ' ' << trade.price << '\n';
}

int main() {
    MatchingEngine engine{[](const MessageHeader &msg) {
        switch (msg.messageType) {
            case MessageTypeEnum::Trade:
                PrintTrade(static_cast<const Trade &>(msg));
                break;
            default:
                assert(!"unexpected message type");
        }
    }};

    std::cerr << "====== Match Engine =====" << std::endl;
    std::cerr << "Enter 'exit' to quit" << std::endl;

    std::string line;
    while (getline(std::cin, line) && line != "exit") {
        /* std::cout << "Received: '" << line << "'" << std::endl; */

        auto fields = ParseLine(line);
        auto newOrder = ConstructNewOrderFromFields(fields);
        engine.OnMessage(newOrder);
    }

    std::cout << '\n';
    auto orders = engine.Dump();
    for (auto const &order : orders) {
        std::cout << order << '\n';
    }

    return 0;
}
