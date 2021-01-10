#ifndef MATCHING_ENGINE_MESSAGES_H
#define MATCHING_ENGINE_MESSAGES_H

#include <string>

#include "fields.h"

namespace gemini {
namespace MessageTypeEnum {
enum Type {
    Unknown,
    NewOrder = 'N',
    Trade = 'X',
};

constexpr const char *ToString(Type type) {
    switch (type) {
        case Type::NewOrder:
            return "NewOrder";
        case Type::Trade:
            return "Trade";
        case Type::Unknown:
            [[fallthrough]];
        default:
            return "<UNKNOWN>";
    }
}

inline Type FromString(const std::string &str) {
    if (str == "NewOrder") {
        return Type::NewOrder;
    } else if (str == "Trade") {
        return Type::Trade;
    }
    return Type::Unknown;
}
}  // namespace MessageTypeEnum

struct MessageHeader {
    MessageTypeEnum::Type messageType = MessageTypeEnum::Unknown;
};

struct NewOrder : MessageHeader {
    std::string orderId;
    std::string symbol;
    SideEnum::Type side;
    unsigned long quantity;
    unsigned long price;

    NewOrder() : MessageHeader{MessageTypeEnum::NewOrder} {}
};

struct Trade : MessageHeader {
    std::string symbol;
    std::string orderId;
    std::string contraOrderId;
    unsigned long quantity;
    unsigned long price;

    Trade() : MessageHeader{MessageTypeEnum::Trade} {}

    inline bool operator==(const Trade &rhs) const {
        return (symbol == rhs.symbol && orderId == rhs.orderId && contraOrderId == rhs.contraOrderId &&
                quantity == rhs.quantity && price == rhs.price);
    }
};

}  // namespace gemini

#endif
