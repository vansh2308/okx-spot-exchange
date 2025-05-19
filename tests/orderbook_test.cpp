#include <gtest/gtest.h>
#include "core/orderbook.h"

class OrderBookTest : public ::testing::Test {
protected:
    void SetUp() override {
        orderBook = std::make_shared<core::OrderBook>();
    }

    std::shared_ptr<core::OrderBook> orderBook;
    const std::string exchange = "OKX";
    const std::string symbol = "BTC-USDT";
    const std::string timestamp = "2024-03-20T12:00:00Z";
};

TEST_F(OrderBookTest, EmptyOrderBook) {
    EXPECT_TRUE(orderBook->getBids().empty());
    EXPECT_TRUE(orderBook->getAsks().empty());
}

TEST_F(OrderBookTest, UpdateBids) {
    std::vector<std::pair<std::string, std::string>> bids = {
        {"100.0", "1.0"},
        {"99.0", "2.0"},
        {"98.0", "3.0"}
    };
    
    orderBook->update(exchange, symbol, bids, {}, timestamp);
    
    auto updatedBids = orderBook->getBids();
    EXPECT_EQ(updatedBids.size(), 3);
    EXPECT_DOUBLE_EQ(updatedBids[0].price, 100.0);
    EXPECT_DOUBLE_EQ(updatedBids[0].quantity, 1.0);
}

TEST_F(OrderBookTest, UpdateAsks) {
    std::vector<std::pair<std::string, std::string>> asks = {
        {"101.0", "1.0"},
        {"102.0", "2.0"},
        {"103.0", "3.0"}
    };
    
    orderBook->update(exchange, symbol, {}, asks, timestamp);
    
    auto updatedAsks = orderBook->getAsks();
    EXPECT_EQ(updatedAsks.size(), 3);
    EXPECT_DOUBLE_EQ(updatedAsks[0].price, 101.0);
    EXPECT_DOUBLE_EQ(updatedAsks[0].quantity, 1.0);
}

TEST_F(OrderBookTest, UpdateBothBidsAndAsks) {
    std::vector<std::pair<std::string, std::string>> bids = {
        {"100.0", "1.0"},
        {"99.0", "2.0"}
    };
    
    std::vector<std::pair<std::string, std::string>> asks = {
        {"101.0", "1.0"},
        {"102.0", "2.0"}
    };
    
    orderBook->update(exchange, symbol, bids, asks, timestamp);
    
    EXPECT_EQ(orderBook->getBids().size(), 2);
    EXPECT_EQ(orderBook->getAsks().size(), 2);
}

TEST_F(OrderBookTest, ClearOrderBook) {
    std::vector<std::pair<std::string, std::string>> bids = {
        {"100.0", "1.0"}
    };
    
    std::vector<std::pair<std::string, std::string>> asks = {
        {"101.0", "1.0"}
    };
    
    orderBook->update(exchange, symbol, bids, asks, timestamp);
    orderBook->update(exchange, symbol, {}, {}, timestamp);  // Clear by updating with empty vectors
    
    EXPECT_TRUE(orderBook->getBids().empty());
    EXPECT_TRUE(orderBook->getAsks().empty());
} 