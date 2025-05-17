#pragma once

#include <memory>
#include <vector>
#include "core/orderbook.h"

namespace models {

class AlmgrenChrissModel {
public:
    AlmgrenChrissModel();
    ~AlmgrenChrissModel();

    // Set model parameters
    void setVolatility(double volatility);
    void setMarketImpactFactor(double factor);
    void setMarketRiskAversion(double riskAversion);
    
    // Calculate market impact for a given order
    double calculateMarketImpact(const std::shared_ptr<core::OrderBook>& orderBook, 
                                double quantity, 
                                bool isBuy) const;
    
    // Calculate optimal execution schedule for a large order
    struct ExecutionSchedule {
        std::vector<double> quantities;
        std::vector<double> times;
        double totalCost;
    };
    
    ExecutionSchedule calculateOptimalExecution(
        const std::shared_ptr<core::OrderBook>& orderBook,
        double totalQuantity,
        bool isBuy,
        int numSteps,
        double timeHorizon) const;
    
    // Get model parameters
    double getVolatility() const;
    double getMarketImpactFactor() const;
    double getMarketRiskAversion() const;

private:
    // Model parameters
    double volatility_;        // Market volatility (σ)
    double marketImpactFactor_; // Market impact factor (γ)
    double riskAversion_;      // Risk aversion parameter (λ)
    
    // Helper calculation methods
    double calculateTemporaryImpact(double rate, const std::shared_ptr<core::OrderBook>& orderBook) const;
    double calculatePermanentImpact(double quantity, const std::shared_ptr<core::OrderBook>& orderBook) const;
};

} // namespace models 