#include "core/orderbook.h"
#include "core/utils.h"
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <chrono>

namespace core {

OrderBook::OrderBook() {
    // Constructor
}

OrderBook::~OrderBook() {
    // Destructor
}

void OrderBook::update(const std::string& exchange, 
                     const std::string& symbol, 
                     const std::vector<std::pair<std::string, std::string>>& bids,
                     const std::vector<std::pair<std::string, std::string>>& asks,
                     const std::string& timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    exchange_ = exchange;
    symbol_ = symbol;
    
    // Parse timestamp
    timestamp_ = utils::parseISOTimestamp(timestamp);
    
    // Record update time
    lastUpdateTime_ = utils::currentTime();
    updateTimes_.push_back(lastUpdateTime_);
    
    // Keep only the last 100 update times for frequency calculation
    if (updateTimes_.size() > 100) {
        updateTimes_.erase(updateTimes_.begin());
    }
    
    // Clear and update bids
    bids_.clear();
    for (const auto& [priceStr, quantityStr] : bids) {
        double price = toDouble(priceStr);
        double quantity = toDouble(quantityStr);
        
        if (price > 0 && quantity > 0) {
            bids_[price] = quantity;
        }
    }
    
    // Clear and update asks
    asks_.clear();
    for (const auto& [priceStr, quantityStr] : asks) {
        double price = toDouble(priceStr);
        double quantity = toDouble(quantityStr);
        
        if (price > 0 && quantity > 0) {
            asks_[price] = quantity;
        }
    }
}

PriceLevels OrderBook::getBids() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    PriceLevels result;
    for (const auto& [price, quantity] : bids_) {
        result.push_back({price, quantity});
    }
    return result;
}

PriceLevels OrderBook::getAsks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    PriceLevels result;
    for (const auto& [price, quantity] : asks_) {
        result.push_back({price, quantity});
    }
    return result;
}

double OrderBook::getBestBid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (bids_.empty()) {
        return 0.0;
    }
    
    return bids_.begin()->first;
}

double OrderBook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (asks_.empty()) {
        return 0.0;
    }
    
    return asks_.begin()->first;
}

double OrderBook::getMidPrice() const {
    double bestBid = getBestBid();
    double bestAsk = getBestAsk();
    
    if (bestBid <= 0.0 || bestAsk <= 0.0) {
        return 0.0;
    }
    
    return (bestBid + bestAsk) / 2.0;
}

double OrderBook::getSpread() const {
    double bestBid = getBestBid();
    double bestAsk = getBestAsk();
    
    if (bestBid <= 0.0 || bestAsk <= 0.0) {
        return 0.0;
    }
    
    return bestAsk - bestBid;
}

double OrderBook::getDepthAtPrice(double price, bool isBid) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isBid) {
        auto it = bids_.find(price);
        return (it != bids_.end()) ? it->second : 0.0;
    } else {
        auto it = asks_.find(price);
        return (it != asks_.end()) ? it->second : 0.0;
    }
}

double OrderBook::getTotalBidVolume() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    return std::accumulate(bids_.begin(), bids_.end(), 0.0, 
        [](double sum, const auto& pair) { return sum + pair.second; });
}

double OrderBook::getTotalAskVolume() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    return std::accumulate(asks_.begin(), asks_.end(), 0.0, 
        [](double sum, const auto& pair) { return sum + pair.second; });
}

double OrderBook::getImbalance() const {
    double bidVolume = getTotalBidVolume();
    double askVolume = getTotalAskVolume();
    
    if (bidVolume <= 0.0 || askVolume <= 0.0) {
        return 0.0;
    }
    
    return bidVolume / (bidVolume + askVolume);
}

double OrderBook::estimateMarketImpact(double quantity, bool isBuy) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    double remainingQuantity = quantity;
    double totalPrice = 0.0;
    
    double referencePrice = isBuy ? getBestAsk() : getBestBid();
    
    if (referencePrice <= 0.0) {
        return 0.0;
    }
    
    // Handle asks_ and bids_ separately instead of using a common reference
    if (isBuy) {
        // Process asks for buy orders
        for (const auto& [price, available] : asks_) {
            double take = std::min(remainingQuantity, available);
            totalPrice += price * take;
            remainingQuantity -= take;
            
            if (remainingQuantity <= 0.0) {
                break;
            }
        }
        
        // If we couldn't fill the entire order with the available liquidity
        if (remainingQuantity > 0.0) {
            // Use the last available price for the remaining quantity
            if (!asks_.empty()) {
                double lastPrice = asks_.rbegin()->first;
                totalPrice += lastPrice * remainingQuantity;
            } else {
                return 0.0; // No liquidity at all
            }
        }
    } else {
        // Process bids for sell orders
        for (const auto& [price, available] : bids_) {
            double take = std::min(remainingQuantity, available);
            totalPrice += price * take;
            remainingQuantity -= take;
            
            if (remainingQuantity <= 0.0) {
                break;
            }
        }
        
        // If we couldn't fill the entire order with the available liquidity
        if (remainingQuantity > 0.0) {
            // Use the last available price for the remaining quantity
            if (!bids_.empty()) {
                double lastPrice = bids_.begin()->first;
                totalPrice += lastPrice * remainingQuantity;
            } else {
                return 0.0; // No liquidity at all
            }
        }
    }
    
    double avgPrice = totalPrice / quantity;
    return isBuy ? (avgPrice - referencePrice) : (referencePrice - avgPrice);
}

std::string OrderBook::getExchange() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return exchange_;
}

std::string OrderBook::getSymbol() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return symbol_;
}

std::chrono::system_clock::time_point OrderBook::getTimestamp() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return timestamp_;
}

std::chrono::system_clock::time_point OrderBook::getLastUpdateTime() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastUpdateTime_;
}

int OrderBook::getLevelsCount(bool isBid) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    return isBid ? bids_.size() : asks_.size();
}

double OrderBook::getUpdateFrequency() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (updateTimes_.size() < 2) {
        return 0.0;
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        updateTimes_.back() - updateTimes_.front()).count();
    
    if (duration <= 0) {
        return 0.0;
    }
    
    // Calculate updates per second
    return (updateTimes_.size() - 1) * 1000.0 / duration;
}

double OrderBook::toDouble(const std::string& str) {
    try {
        return std::stod(str);
    } catch (const std::exception& e) {
        return 0.0;
    }
}

} // namespace core 