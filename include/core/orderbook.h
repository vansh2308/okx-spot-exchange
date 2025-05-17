#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <utility>

namespace core {

struct OrderBookLevel {
    double price;
    double quantity;
};

using PriceLevels = std::vector<OrderBookLevel>;

class OrderBook {
public:
    OrderBook();
    ~OrderBook();

    // Update methods
    void update(const std::string& exchange, 
                const std::string& symbol, 
                const std::vector<std::pair<std::string, std::string>>& bids,
                const std::vector<std::pair<std::string, std::string>>& asks,
                const std::string& timestamp);

    // Snapshot retrieval
    PriceLevels getBids() const;
    PriceLevels getAsks() const;
    
    // Market data access
    double getBestBid() const;
    double getBestAsk() const;
    double getMidPrice() const;
    double getSpread() const;
    
    // Order book statistics
    double getDepthAtPrice(double price, bool isBid) const;
    double getTotalBidVolume() const;
    double getTotalAskVolume() const;
    double getImbalance() const; // bid volume / (bid volume + ask volume)
    
    // Market impact estimation
    double estimateMarketImpact(double quantity, bool isBuy) const;
    
    // Metadata
    std::string getExchange() const;
    std::string getSymbol() const;
    std::chrono::system_clock::time_point getTimestamp() const;
    std::chrono::system_clock::time_point getLastUpdateTime() const;
    
    // Performance metrics
    int getLevelsCount(bool isBid) const;
    double getUpdateFrequency() const; // updates per second

private:
    std::string exchange_;
    std::string symbol_;
    std::chrono::system_clock::time_point timestamp_; // from exchange
    std::chrono::system_clock::time_point lastUpdateTime_; // local time
    std::vector<std::chrono::system_clock::time_point> updateTimes_; // for calculating frequency
    
    std::map<double, double, std::greater<double>> bids_; // price -> quantity, sorted by price desc
    std::map<double, double> asks_; // price -> quantity, sorted by price asc
    
    mutable std::mutex mutex_;
    
    // Convert string price/quantity to double
    static double toDouble(const std::string& str);
};

} // namespace core 