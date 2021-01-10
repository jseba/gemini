#ifndef MATCHING_ENGINE__ORDER_H
#define MATCHING_ENGINE__ORDER_H

#include "messages.h"

namespace gemini {
class Order {
  public:
    Order(unsigned long sequenceNumber, const NewOrder &newOrder);

    // movable, non-copyable
    Order(const Order &) = delete;
    Order &operator=(const Order &) = delete;

    Order(Order &&) = default;
    Order &operator=(Order &&) = default;

    unsigned long SequenceNumber() const noexcept;
    const std::string &OrderId() const noexcept;
    const std::string &Symbol() const noexcept;
    SideEnum::Type Side() const noexcept;
    unsigned long Price() const noexcept;
    unsigned long Quantity() const noexcept;

    // only quantity may be changed, and then only by decreasing due to match
    void DecreaseQuantity(unsigned long value) noexcept;

    std::string ToString() const;

  private:
    unsigned long m_sequenceNumber;
    std::string m_orderId;
    std::string m_symbol;
    SideEnum::Type m_side;
    unsigned long m_price;
    unsigned long m_quantity;
};
} // namespace gemini

#endif // MATCHING_ENGINE__ORDER_H
