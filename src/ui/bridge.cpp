#include "ui/bridge.h"
#include "core/logger.h"
#include <nlohmann/json.hpp>

namespace ui {

Bridge::Bridge(std::shared_ptr<processing::MessageProcessor> processor,
               std::shared_ptr<models::Simulator> simulator,
               QObject* parent)
    : QObject(parent)
    , processor_(processor)
    , simulator_(simulator)
    , orderBook_(std::make_shared<core::OrderBook>())
    , timer_(new QTimer(this))
    , logger_(core::Logger::getInstance())
{
    connect(timer_, &QTimer::timeout, this, &Bridge::processMessage);
    timer_->setInterval(100); // Process messages every 100ms
}

Bridge::~Bridge() {
    stop();
}

void Bridge::start() {
    timer_->start();
}

void Bridge::stop() {
    timer_->stop();
}

void Bridge::processMessage() {
    processing::WebSocketMessage message = processor_->dequeue();
    if (!message.data.empty()) {
        try {
            // Parse JSON message
            auto json = nlohmann::json::parse(message.data);
            
            // Extract orderbook data
            std::vector<core::OrderBookLevel> bids, asks;
            std::vector<std::pair<std::string, std::string>> bidPairs, askPairs;
            
            // Parse bids
            for (const auto& bid : json["bids"]) {
                bids.push_back({
                    std::stod(bid[0].get<std::string>()),
                    std::stod(bid[1].get<std::string>())
                });
                bidPairs.emplace_back(bid[0].get<std::string>(), bid[1].get<std::string>());
            }
            
            // Parse asks
            for (const auto& ask : json["asks"]) {
                asks.push_back({
                    std::stod(ask[0].get<std::string>()),
                    std::stod(ask[1].get<std::string>())
                });
                askPairs.emplace_back(ask[0].get<std::string>(), ask[1].get<std::string>());
            }
            
            // Update orderbook with string pairs
            orderBook_->update(json["exchange"], json["symbol"], bidPairs, askPairs, json["timestamp"].get<std::string>());
            
            // Run simulation
            auto result = simulator_->simulate(orderBook_);
            
            // Emit signals with OrderBookLevel vectors
            emit orderBookUpdated(bids, asks);
            emit simulationUpdated(result);
            
        } catch (const std::exception& e) {
            logger_.error("Error processing message: {}", e.what());
        }
    }
}

} // namespace ui 