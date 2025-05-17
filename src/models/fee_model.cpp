#include "models/fee_model.h"
#include "core/logger.h"

namespace models {

FeeModel::FeeModel(std::shared_ptr<core::Config> config)
    : config_(config) {
    // Constructor
}

FeeModel::~FeeModel() {
    // Destructor
}

double FeeModel::calculateFees(const std::string& exchange, 
                             const std::string& feeTier, 
                             double quantity, 
                             double price, 
                             double makerRatio) const {
    if (quantity <= 0.0 || price <= 0.0) {
        core::Logger::getInstance().warn("Invalid quantity or price for fee calculation");
        return 0.0;
    }
    
    if (makerRatio < 0.0 || makerRatio > 1.0) {
        core::Logger::getInstance().warn("Invalid maker ratio (must be between 0.0 and 1.0): {}", makerRatio);
        makerRatio = 0.0; // Assume full taker for safety
    }
    
    // Calculate notional value (quantity * price)
    double notionalValue = quantity * price;
    
    // Get fee rates
    double makerFeeRate = getMakerFeeRate(exchange, feeTier);
    double takerFeeRate = getTakerFeeRate(exchange, feeTier);
    
    // Calculate maker and taker components
    double makerFee = notionalValue * makerFeeRate * makerRatio;
    double takerFee = notionalValue * takerFeeRate * (1.0 - makerRatio);
    
    // Total fee
    return makerFee + takerFee;
}

double FeeModel::calculateMakerFee(const std::string& exchange, 
                                 const std::string& feeTier, 
                                 double quantity, 
                                 double price) const {
    if (quantity <= 0.0 || price <= 0.0) {
        return 0.0;
    }
    
    double notionalValue = quantity * price;
    double makerFeeRate = getMakerFeeRate(exchange, feeTier);
    
    return notionalValue * makerFeeRate;
}

double FeeModel::calculateTakerFee(const std::string& exchange, 
                                 const std::string& feeTier, 
                                 double quantity, 
                                 double price) const {
    if (quantity <= 0.0 || price <= 0.0) {
        return 0.0;
    }
    
    double notionalValue = quantity * price;
    double takerFeeRate = getTakerFeeRate(exchange, feeTier);
    
    return notionalValue * takerFeeRate;
}

double FeeModel::getMakerFeeRate(const std::string& exchange, const std::string& feeTier) const {
    if (!config_) {
        core::Logger::getInstance().error("Config not set in FeeModel");
        return 0.0;
    }
    
    return config_->getMakerFee(exchange, feeTier);
}

double FeeModel::getTakerFeeRate(const std::string& exchange, const std::string& feeTier) const {
    if (!config_) {
        core::Logger::getInstance().error("Config not set in FeeModel");
        return 0.0;
    }
    
    return config_->getTakerFee(exchange, feeTier);
}

std::vector<std::string> FeeModel::getFeeTiers(const std::string& exchange) const {
    if (!config_) {
        core::Logger::getInstance().error("Config not set in FeeModel");
        return {};
    }
    
    std::vector<std::string> tiers;
    auto feeTiers = config_->getFeeTiers(exchange);
    
    for (const auto& tier : feeTiers) {
        tiers.push_back(tier.name);
    }
    
    return tiers;
}

} // namespace models 