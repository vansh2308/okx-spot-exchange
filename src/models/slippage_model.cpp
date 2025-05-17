#include "models/slippage_model.h"
#include "core/logger.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace models {

SlippageModel::SlippageModel(ModelType type)
    : modelType_(type) {
    // Initialize regression result with default values
    regression_.slope = 0.0;
    regression_.intercept = 0.0;
    regression_.r_squared = 0.0;
}

SlippageModel::~SlippageModel() {
    // Destructor
}

void SlippageModel::setModelType(ModelType type) {
    modelType_ = type;
}

void SlippageModel::setDataPoints(const std::vector<double>& quantities, const std::vector<double>& slippages) {
    if (quantities.size() != slippages.size()) {
        core::Logger::getInstance().error("Quantity and slippage vectors must have the same size");
        return;
    }
    
    quantityData_ = quantities;
    slippageData_ = slippages;
}

bool SlippageModel::train() {
    if (quantityData_.empty() || slippageData_.empty()) {
        core::Logger::getInstance().warn("Cannot train slippage model with empty data");
        return false;
    }
    
    if (quantityData_.size() != slippageData_.size()) {
        core::Logger::getInstance().error("Quantity and slippage vectors must have the same size");
        return false;
    }
    
    // Train the model based on the selected type
    if (modelType_ == ModelType::LINEAR_REGRESSION || modelType_ == ModelType::QUANTILE_REGRESSION) {
        // Use linear regression for both linear and quantile models
        regression_ = core::utils::linearRegression(quantityData_, slippageData_);
        
        core::Logger::getInstance().info("Slippage model trained: slope={}, intercept={}, R²={}",
            regression_.slope, regression_.intercept, regression_.r_squared);
        
        return true;
    } else {
        // OrderBook-based model doesn't need training
        core::Logger::getInstance().info("Using orderbook-based slippage model (no training required)");
        return true;
    }
}

double SlippageModel::predictSlippage(const std::shared_ptr<core::OrderBook>& orderBook, 
                                    double quantity, 
                                    bool isBuy) const {
    // Choose prediction method based on model type
    switch (modelType_) {
        case ModelType::LINEAR_REGRESSION:
            return predictLinearSlippage(quantity);
        
        case ModelType::QUANTILE_REGRESSION:
            return predictQuantileSlippage(quantity);
        
        case ModelType::ORDERBOOK_BASED:
        default:
            return predictOrderBookSlippage(orderBook, quantity, isBuy);
    }
}

double SlippageModel::calculateSlippage(const std::shared_ptr<core::OrderBook>& orderBook,
                                     double quantity,
                                     bool isBuy) const {
    // This always uses the orderbook directly
    return predictOrderBookSlippage(orderBook, quantity, isBuy);
}

std::map<double, double> SlippageModel::calculateSlippageProfile(const std::shared_ptr<core::OrderBook>& orderBook,
                                                              double maxQuantity,
                                                              bool isBuy,
                                                              int steps) const {
    std::map<double, double> profile;
    
    if (!orderBook || maxQuantity <= 0.0 || steps <= 0) {
        return profile;
    }
    
    // Calculate slippage for each step
    for (int i = 1; i <= steps; ++i) {
        double quantity = maxQuantity * i / steps;
        double slippage = predictSlippage(orderBook, quantity, isBuy);
        profile[quantity] = slippage;
    }
    
    return profile;
}

SlippageModel::ModelType SlippageModel::getModelType() const {
    return modelType_;
}

core::utils::RegressionResult SlippageModel::getRegressionResult() const {
    return regression_;
}

double SlippageModel::getR2() const {
    return regression_.r_squared;
}

double SlippageModel::predictLinearSlippage(double quantity) const {
    if (regression_.slope == 0.0 && regression_.intercept == 0.0) {
        // Model not trained yet
        return 0.0;
    }
    
    // Apply linear model: slippage = slope * quantity + intercept
    return regression_.slope * quantity + regression_.intercept;
}

double SlippageModel::predictQuantileSlippage(double quantity, double quantile) const {
    if (quantityData_.empty() || slippageData_.empty()) {
        return predictLinearSlippage(quantity);  // Fall back to linear model
    }
    
    // If the quantity is outside the training range, use linear extrapolation
    double minQuantity = *std::min_element(quantityData_.begin(), quantityData_.end());
    double maxQuantity = *std::max_element(quantityData_.begin(), quantityData_.end());
    
    if (quantity < minQuantity || quantity > maxQuantity) {
        return predictLinearSlippage(quantity);
    }
    
    // Find all data points with similar quantities
    std::vector<double> similarSlippages;
    double quantityRange = (maxQuantity - minQuantity) * 0.1;  // 10% window
    
    for (size_t i = 0; i < quantityData_.size(); ++i) {
        if (std::abs(quantityData_[i] - quantity) <= quantityRange) {
            similarSlippages.push_back(slippageData_[i]);
        }
    }
    
    if (similarSlippages.empty()) {
        return predictLinearSlippage(quantity);
    }
    
    // Calculate the specified quantile of the similar slippages
    return core::utils::percentile(similarSlippages, quantile);
}

double SlippageModel::predictOrderBookSlippage(const std::shared_ptr<core::OrderBook>& orderBook, 
                                            double quantity, 
                                            bool isBuy) const {
    if (!orderBook || quantity <= 0.0) {
        return 0.0;
    }
    
    // Get reference price
    double referencePrice = isBuy ? orderBook->getBestAsk() : orderBook->getBestBid();
    if (referencePrice <= 0.0) {
        return 0.0;
    }
    
    // Simulate market order execution
    double totalCost = 0.0;
    double remainingQuantity = quantity;
    
    // Get the price levels
    core::PriceLevels levels = isBuy ? orderBook->getAsks() : orderBook->getBids();
    
    // For sell orders, we want to start from highest price
    if (!isBuy) {
        std::reverse(levels.begin(), levels.end());
    }
    
    // Iterate through price levels
    for (const auto& level : levels) {
        double levelPrice = level.price;
        double levelQuantity = level.quantity;
        
        double takenQuantity = std::min(remainingQuantity, levelQuantity);
        totalCost += levelPrice * takenQuantity;
        remainingQuantity -= takenQuantity;
        
        if (remainingQuantity <= 0.0) {
            break;
        }
    }
    
    // If there's not enough liquidity, use the last price level
    if (remainingQuantity > 0.0 && !levels.empty()) {
        double lastPrice = levels.back().price;
        totalCost += lastPrice * remainingQuantity;
    }
    
    // Calculate average execution price
    double avgPrice = totalCost / quantity;
    
    // Calculate slippage as the difference between average execution price and reference price
    // For buys: positive slippage means paying more than reference
    // For sells: positive slippage means receiving less than reference
    double slippage = isBuy ? (avgPrice - referencePrice) : (referencePrice - avgPrice);
    
    // Convert to percentage of reference price
    return slippage / referencePrice;
}

} // namespace models 