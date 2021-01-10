# Gemini Code Exercise for Josh Seba

## Build Instructions

Prerequisites:
  1. cmake version 3.10 or higher
  2. a version of g++ that supports C++17

This is a standard CMake based application. The following commands should build the application when run from this directory:
```
$ cd src
$ cmake -B build
$ cmake --build build
```

This will put the artifacts in src/build. The matching engine itself will be at src/build/app/matching_engine and the test
binary will be at src/build/test/test_matching_engine.

The scripts `run.sh` and `run_tests.sh` will run the application and the unit tests (respectively) in a Docker container.

## Design Approach

I approached this exercise by working on the order book data structure, then getting the orders into the book and finally by implementing
a basic matching algorithm.

Since the orders need to be sorted by price, I chose to use the standard `std::multimap`. This had an unexpected benefit; the requirements
of `std::multimap` state that if a key already exist, the new value should be inserted at the upper range of the already inserted keys.
Thus, the time invariant is automatically maintained by insertion order.

In order to meet the requirement of printing the remaining resting orders at the end of the input, there is a second index that maintains
a map of incoming sequence number to a pointer into the price-time order book. This allows the application to walk the orders by their
arrival time in constant time.

The matching algorithm works by examining the contra-side order book. Since the orders are sorted in price-time priority, this simply means
walking from the beginning of the data structure, checking that the price at the current resting order matches against the inbound order. If
there is a match, then a trade is generated and put in the queue. The order quantities are then adjusted. If the resting order's quantity
drops to zero as a result of the trade, it is put into a queue for later removal, in order to preserve the iterators for the matching loop.
If the inbound order quantity drops to zero, the end of the book is hit, or the price no longer matches a resting order, the loop is
terminated. Upon termination of the loop, all generated trades are sent out, followed by removing any resting orders whose quantities have
dropped to zero.

The engine and order book are designed to be unit testable by using callback functions that can be hook by the tests to inspect the output.
Simple message types are included to provide callback data; only two are implemented for this exercise: NewOrder and Trade.

The main application runner creates a matching engine hooked up to an output function that prints the trades as they are emitted. It parses
the input from `stdin` and creates `NewOrder` messages to push into the engine. The engine submits then submits these to the order book for
matching.

For unit tests, I tried to cover a decent set of scenarios. Covered scenarios include simple ones, such as a basic resting order and basic
order fill. More complicated scenarios are also included, ensuring that orders are traded in price-time priority, multiple price levels are
hit and that any remainder remains on the book to be hit later.

### Time Spent

I spent roughly a weekend on this project, in total about 1 day to write the matching engine and a half day writing tests.

