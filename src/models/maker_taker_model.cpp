#include "models/maker_taker_model.h"
#include "core/logger.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

namespace models {

MakerTakerModel::MakerTakerModel() {
    // Initialize with default coefficients
    coefficients_ = {0.0, -0.5, 2.0, -0.3}; // [intercept, quantity_coef, spread_coef, volatility_coef]
}

MakerTakerModel::~MakerTakerModel() {
    // Destructor
}

void MakerTakerModel::setTrainingData(const std::vector<double>& quantities,
                                    const std::vector<double>& spreads,
                                    const std::vector<double>& volatilities,
                                    const std::vector<double>& makerRatios) {
    // Validate input dimensions
    if (quantities.size() != spreads.size() || 
        quantities.size() != volatilities.size() || 
        quantities.size() != makerRatios.size()) {
        core::Logger::getInstance().error("Training data dimensions don't match");
        return;
    }
    
    // Store the training data
    quantityData_ = quantities;
    spreadData_ = spreads;
    volatilityData_ = volatilities;
    makerRatioData_ = makerRatios;
    
    core::Logger::getInstance().info("Training data set with {} samples", quantities.size());
}

bool MakerTakerModel::train() {
    if (quantityData_.empty() || spreadData_.empty() || 
        volatilityData_.empty() || makerRatioData_.empty()) {
        core::Logger::getInstance().warn("Cannot train model with empty data");
        return false;
    }
    
    // Normalize data
    double maxQuantity = *std::max_element(quantityData_.begin(), quantityData_.end());
    double maxSpread = *std::max_element(spreadData_.begin(), spreadData_.end());
    double maxVolatility = *std::max_element(volatilityData_.begin(), volatilityData_.end());
    
    std::vector<double> normQuantities;
    std::vector<double> normSpreads;
    std::vector<double> normVolatilities;
    
    for (size_t i = 0; i < quantityData_.size(); ++i) {
        normQuantities.push_back(quantityData_[i] / maxQuantity);
        normSpreads.push_back(spreadData_[i] / maxSpread);
        normVolatilities.push_back(volatilityData_[i] / maxVolatility);
    }
    
    // Logistic regression with gradient descent
    // Initialize coefficients
    coefficients_ = {0.0, 0.0, 0.0, 0.0};
    
    // Learning parameters
    double learningRate = 0.01;
    int maxIterations = 1000;
    double convergenceThreshold = 0.0001;
    
    // Gradient descent
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        std::vector<double> gradients = {0.0, 0.0, 0.0, 0.0};
        double prevCost = 0.0;
        
        // Calculate gradients
        for (size_t i = 0; i < normQuantities.size(); ++i) {
            double prediction = predict(normQuantities[i], normSpreads[i], normVolatilities[i]);
            double error = prediction - makerRatioData_[i];
            
            gradients[0] += error;
            gradients[1] += error * normQuantities[i];
            gradients[2] += error * normSpreads[i];
            gradients[3] += error * normVolatilities[i];
            
            prevCost += std::pow(error, 2);
        }
        
        // Average the gradients
        for (auto& gradient : gradients) {
            gradient /= normQuantities.size();
        }
        
        // Update coefficients
        for (size_t i = 0; i < 4; ++i) {
            coefficients_[i] -= learningRate * gradients[i];
        }
        
        // Check for convergence
        double cost = 0.0;
        for (size_t i = 0; i < normQuantities.size(); ++i) {
            double prediction = predict(normQuantities[i], normSpreads[i], normVolatilities[i]);
            double error = prediction - makerRatioData_[i];
            cost += std::pow(error, 2);
        }
        cost /= normQuantities.size();
        
        if (std::abs(prevCost - cost) < convergenceThreshold) {
            core::Logger::getInstance().info("Model converged after {} iterations", iteration);
            break;
        }
    }
    
    core::Logger::getInstance().info("Model trained with coefficients: [{}, {}, {}, {}]",
        coefficients_[0], coefficients_[1], coefficients_[2], coefficients_[3]);
    
    return true;
}

double MakerTakerModel::predictMakerRatio(const std::shared_ptr<core::OrderBook>& orderBook,
                                        double quantity,
                                        double volatility) const {
    if (!orderBook || quantity <= 0.0) {
        return 0.0; // Default to all taker orders
    }
    
    // Get spread
    double spread = orderBook->getSpread();
    double midPrice = orderBook->getMidPrice();
    
    // Normalize
    double normQuantity = quantity / 100.0;  // Assuming 100 BTC is a large order
    double normSpread = midPrice > 0.0 ? spread / midPrice : 0.0;
    double normVolatility = volatility;
    
    return predict(normQuantity, normSpread, normVolatility);
}

double MakerTakerModel::predictMakerProbability(const std::shared_ptr<core::OrderBook>& orderBook,
                                             double quantity,
                                             double volatility) const {
    // Maker probability is the same as maker ratio in our model
    return predictMakerRatio(orderBook, quantity, volatility);
}

std::vector<std::pair<double, double>> MakerTakerModel::calculateProbabilityCurve(
    const std::shared_ptr<core::OrderBook>& orderBook,
    double maxQuantity,
    double volatility,
    int steps) const {
    
    std::vector<std::pair<double, double>> curve;
    
    if (!orderBook || maxQuantity <= 0.0 || steps <= 0) {
        return curve;
    }
    
    // Calculate probability for different quantities
    for (int i = 0; i <= steps; ++i) {
        double quantity = maxQuantity * i / steps;
        double probability = predictMakerProbability(orderBook, quantity, volatility);
        curve.emplace_back(quantity, probability);
    }
    
    return curve;
}

double MakerTakerModel::getModelAccuracy() const {
    if (quantityData_.empty() || makerRatioData_.empty()) {
        return 0.0;
    }
    
    // Calculate accuracy as 1 - (mean squared error / variance)
    double sumSquaredError = 0.0;
    double meanMakerRatio = std::accumulate(makerRatioData_.begin(), makerRatioData_.end(), 0.0) / makerRatioData_.size();
    double totalVariance = 0.0;
    
    for (size_t i = 0; i < quantityData_.size(); ++i) {
        // Normalize
        double normQuantity = quantityData_[i] / 100.0;
        double normSpread = spreadData_[i];
        double normVolatility = volatilityData_[i];
        
        double prediction = predict(normQuantity, normSpread, normVolatility);
        double error = prediction - makerRatioData_[i];
        
        sumSquaredError += error * error;
        double variance = makerRatioData_[i] - meanMakerRatio;
        totalVariance += variance * variance;
    }
    
    if (totalVariance == 0.0) {
        return 0.0;
    }
    
    return 1.0 - (sumSquaredError / totalVariance);
}

std::vector<double> MakerTakerModel::getCoefficients() const {
    return coefficients_;
}

double MakerTakerModel::logistic(double z) const {
    // Logistic function: 1 / (1 + e^(-z))
    return 1.0 / (1.0 + std::exp(-z));
}

double MakerTakerModel::predict(double quantity, double spread, double volatility) const {
    // Linear combination of features
    double z = coefficients_[0] + 
               coefficients_[1] * quantity + 
               coefficients_[2] * spread + 
               coefficients_[3] * volatility;
    
    // Apply logistic function
    return logistic(z);
}

} // namespace models 