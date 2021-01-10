#include "order.h"

namespace gemini {
Order::Order(unsigned long sequenceNumber, const NewOrder &newOrder)
    : m_sequenceNumber(sequenceNumber), m_orderId(newOrder.orderId),
      m_symbol(newOrder.symbol), m_side(newOrder.side), m_price(newOrder.price),
      m_quantity(newOrder.quantity) {}

unsigned long Order::SequenceNumber() const noexcept {
    return m_sequenceNumber;
}

const std::string &Order::OrderId() const noexcept { return m_orderId; }

const std::string &Order::Symbol() const noexcept { return m_symbol; }

SideEnum::Type Order::Side() const noexcept { return m_side; }

unsigned long Order::Price() const noexcept { return m_price; }

unsigned long Order::Quantity() const noexcept { return m_quantity; }

void Order::DecreaseQuantity(unsigned long value) noexcept {
    m_quantity -= value;
}

std::string Order::ToString() const {
    // 64 character string should be long enough
    std::string result;
    result.resize(64);

    snprintf(result.data(), result.size(), "%s %s %s %lu %lu",
             m_orderId.c_str(), SideEnum::ToString(m_side), m_symbol.c_str(),
             m_quantity, m_price);

    return result;
}

} // namespace gemini
