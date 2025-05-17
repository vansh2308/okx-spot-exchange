#include "models/simulator.h"
#include "core/logger.h"
#include <thread>
#include <sstream>

namespace models {

Simulator::Simulator(std::shared_ptr<core::Config> config)
    : config_(config), 
      quantity_(0.0),
      volatility_(0.2),
      continuousSimulationRunning_(false) {
    // Initialize default values
    exchange_ = config_->getDefaultExchange();
    asset_ = config_->getDefaultAsset();
    orderType_ = config_->getDefaultOrderType();
    quantity_ = config_->getDefaultQuantityUsd();
    volatility_ = config_->getDefaultVolatility();
    feeTier_ = config_->getDefaultFeeTier();
}

Simulator::~Simulator() {
    stopContinuousSimulation();
}

void Simulator::init() {
    // Create all model components
    marketImpactModel_ = std::make_shared<AlmgrenChrissModel>();
    slippageModel_ = std::make_shared<SlippageModel>();
    feeModel_ = std::make_shared<FeeModel>(config_);
    makerTakerModel_ = std::make_shared<MakerTakerModel>();
    
    // Set default values
    marketImpactModel_->setVolatility(volatility_);
    
    core::Logger::getInstance().info("Simulator initialized for {} on {}", asset_, exchange_);
}

void Simulator::setExchange(const std::string& exchange) {
    exchange_ = exchange;
}

void Simulator::setAsset(const std::string& asset) {
    asset_ = asset;
}

void Simulator::setOrderType(const std::string& orderType) {
    orderType_ = orderType;
}

void Simulator::setQuantity(double quantity) {
    if (quantity <= 0.0) {
        core::Logger::getInstance().warn("Invalid quantity: {}, must be positive", quantity);
        return;
    }
    
    quantity_ = quantity;
}

void Simulator::setVolatility(double volatility) {
    if (volatility <= 0.0) {
        core::Logger::getInstance().warn("Invalid volatility: {}, must be positive", volatility);
        return;
    }
    
    volatility_ = volatility;
    
    // Update market impact model
    if (marketImpactModel_) {
        marketImpactModel_->setVolatility(volatility_);
    }
}

void Simulator::setFeeTier(const std::string& feeTier) {
    feeTier_ = feeTier;
}

SimulationResult Simulator::simulate(const std::shared_ptr<core::OrderBook>& orderBook) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Initialize result with defaults
    SimulationResult result;
    result.expectedSlippage = 0.0;
    result.expectedFees = 0.0;
    result.expectedMarketImpact = 0.0;
    result.netCost = 0.0;
    result.makerRatio = 0.0;
    result.internalLatency = 0.0;
    result.timestamp = std::chrono::system_clock::now();
    
    if (!orderBook) {
        core::Logger::getInstance().warn("Cannot simulate with null order book");
        return result;
    }
    
    try {
        // Get current price
        double price = orderBook->getMidPrice();
        if (price <= 0.0) {
            return result;
        }
        
        // Determine if buy/sell based on quantity sign
        bool isBuy = (quantity_ >= 0.0);
        double absQuantity = std::abs(quantity_);
        
        // Convert USD quantity to asset quantity if needed
        double assetQuantity = absQuantity;
        if (orderType_ == "USD") {
            assetQuantity = absQuantity / price;
        }
        
        // Predict maker/taker ratio
        double makerRatio = 0.0;
        if (makerTakerModel_) {
            makerRatio = makerTakerModel_->predictMakerRatio(orderBook, assetQuantity, volatility_);
        }
        
        // Calculate slippage
        double slippagePct = 0.0;
        if (slippageModel_) {
            slippagePct = slippageModel_->calculateSlippage(orderBook, assetQuantity, isBuy);
        }
        
        // Calculate market impact
        double marketImpactPct = 0.0;
        if (marketImpactModel_) {
            marketImpactPct = marketImpactModel_->calculateMarketImpact(orderBook, assetQuantity, isBuy) / price;
        }
        
        // Calculate fees
        double fees = 0.0;
        if (feeModel_) {
            fees = feeModel_->calculateFees(exchange_, feeTier_, assetQuantity, price, makerRatio);
        }
        
        // Calculate net cost
        double slippageAmount = price * assetQuantity * slippagePct;
        double marketImpactAmount = price * assetQuantity * marketImpactPct;
        double netCost = slippageAmount + marketImpactAmount + fees;
        
        // Fill the result
        result.expectedSlippage = slippagePct * 100.0; // Convert to percentage
        result.expectedMarketImpact = marketImpactPct * 100.0; // Convert to percentage
        result.expectedFees = fees;
        result.netCost = netCost;
        result.makerRatio = makerRatio;
        result.timestamp = orderBook->getLastUpdateTime();
        
        // Calculate latency
        auto endTime = std::chrono::high_resolution_clock::now();
        result.internalLatency = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime).count();
        
        // Store latest result
        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            latestResult_ = result;
        }
        
        // Call callback if registered
        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (resultCallback_) {
                resultCallback_(result);
            }
        }
        
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Error in simulation: {}", e.what());
    }
    
    return result;
}

void Simulator::registerResultCallback(ResultCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    resultCallback_ = callback;
}

void Simulator::unregisterResultCallback() {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    resultCallback_ = nullptr;
}

void Simulator::startContinuousSimulation(const std::shared_ptr<core::OrderBook>& orderBook) {
    if (!orderBook) {
        core::Logger::getInstance().error("Cannot start continuous simulation with null order book");
        return;
    }
    
    if (isSimulationRunning()) {
        core::Logger::getInstance().info("Continuous simulation already running");
        return;
    }
    
    // Set flag
    continuousSimulationRunning_ = true;
    
    // Start simulation in a separate thread
    std::thread([this, orderBook]() {
        try {
            core::Logger::getInstance().info("Starting continuous simulation");
            
            while (continuousSimulationRunning_) {
                // Run one simulation
                simulate(orderBook);
                
                // Sleep for update interval
                int updateIntervalMs = config_ ? config_->getUpdateIntervalMs() : 1000;
                std::this_thread::sleep_for(std::chrono::milliseconds(updateIntervalMs));
            }
            
            core::Logger::getInstance().info("Continuous simulation stopped");
        } catch (const std::exception& e) {
            core::Logger::getInstance().error("Error in continuous simulation thread: {}", e.what());
            continuousSimulationRunning_ = false;
        }
    }).detach(); // Detach to run independently
}

void Simulator::stopContinuousSimulation() {
    continuousSimulationRunning_ = false;
    core::Logger::getInstance().info("Stopping continuous simulation");
}

bool Simulator::isSimulationRunning() const {
    return continuousSimulationRunning_;
}

std::string Simulator::getExchange() const {
    return exchange_;
}

std::string Simulator::getAsset() const {
    return asset_;
}

std::string Simulator::getOrderType() const {
    return orderType_;
}

double Simulator::getQuantity() const {
    return quantity_;
}

double Simulator::getVolatility() const {
    return volatility_;
}

std::string Simulator::getFeeTier() const {
    return feeTier_;
}

SimulationResult Simulator::getLatestResult() const {
    std::lock_guard<std::mutex> lock(resultMutex_);
    return latestResult_;
}

} // namespace models 