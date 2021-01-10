#include "catch.hpp"

#include "matching_engine.h"

using namespace gemini;

namespace Catch {
template <> struct StringMaker<gemini::Trade> {
    static std::string convert(const gemini::Trade &trade) {
        std::string result;

        result += trade.symbol;
        result += ' ';
        result += trade.orderId;
        result += ' ';
        result += trade.contraOrderId;
        result += ' ';
        result += std::to_string(trade.quantity);
        result += ' ';
        result += std::to_string(trade.price);

        return result;
    }
};
} // namespace Catch

NewOrder ConstructNewOrder(std::string orderId, std::string symbol,
                           SideEnum::Type side, unsigned long quantity,
                           unsigned long price) {
    NewOrder newOrder;

    newOrder.orderId = orderId;
    newOrder.symbol = symbol;
    newOrder.side = side;
    newOrder.quantity = quantity;
    newOrder.price = price;

    return newOrder;
}

Trade ConstructTrade(std::string symbol, std::string orderId,
                     std::string contraOrderId, unsigned long quantity,
                     unsigned long price) {
    Trade trade;

    trade.symbol = symbol;
    trade.orderId = orderId;
    trade.contraOrderId = contraOrderId;
    trade.quantity = quantity;
    trade.price = price;

    return trade;
}

TEST_CASE("Test constructor", "[basic]") {
    MatchingEngine engine([](const MessageHeader &) {});
    REQUIRE(engine.Dump().empty());
}

TEST_CASE("Test basic add order", "[basic]") {
    MatchingEngine engine([](const MessageHeader &) {});

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    engine.OnMessage(newOrder1);

    std::vector<std::string> expectedOrders;
    expectedOrders.push_back(Order(1, newOrder1).ToString());

    auto actualOrders = engine.Dump();
    REQUIRE(actualOrders.size() == 1);
    REQUIRE(expectedOrders == actualOrders);
}

TEST_CASE("Test basic order fill", "[basic]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 =
        ConstructNewOrder("2", "BTCUSD", SideEnum::Sell, 100, 1234);

    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "2", "1", 100, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // check order book is empty
    REQUIRE(engine.Dump().empty());
}

TEST_CASE("Test basic partial fill", "[basic]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 = ConstructNewOrder("2", "BTCUSD", SideEnum::Sell, 99, 1234);

    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "2", "1", 99, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // reduce order 1 quantity
    newOrder1.quantity -= newOrder2.quantity;

    // check order 1 left on book
    std::vector<std::string> expectedOrders;
    expectedOrders.push_back(Order(1, newOrder1).ToString());

    auto actualOrders = engine.Dump();
    REQUIRE(actualOrders.size() == 1);
    REQUIRE(expectedOrders == actualOrders);
}

TEST_CASE("Test basic non-matching orders", "[basic]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 =
        ConstructNewOrder("2", "BTCUSD", SideEnum::Sell, 100, 1235);

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);

    REQUIRE(actualTrades.empty());

    // check both orders left on book in correct order
    std::vector<std::string> expectedOrders;
    expectedOrders.push_back(Order(2, newOrder2).ToString());
    expectedOrders.push_back(Order(1, newOrder1).ToString());

    auto actualOrders = engine.Dump();
    REQUIRE(actualOrders.size() == 2);
    REQUIRE(expectedOrders == actualOrders);
}

TEST_CASE("Test inbound order hits multiple resting", "[fills]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 = ConstructNewOrder("2", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder3 =
        ConstructNewOrder("3", "BTCUSD", SideEnum::Sell, 200, 1234);

    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "1", 100, 1234));
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "2", 100, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);
    engine.OnMessage(newOrder3);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // check order book is empty
    REQUIRE(engine.Dump().empty());
}

TEST_CASE("Test inbound order hits multiple resting before resting",
          "[fills]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 = ConstructNewOrder("2", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder3 =
        ConstructNewOrder("3", "BTCUSD", SideEnum::Sell, 300, 1234);

    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "1", 100, 1234));
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "2", 100, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);
    engine.OnMessage(newOrder3);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // reduce order 1 quantity
    newOrder3.quantity -= newOrder1.quantity;
    newOrder3.quantity -= newOrder2.quantity;

    // check order 3 left on book
    std::vector<std::string> expectedOrders;
    expectedOrders.push_back(Order(3, newOrder3).ToString());

    auto actualOrders = engine.Dump();
    REQUIRE(!actualOrders.empty());
    REQUIRE(expectedOrders == actualOrders);
}

TEST_CASE("Test inbound order trades at best price", "[fills]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 =
        ConstructNewOrder("2", "BTCUSD", SideEnum::Sell, 100, 1230);

    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "2", "1", 100, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // check order book is empty
    REQUIRE(engine.Dump().empty());
}

TEST_CASE("Test inbound order trades at multiple price levels", "[fills]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 = ConstructNewOrder("2", "BTCUSD", SideEnum::Buy, 100, 1235);
    auto newOrder3 =
        ConstructNewOrder("3", "BTCUSD", SideEnum::Sell, 200, 1230);

    // order should be by best price level
    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "2", 100, 1235));
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "1", 100, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);
    engine.OnMessage(newOrder3);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // check order book is empty
    REQUIRE(engine.Dump().empty());
}

TEST_CASE("Test inbound order hits multiple orders by time priority",
          "[fills]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 = ConstructNewOrder("2", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder3 =
        ConstructNewOrder("3", "BTCUSD", SideEnum::Sell, 200, 1234);

    // order should be by time priority
    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "1", 100, 1234));
    expectedTrades.push_back(ConstructTrade("BTCUSD", "3", "2", 100, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);
    engine.OnMessage(newOrder3);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // check order book is empty
    REQUIRE(engine.Dump().empty());
}

TEST_CASE("Test inbound order hits multiple price levels by time priority",
          "[fills]") {
    std::vector<Trade> actualTrades;

    MatchingEngine engine([&](const MessageHeader &msg) {
        REQUIRE(msg.messageType == MessageTypeEnum::Trade);
        actualTrades.push_back(static_cast<const Trade &>(msg));
    });

    auto newOrder1 = ConstructNewOrder("1", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder2 = ConstructNewOrder("2", "BTCUSD", SideEnum::Buy, 100, 1234);
    auto newOrder3 = ConstructNewOrder("3", "BTCUSD", SideEnum::Buy, 100, 1235);
    auto newOrder4 = ConstructNewOrder("4", "BTCUSD", SideEnum::Buy, 100, 1235);
    auto newOrder5 =
        ConstructNewOrder("5", "BTCUSD", SideEnum::Sell, 400, 1234);

    // order should be by best price, then by time priority
    std::vector<Trade> expectedTrades;
    expectedTrades.push_back(ConstructTrade("BTCUSD", "5", "3", 100, 1235));
    expectedTrades.push_back(ConstructTrade("BTCUSD", "5", "4", 100, 1235));
    expectedTrades.push_back(ConstructTrade("BTCUSD", "5", "1", 100, 1234));
    expectedTrades.push_back(ConstructTrade("BTCUSD", "5", "2", 100, 1234));

    engine.OnMessage(newOrder1);
    engine.OnMessage(newOrder2);
    engine.OnMessage(newOrder3);
    engine.OnMessage(newOrder4);
    engine.OnMessage(newOrder5);

    REQUIRE(!actualTrades.empty());
    REQUIRE(expectedTrades == actualTrades);

    // check order book is empty
    REQUIRE(engine.Dump().empty());
}
