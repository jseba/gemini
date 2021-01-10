#ifndef MATCHING_ENGINE__FIELDS_H
#define MATCHING_ENGINE__FIELDS_H

#include <string>

namespace gemini {

namespace SideEnum {
enum Type {
    Unknown,
    Buy = 'B',
    Sell = 'S',
};

constexpr const char *ToString(Type type) {
    switch (type) {
    case Type::Buy:
        return "BUY";
    case Type::Sell:
        return "SELL";
    case Type::Unknown:
        [[fallthrough]];
    default:
        return "<UNKNOWN>";
    }
    return "<UNKNOWN>";
}

inline Type FromString(const std::string &str) {
    if (str == "BUY") {
        return Type::Buy;
    } else if (str == "SELL") {
        return Type::Sell;
    }
    return Type::Unknown;
}

constexpr Type ContraSide(Type type) {
    if (type == Type::Buy) {
        return Type::Sell;
    }
    return Type::Buy;
}
} // namespace SideEnum

} // namespace gemini
#endif // MATCHING_ENGINE__FIELDS_H
