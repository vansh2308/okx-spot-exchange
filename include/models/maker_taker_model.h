#pragma once

#include <memory>
#include <vector>
#include "core/orderbook.h"
#include "core/utils.h"

namespace models {

class MakerTakerModel {
public:
    MakerTakerModel();
    ~MakerTakerModel();

    // Train the model with historical data
    void setTrainingData(const std::vector<double>& quantities,
                         const std::vector<double>& spreads,
                         const std::vector<double>& volatilities,
                         const std::vector<double>& makerRatios);
    bool train();
    
    // Predict maker-taker ratio for a given order
    double predictMakerRatio(const std::shared_ptr<core::OrderBook>& orderBook,
                           double quantity,
                           double volatility) const;
    
    // Probability of maker vs taker execution
    double predictMakerProbability(const std::shared_ptr<core::OrderBook>& orderBook,
                                 double quantity,
                                 double volatility) const;
    
    // Calculate probability curve over a range of quantities
    std::vector<std::pair<double, double>> calculateProbabilityCurve(
        const std::shared_ptr<core::OrderBook>& orderBook,
        double maxQuantity,
        double volatility,
        int steps) const;
    
    // Get model parameters and quality metrics
    double getModelAccuracy() const;
    std::vector<double> getCoefficients() const;

private:
    // Logistic regression model parameters
    std::vector<double> coefficients_; // [intercept, quantity_coef, spread_coef, volatility_coef]
    
    // Training data
    std::vector<double> quantityData_;
    std::vector<double> spreadData_;
    std::vector<double> volatilityData_;
    std::vector<double> makerRatioData_;
    
    // Logistic function
    double logistic(double z) const;
    
    // Predict using logistic regression
    double predict(double quantity, double spread, double volatility) const;
};

} // namespace models 