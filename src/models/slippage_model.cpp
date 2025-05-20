#include "models/slippage_model.h"
#include "core/logger.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <Eigen/Dense>

namespace models {

SlippageModel::SlippageModel(ModelType type)
    : modelType_(type) {
    // Initialize quantile regression parameters
    quantiles_ = {0.1, 0.25, 0.5, 0.75, 0.9};
    coefficients_.resize(quantiles_.size());
    
    // Initialize feature weights
    featureWeights_ = {
        {"volume", 0.4},
        {"spread", 0.3},
        {"volatility", 0.2},
        {"time_of_day", 0.1}
    };
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
    
    // Convert to DataPoint format for training
    trainingData_.clear();
    trainingData_.reserve(quantities.size());
    
    for (size_t i = 0; i < quantities.size(); ++i) {
        DataPoint point;
        point.volume = quantities[i];
        point.slippage = slippages[i];
        point.spread = currentSpread_;
        point.volatility = currentVolatility_;
        point.timeOfDay = getTimeOfDay();
        trainingData_.push_back(point);
    }
}

bool SlippageModel::train() {
    if (trainingData_.empty()) {
        core::Logger::getInstance().warn("Cannot train slippage model with empty data");
        return false;
    }
    
    if (modelType_ == ModelType::QUANTILE_REGRESSION) {
        return trainQuantileRegression();
    } else if (modelType_ == ModelType::ORDERBOOK_BASED) {
        core::Logger::getInstance().info("Using orderbook-based slippage model (no training required)");
        return true;
    }
    
    return false;
}

bool SlippageModel::trainQuantileRegression() {
    try {
        // Prepare feature matrix and target vector
        Eigen::MatrixXd X(trainingData_.size(), 4);  // 4 features
        Eigen::VectorXd y(trainingData_.size());
        
        for (size_t i = 0; i < trainingData_.size(); ++i) {
            const auto& point = trainingData_[i];
            X(i, 0) = point.volume;
            X(i, 1) = point.spread;
            X(i, 2) = point.volatility;
            X(i, 3) = point.timeOfDay;
            y(i) = point.slippage;
        }
        
        // Train for each quantile
        for (size_t q = 0; q < quantiles_.size(); ++q) {
            double quantile = quantiles_[q];
            coefficients_[q] = fitQuantileRegression(X, y, quantile);
        }
        
        // Calculate model metrics
        calculateModelMetrics();
        
        core::Logger::getInstance().info("Quantile regression model trained successfully with {} data points",
            trainingData_.size());
        return true;
        
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Error training quantile regression model: {}", e.what());
        return false;
    }
}

Eigen::VectorXd SlippageModel::fitQuantileRegression(const Eigen::MatrixXd& X, 
                                                    const Eigen::VectorXd& y,
                                                    double quantile) {
    // Initialize parameters
    Eigen::VectorXd beta = Eigen::VectorXd::Zero(X.cols());
    double learningRate = 0.01;
    int maxIterations = 1000;
    double tolerance = 1e-6;
    
    // Iterative optimization using gradient descent
    for (int iter = 0; iter < maxIterations; ++iter) {
        Eigen::VectorXd residuals = y - X * beta;
        Eigen::VectorXd gradient = Eigen::VectorXd::Zero(X.cols());
        
        // Calculate gradient for quantile regression
        for (int i = 0; i < X.rows(); ++i) {
            double residual = residuals(i);
            double weight = (residual < 0) ? (1 - quantile) : quantile;
            gradient += weight * X.row(i).transpose() * residual;
        }
        
        // Update parameters
        Eigen::VectorXd newBeta = beta - learningRate * gradient;
        
        // Check convergence
        if ((newBeta - beta).norm() < tolerance) {
            break;
        }
        
        beta = newBeta;
    }
    
    return beta;
}

double SlippageModel::predictSlippage(const std::shared_ptr<core::OrderBook>& orderBook, 
                                    double quantity, 
                                    bool isBuy) const {
    if (modelType_ == ModelType::QUANTILE_REGRESSION) {
        return predictQuantileSlippage(quantity, 0.5);  // Use median by default
    } else {
        return predictOrderBookSlippage(orderBook, quantity, isBuy);
    }
}

double SlippageModel::predictQuantileSlippage(double quantity, double quantile) const {
    if (coefficients_.empty()) {
        return 0.0;
    }
    
    // Find the closest quantile
    size_t quantileIndex = 0;
    double minDiff = std::abs(quantiles_[0] - quantile);
    
    for (size_t i = 1; i < quantiles_.size(); ++i) {
        double diff = std::abs(quantiles_[i] - quantile);
        if (diff < minDiff) {
            minDiff = diff;
            quantileIndex = i;
        }
    }
    
    // Prepare feature vector
    Eigen::VectorXd features(4);
    features(0) = quantity;  // volume
    features(1) = currentSpread_;  // spread
    features(2) = currentVolatility_;  // volatility
    features(3) = getTimeOfDay();  // time of day
    
    // Apply feature weights
    features(0) *= featureWeights_.at("volume");
    features(1) *= featureWeights_.at("spread");
    features(2) *= featureWeights_.at("volatility");
    features(3) *= featureWeights_.at("time_of_day");
    
    // Predict using the selected quantile's coefficients
    return features.dot(coefficients_[quantileIndex]);
}

void SlippageModel::calculateModelMetrics() {
    if (trainingData_.empty() || coefficients_.empty()) {
        return;
    }
    
    // Calculate metrics for each quantile
    for (size_t q = 0; q < quantiles_.size(); ++q) {
        double quantile = quantiles_[q];
        const auto& beta = coefficients_[q];
        
        // Calculate predictions
        std::vector<double> predictions;
        std::vector<double> actuals;
        
        for (const auto& point : trainingData_) {
            Eigen::VectorXd features(4);
            features(0) = point.volume;
            features(1) = point.spread;
            features(2) = point.volatility;
            features(3) = point.timeOfDay;
            
            // Apply feature weights
            features(0) *= featureWeights_.at("volume");
            features(1) *= featureWeights_.at("spread");
            features(2) *= featureWeights_.at("volatility");
            features(3) *= featureWeights_.at("time_of_day");
            
            predictions.push_back(features.dot(beta));
            actuals.push_back(point.slippage);
        }
        
        // Calculate metrics
        double mae = calculateMAE(predictions, actuals);
        double mse = calculateMSE(predictions, actuals);
        double r2 = calculateR2(predictions, actuals);
        
        core::Logger::getInstance().info("Quantile {} metrics - MAE: {:.6f}, MSE: {:.6f}, R²: {:.6f}",
            quantile, mae, mse, r2);
    }
}

double SlippageModel::calculateMAE(const std::vector<double>& predictions,
                                 const std::vector<double>& actuals) const {
    double sum = 0.0;
    for (size_t i = 0; i < predictions.size(); ++i) {
        sum += std::abs(predictions[i] - actuals[i]);
    }
    return sum / predictions.size();
}

double SlippageModel::calculateMSE(const std::vector<double>& predictions,
                                 const std::vector<double>& actuals) const {
    double sum = 0.0;
    for (size_t i = 0; i < predictions.size(); ++i) {
        double diff = predictions[i] - actuals[i];
        sum += diff * diff;
    }
    return sum / predictions.size();
}

double SlippageModel::calculateR2(const std::vector<double>& predictions,
                                const std::vector<double>& actuals) const {
    double mean = std::accumulate(actuals.begin(), actuals.end(), 0.0) / actuals.size();
    
    double ssTotal = 0.0;
    double ssResidual = 0.0;
    
    for (size_t i = 0; i < predictions.size(); ++i) {
        ssTotal += std::pow(actuals[i] - mean, 2);
        ssResidual += std::pow(actuals[i] - predictions[i], 2);
    }
    
    return 1.0 - (ssResidual / ssTotal);
}

double SlippageModel::getTimeOfDay() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    return (tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec) / 86400.0;  // Normalize to [0,1]
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