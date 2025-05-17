#pragma once

#include <memory>
#include <functional>
#include <string>
#include <map>
#include <chrono>
#include <mutex>
#include <atomic>

#include "core/orderbook.h"
#include "core/config.h"
#include "models/almgren_chriss.h"
#include "models/slippage_model.h"
#include "models/fee_model.h"
#include "models/maker_taker_model.h"

namespace models {

struct SimulationResult {
    double expectedSlippage;
    double expectedFees;
    double expectedMarketImpact;
    double netCost;
    double makerRatio;
    double internalLatency;  // measured processing time in microseconds
    std::chrono::system_clock::time_point timestamp;
};

class Simulator {
public:
    using ResultCallback = std::function<void(const SimulationResult&)>;

    Simulator(std::shared_ptr<core::Config> config);
    ~Simulator();

    // Initialize models
    void init();
    
    // Set simulation parameters
    void setExchange(const std::string& exchange);
    void setAsset(const std::string& asset);
    void setOrderType(const std::string& orderType);
    void setQuantity(double quantity);
    void setVolatility(double volatility);
    void setFeeTier(const std::string& feeTier);
    
    // Run simulation
    SimulationResult simulate(const std::shared_ptr<core::OrderBook>& orderBook);
    
    // Register callback for continuous simulation results
    void registerResultCallback(ResultCallback callback);
    void unregisterResultCallback();
    
    // Start/stop continuous simulation
    void startContinuousSimulation(const std::shared_ptr<core::OrderBook>& orderBook);
    void stopContinuousSimulation();
    bool isSimulationRunning() const;
    
    // Get current parameters
    std::string getExchange() const;
    std::string getAsset() const;
    std::string getOrderType() const;
    double getQuantity() const;
    double getVolatility() const;
    std::string getFeeTier() const;
    
    // Get latest result
    SimulationResult getLatestResult() const;

private:
    // Configuration
    std::shared_ptr<core::Config> config_;
    
    // Models
    std::shared_ptr<AlmgrenChrissModel> marketImpactModel_;
    std::shared_ptr<SlippageModel> slippageModel_;
    std::shared_ptr<FeeModel> feeModel_;
    std::shared_ptr<MakerTakerModel> makerTakerModel_;
    
    // Simulation parameters
    std::string exchange_;
    std::string asset_;
    std::string orderType_;
    double quantity_;
    double volatility_;
    std::string feeTier_;
    
    // Results and callback
    SimulationResult latestResult_;
    ResultCallback resultCallback_;
    mutable std::mutex resultMutex_;
    mutable std::mutex callbackMutex_;
    
    // Continuous simulation
    std::atomic<bool> continuousSimulationRunning_;
};

} // namespace models 