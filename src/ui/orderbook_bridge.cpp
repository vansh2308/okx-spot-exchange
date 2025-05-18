#include "ui/orderbook_bridge.h"
#include "core/logger.h"
#include <nlohmann/json.hpp>

namespace ui {

OrderBookBridge::OrderBookBridge(std::shared_ptr<processing::MessageProcessor> processor, QObject* parent)
    : QObject(parent)
    , processor_(processor)
    , timer_(new QTimer(this))
    , logger_(core::Logger::getInstance())
{
    connect(timer_, &QTimer::timeout, this, &OrderBookBridge::processMessage);
    timer_->setInterval(100); // Process messages every 100ms
}

OrderBookBridge::~OrderBookBridge() {
    stop();
}

void OrderBookBridge::start() {
    timer_->start();
}

void OrderBookBridge::stop() {
    timer_->stop();
}

void OrderBookBridge::processMessage() {
    processing::WebSocketMessage message = processor_->dequeue();
    if (!message.data.empty()) {
        try {
            // Parse JSON message
            auto json = nlohmann::json::parse(message.data);
            
            // Extract orderbook data
            std::vector<core::OrderBookLevel> bids, asks;
            
            // Parse bids
            for (const auto& bid : json["bids"]) {
                bids.push_back({
                    std::stod(bid[0].get<std::string>()),
                    std::stod(bid[1].get<std::string>())
                });
            }
            
            // Parse asks
            for (const auto& ask : json["asks"]) {
                asks.push_back({
                    std::stod(ask[0].get<std::string>()),
                    std::stod(ask[1].get<std::string>())
                });
            }
            
            // Emit signal with the parsed data
            emit orderBookUpdated(bids, asks);
            
        } catch (const std::exception& e) {
            logger_.error("Error processing message: {}", e.what());
        }
    }
}

} // namespace ui 