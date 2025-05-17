#pragma once

#include <string>
#include <memory>
#include "core/config.h"

namespace models {

class FeeModel {
public:
    FeeModel(std::shared_ptr<core::Config> config);
    ~FeeModel();

    // Calculate fees for an order
    double calculateFees(const std::string& exchange, 
                       const std::string& feeTier, 
                       double quantity, 
                       double price, 
                       double makerRatio) const;
    
    // Calculate maker fee component
    double calculateMakerFee(const std::string& exchange, 
                           const std::string& feeTier, 
                           double quantity, 
                           double price) const;
    
    // Calculate taker fee component
    double calculateTakerFee(const std::string& exchange, 
                           const std::string& feeTier, 
                           double quantity, 
                           double price) const;
    
    // Get fee rates
    double getMakerFeeRate(const std::string& exchange, const std::string& feeTier) const;
    double getTakerFeeRate(const std::string& exchange, const std::string& feeTier) const;
    
    // Available fee tiers
    std::vector<std::string> getFeeTiers(const std::string& exchange) const;

private:
    std::shared_ptr<core::Config> config_;
};

} // namespace models 