#pragma once

#include <memory>
#include <vector>
#include <map>
#include "core/orderbook.h"
#include "core/utils.h"

namespace models {

class SlippageModel {
public:
    enum class ModelType {
        LINEAR_REGRESSION,
        QUANTILE_REGRESSION,
        ORDERBOOK_BASED
    };

    SlippageModel(ModelType type = ModelType::ORDERBOOK_BASED);
    ~SlippageModel();

    // Set model parameters
    void setModelType(ModelType type);
    void setDataPoints(const std::vector<double>& quantities, const std::vector<double>& slippages);
    
    // Train the model with historical data
    bool train();
    
    // Predict slippage for a given order
    double predictSlippage(const std::shared_ptr<core::OrderBook>& orderBook, 
                         double quantity, 
                         bool isBuy) const;
    
    // Calculate slippage from actual order book data
    double calculateSlippage(const std::shared_ptr<core::OrderBook>& orderBook,
                           double quantity,
                           bool isBuy) const;
    
    // Calculate slippage for a range of quantities
    std::map<double, double> calculateSlippageProfile(const std::shared_ptr<core::OrderBook>& orderBook,
                                                   double maxQuantity,
                                                   bool isBuy,
                                                   int steps) const;
    
    // Get model parameters
    ModelType getModelType() const;
    core::utils::RegressionResult getRegressionResult() const;
    double getR2() const;

private:
    // Model type and parameters
    ModelType modelType_;
    core::utils::RegressionResult regression_;
    
    // Training data
    std::vector<double> quantityData_;
    std::vector<double> slippageData_;
    
    // Internal calculation methods
    double predictLinearSlippage(double quantity) const;
    double predictQuantileSlippage(double quantity, double quantile = 0.95) const;
    double predictOrderBookSlippage(const std::shared_ptr<core::OrderBook>& orderBook, 
                                 double quantity, 
                                 bool isBuy) const;
};

} // namespace models 