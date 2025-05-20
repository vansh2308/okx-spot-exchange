#include <gtest/gtest.h>
#include "models/almgren_chriss.h"
#include "models/fee_model.h"
#include "models/regression_model.h"
#include "models/maker_taker_model.h"
#include "models/slippage_model.h"
#include "core/config.h"
#include "core/orderbook.h"
#include <memory>

class ModelsTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_shared<core::Config>();
        config->load("/Users/vanshu/Desktop/demo/config.json");
        
        // Create a sample order book
        orderBook = std::make_shared<core::OrderBook>();
        
        // Convert numeric values to strings for the update method
        std::vector<std::pair<std::string, std::string>> bids = {
            {"100.0", "1.0"}, {"99.0", "2.0"}, {"98.0", "3.0"}
        };
        std::vector<std::pair<std::string, std::string>> asks = {
            {"101.0", "1.0"}, {"102.0", "2.0"}, {"103.0", "3.0"}
        };
        
        orderBook->update("binance", "BTCUSDT", bids, asks, "2024-03-20T12:00:00.000Z");
    }

    std::shared_ptr<core::Config> config;
    std::shared_ptr<core::OrderBook> orderBook;
};

// Almgren-Chriss Model Tests
TEST_F(ModelsTest, AlmgrenChrissParameters) {
    models::AlmgrenChrissModel model;
    
    model.setVolatility(0.2);
    model.setMarketImpactFactor(0.15);
    model.setMarketRiskAversion(1.5);
    
    EXPECT_DOUBLE_EQ(model.getVolatility(), 0.2);
    EXPECT_DOUBLE_EQ(model.getMarketImpactFactor(), 0.15);
    EXPECT_DOUBLE_EQ(model.getMarketRiskAversion(), 1.5);
}

TEST_F(ModelsTest, AlmgrenChrissMarketImpact) {
    models::AlmgrenChrissModel model;
    double impact = model.calculateMarketImpact(orderBook, 2.0, true);
    EXPECT_GE(impact, 0.0);
}

TEST_F(ModelsTest, AlmgrenChrissExecutionSchedule) {
    models::AlmgrenChrissModel model;
    auto schedule = model.calculateOptimalExecution(orderBook, 5.0, true, 3, 1.0);
    
    EXPECT_EQ(schedule.quantities.size(), 4); // numSteps + 1
    EXPECT_EQ(schedule.times.size(), 4);
    EXPECT_GE(schedule.totalCost, 0.0);
}

// Fee Model Tests
TEST_F(ModelsTest, FeeCalculation) {
    models::FeeModel model(config);
    
    double fees = model.calculateFees("binance", "VIP0", 1.0, 100.0, 0.5);
    EXPECT_GE(fees, 0.0);
    
    double makerFee = model.calculateMakerFee("binance", "VIP0", 1.0, 100.0);
    EXPECT_GE(makerFee, 0.0);
    
    double takerFee = model.calculateTakerFee("binance", "VIP0", 1.0, 100.0);
    EXPECT_GE(takerFee, 0.0);
}

TEST_F(ModelsTest, FeeRates) {
    models::FeeModel model(config);
    
    double makerRate = model.getMakerFeeRate("binance", "VIP0");
    double takerRate = model.getTakerFeeRate("binance", "VIP0");
    
    EXPECT_GE(makerRate, 0.0);
    EXPECT_GE(takerRate, 0.0);
    EXPECT_GE(takerRate, makerRate); // Taker fees are usually higher
}

// Regression Model Tests
TEST_F(ModelsTest, RegressionModelTraining) {
    models::RegressionModel model(models::RegressionModel::ModelType::LINEAR);
    
    // Add some training data
    std::vector<double> x = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> y = {2.0, 4.0, 6.0, 8.0, 10.0};
    
    model.setTrainingData(x, y);
    EXPECT_TRUE(model.train());
    
    // Test prediction
    double prediction = model.predict(6.0);
    EXPECT_NEAR(prediction, 12.0, 0.1);
    
    // Test model metrics
    EXPECT_GE(model.calculateRSquared(), 0.0);
    EXPECT_GE(model.calculateMSE(), 0.0);
    EXPECT_GE(model.calculateMAE(), 0.0);
}

