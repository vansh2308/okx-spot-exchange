#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "models/simulator.h"
#include "core/config.h"
#include "core/orderbook.h"

class SimulatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_shared<core::Config>();
        config->load("/Users/vanshu/Desktop/demo/config.json");
        simulator = std::make_shared<models::Simulator>(config);
        simulator->init();
        orderBook = std::make_shared<core::OrderBook>();
    }

    std::shared_ptr<core::Config> config;
    std::shared_ptr<models::Simulator> simulator;
    std::shared_ptr<core::OrderBook> orderBook;
    const std::string exchange = "OKX";
    const std::string symbol = "BTC-USDT";
    const std::string timestamp = "2024-03-20T12:00:00Z";
};

TEST_F(SimulatorTest, InitialParameters) {
    EXPECT_EQ(simulator->getExchange(), "OKX");
    EXPECT_EQ(simulator->getAsset(), "BTC-USDT");
    EXPECT_EQ(simulator->getOrderType(), "market");
    EXPECT_DOUBLE_EQ(simulator->getQuantity(), 100.0);
    EXPECT_DOUBLE_EQ(simulator->getVolatility(), 1.0);
    EXPECT_EQ(simulator->getFeeTier(), "tier1");
}

TEST_F(SimulatorTest, SetParameters) {
    simulator->setExchange("Binance");
    simulator->setAsset("ETH-USDT");
    simulator->setOrderType("limit");
    simulator->setQuantity(200.0);
    simulator->setVolatility(2.0);
    simulator->setFeeTier("tier2");

    EXPECT_EQ(simulator->getExchange(), "Binance");
    EXPECT_EQ(simulator->getAsset(), "ETH-USDT");
    EXPECT_EQ(simulator->getOrderType(), "limit");
    EXPECT_DOUBLE_EQ(simulator->getQuantity(), 200.0);
    EXPECT_DOUBLE_EQ(simulator->getVolatility(), 2.0);
    EXPECT_EQ(simulator->getFeeTier(), "tier2");
}

TEST_F(SimulatorTest, SimulateWithEmptyOrderBook) {
    auto result = simulator->simulate(orderBook);
    EXPECT_DOUBLE_EQ(result.expectedSlippage, 0.0);
    EXPECT_DOUBLE_EQ(result.expectedFees, 0.0);
    EXPECT_DOUBLE_EQ(result.expectedMarketImpact, 0.0);
    EXPECT_DOUBLE_EQ(result.netCost, 0.0);
    EXPECT_DOUBLE_EQ(result.makerRatio, 0.0);
    EXPECT_DOUBLE_EQ(result.internalLatency, 0.0);
}

TEST_F(SimulatorTest, SimulateWithOrderBook) {
    std::vector<std::pair<std::string, std::string>> bids = {
        {"100.0", "1.0"},
        {"99.0", "2.0"}
    };
    
    std::vector<std::pair<std::string, std::string>> asks = {
        {"101.0", "1.0"},
        {"102.0", "2.0"}
    };
    
    orderBook->update(exchange, symbol, bids, asks, timestamp);
    auto result = simulator->simulate(orderBook);
    
    EXPECT_GE(result.expectedSlippage, 0.0);
    EXPECT_GE(result.expectedFees, 0.0);
    EXPECT_GE(result.expectedMarketImpact, 0.0);
    EXPECT_GE(result.netCost, 0.0);
    EXPECT_GE(result.makerRatio, 0.0);
    EXPECT_GE(result.internalLatency, 0.0);
}

TEST_F(SimulatorTest, ContinuousSimulation) {
    bool callbackCalled = false;
    simulator->registerResultCallback([&](const models::SimulationResult& result) {
        callbackCalled = true;
    });

    simulator->startContinuousSimulation(orderBook);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    simulator->stopContinuousSimulation();

    EXPECT_TRUE(callbackCalled);
} 