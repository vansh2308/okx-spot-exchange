#include "models/almgren_chriss.h"
#include "core/logger.h"
#include <cmath>
#include <numeric>

namespace models {

AlmgrenChrissModel::AlmgrenChrissModel()
    : volatility_(0.3),      // Default volatility parameter
      marketImpactFactor_(0.1), // Default market impact factor
      riskAversion_(1.0) {   // Default risk aversion parameter
}

AlmgrenChrissModel::~AlmgrenChrissModel() {
    // Destructor
}

void AlmgrenChrissModel::setVolatility(double volatility) {
    if (volatility <= 0.0) {
        core::Logger::getInstance().warn("Invalid volatility value: {}, using default", volatility);
        return;
    }
    
    volatility_ = volatility;
}

void AlmgrenChrissModel::setMarketImpactFactor(double factor) {
    if (factor <= 0.0) {
        core::Logger::getInstance().warn("Invalid market impact factor: {}, using default", factor);
        return;
    }
    
    marketImpactFactor_ = factor;
}

void AlmgrenChrissModel::setMarketRiskAversion(double riskAversion) {
    if (riskAversion < 0.0) {
        core::Logger::getInstance().warn("Invalid risk aversion: {}, using default", riskAversion);
        return;
    }
    
    riskAversion_ = riskAversion;
}

double AlmgrenChrissModel::calculateMarketImpact(const std::shared_ptr<core::OrderBook>& orderBook, 
                                              double quantity, 
                                              bool isBuy) const {
    if (!orderBook || quantity <= 0.0) {
        return 0.0;
    }
    
    // Get reference price (mid price)
    double referencePrice = orderBook->getMidPrice();
    if (referencePrice <= 0.0) {
        return 0.0;
    }
    
    // Calculate market depth and imbalance
    double totalVolume = isBuy ? orderBook->getTotalAskVolume() : orderBook->getTotalBidVolume();
    if (totalVolume <= 0.0) {
        return 0.0;
    }
    
    // Calculate volume ratio (order size relative to available liquidity)
    double volumeRatio = quantity / totalVolume;
    
    // Calculate temporary impact using square root model
    double temporaryImpact = calculateTemporaryImpact(volumeRatio, orderBook);
    
    // Calculate permanent impact
    double permanentImpact = calculatePermanentImpact(quantity, orderBook);
    
    // Total impact is the sum of temporary and permanent impacts
    return temporaryImpact + permanentImpact;
}

AlmgrenChrissModel::ExecutionSchedule AlmgrenChrissModel::calculateOptimalExecution(
    const std::shared_ptr<core::OrderBook>& orderBook,
    double totalQuantity,
    bool isBuy,
    int numSteps,
    double timeHorizon) const {
    
    ExecutionSchedule schedule;
    
    if (!orderBook || totalQuantity <= 0.0 || numSteps <= 0 || timeHorizon <= 0.0) {
        return schedule;
    }
    
    // Get reference price
    double referencePrice = orderBook->getMidPrice();
    if (referencePrice <= 0.0) {
        return schedule;
    }
    
    // Calculate Almgren-Chriss parameters
    double sigma = volatility_ * referencePrice; // Price volatility in absolute terms
    double eta = marketImpactFactor_ * referencePrice; // Temporary impact factor
    double gamma = marketImpactFactor_ * 0.1 * referencePrice; // Permanent impact factor (typically smaller than temporary)
    
    // Calculate optimal trading trajectory
    double tau = timeHorizon / numSteps;
    double kappa = std::sqrt(riskAversion_ * sigma * sigma / eta); // Trading rate parameter
    
    // Hyperbolic functions for the solution
    double sinh_kT = std::sinh(kappa * timeHorizon);
    double cosh_kT = std::cosh(kappa * timeHorizon);
    
    schedule.quantities.resize(numSteps + 1, 0.0);
    schedule.times.resize(numSteps + 1, 0.0);
    
    // Initial quantity is the total quantity
    schedule.quantities[0] = totalQuantity;
    schedule.times[0] = 0.0;
    
    // Calculate remaining quantities at each time step
    for (int i = 1; i <= numSteps; ++i) {
        double t = i * tau;
        schedule.times[i] = t;
        
        // Almgren-Chriss formula for optimal remaining quantity
        double remainingRatio = sinh_kT != 0.0 ? 
            std::sinh(kappa * (timeHorizon - t)) / sinh_kT : 
            (numSteps - i) / static_cast<double>(numSteps);
            
        schedule.quantities[i] = totalQuantity * remainingRatio;
    }
    
    // Calculate total cost
    double totalCost = 0.0;
    for (int i = 0; i < numSteps; ++i) {
        double tradeSize = schedule.quantities[i] - schedule.quantities[i+1];
        
        // Temporary impact cost for this trade
        double tempImpact = eta * std::pow(tradeSize / tau, 0.5);
        
        // Permanent impact cost (cumulative)
        double permImpact = gamma * tradeSize;
        
        // Add to total cost
        totalCost += tradeSize * (tempImpact + permImpact / 2.0);
    }
    
    schedule.totalCost = totalCost;
    
    return schedule;
}

double AlmgrenChrissModel::getVolatility() const {
    return volatility_;
}

double AlmgrenChrissModel::getMarketImpactFactor() const {
    return marketImpactFactor_;
}

double AlmgrenChrissModel::getMarketRiskAversion() const {
    return riskAversion_;
}

double AlmgrenChrissModel::calculateTemporaryImpact(double rate, const std::shared_ptr<core::OrderBook>& orderBook) const {
    // Temporary impact model: η * √(rate)
    // η is the market impact factor, rate is the trading rate
    
    if (!orderBook) {
        return 0.0;
    }
    
    double referencePrice = orderBook->getMidPrice();
    double sigma = volatility_ * referencePrice; // Price volatility
    
    // Calculate average spread as a proxy for market liquidity
    double spread = orderBook->getSpread();
    double midPrice = orderBook->getMidPrice();
    double relativeSpread = midPrice > 0.0 ? spread / midPrice : 0.001;
    
    // Adjust impact factor based on spread
    double adjustedFactor = marketImpactFactor_ * (1.0 + 10.0 * relativeSpread);
    
    // Use square root model for temporary impact
    return adjustedFactor * referencePrice * std::sqrt(rate);
}

double AlmgrenChrissModel::calculatePermanentImpact(double quantity, const std::shared_ptr<core::OrderBook>& orderBook) const {
    // Permanent impact model: γ * quantity
    // γ is the permanent impact factor
    
    if (!orderBook) {
        return 0.0;
    }
    
    double referencePrice = orderBook->getMidPrice();
    
    // Calculate total volume as a measure of market depth
    double totalVolume = orderBook->getTotalBidVolume() + orderBook->getTotalAskVolume();
    
    if (totalVolume <= 0.0) {
        return 0.0;
    }
    
    // Volume ratio (order size relative to total volume)
    double volumeRatio = quantity / totalVolume;
    
    // Permanent impact factor (typically smaller than temporary impact)
    double gamma = marketImpactFactor_ * 0.1;
    
    // Linear permanent impact model
    return gamma * referencePrice * volumeRatio;
}

} // namespace models 