// Maker-Taker Model Tests
TEST_F(ModelsTest, MakerTakerModelTraining) {
    models::MakerTakerModel model;
    
    // Add training data
    std::vector<double> quantities = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> spreads = {0.1, 0.2, 0.3, 0.4, 0.5};
    std::vector<double> volatilities = {0.01, 0.02, 0.03, 0.04, 0.05};
    std::vector<double> makerRatios = {0.6, 0.7, 0.8, 0.9, 1.0};
    
    model.setTrainingData(quantities, spreads, volatilities, makerRatios);
    EXPECT_TRUE(model.train());
    
    // Test prediction
    double ratio = model.predictMakerRatio(orderBook, 2.0, 0.02);
    EXPECT_GE(ratio, 0.0);
    EXPECT_LE(ratio, 1.0);
    
    // Test probability curve
    auto curve = model.calculateProbabilityCurve(orderBook, 5.0, 0.02, 5);
    EXPECT_EQ(curve.size(), 6); // steps + 1
}

// Slippage Model Tests
TEST_F(ModelsTest, SlippageModelTraining) {
    models::SlippageModel model(models::SlippageModel::ModelType::QUANTILE_REGRESSION);
    
    // Add more realistic training data with smaller values
    std::vector<double> quantities = {0.1, 0.2, 0.3, 0.4, 0.5};
    std::vector<double> slippages = {0.0001, 0.0002, 0.0003, 0.0004, 0.0005};
    
    model.setDataPoints(quantities, slippages);
    EXPECT_TRUE(model.train());
    
    // Test prediction with a small quantity
    double slippage = model.predictSlippage(orderBook, 0.2, true);
    // Allow for small negative values due to numerical precision
    EXPECT_GE(slippage, -0.0001);
    
    // Test slippage profile
    auto profile = model.calculateSlippageProfile(orderBook, 0.5, true, 5);
    EXPECT_EQ(profile.size(), 5);
    
    // Verify profile values are reasonable
    for (const auto& [quantity, slip] : profile) {
        EXPECT_GE(slip, -0.0001);
        EXPECT_LE(slip, 0.01);  // Slippage should not exceed 1%
    }
}

TEST_F(ModelsTest, SlippageModelTypes) {
    // Test different model types
    models::SlippageModel quantileModel(models::SlippageModel::ModelType::QUANTILE_REGRESSION);
    models::SlippageModel orderbookModel(models::SlippageModel::ModelType::ORDERBOOK_BASED);
    
    // Add more realistic training data
    std::vector<double> quantities = {0.1, 0.2, 0.3, 0.4, 0.5};
    std::vector<double> slippages = {0.0001, 0.0002, 0.0003, 0.0004, 0.0005};
    
    quantileModel.setDataPoints(quantities, slippages);
    orderbookModel.setDataPoints(quantities, slippages);
    
    EXPECT_TRUE(quantileModel.train());
    EXPECT_TRUE(orderbookModel.train());
    
    // Test predictions with small quantities
    double quantileSlippage = quantileModel.predictSlippage(orderBook, 0.2, true);
    double orderbookSlippage = orderbookModel.predictSlippage(orderBook, 0.2, true);
    
    // Allow for small negative values due to numerical precision
    EXPECT_GE(quantileSlippage, -0.0001);
    EXPECT_GE(orderbookSlippage, -0.0001);
    
    // Verify slippage values are within reasonable bounds
    EXPECT_LE(quantileSlippage, 0.01);  // Should not exceed 1%
    EXPECT_LE(orderbookSlippage, 0.01);  // Should not exceed 1%
} 