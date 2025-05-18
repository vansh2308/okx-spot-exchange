// orderbook_bridge.cpp
#include "orderbook_bridge.h"
#include <nlohmann/json.hpp>

OrderBookBridge::OrderBookBridge(std::shared_ptr<processing::MessageProcessor> processor, QObject* parent)
    : QObject(parent), processor_(processor) {
    connect(&timer_, &QTimer::timeout, this, [=]() {
        auto msg = processor_->dequeue();
        if (msg.data.empty()) return;

        try {
            auto json = nlohmann::json::parse(msg.data);
            QVector<OrderLevel> bids, asks;

            for (const auto& bid : json["bids"]) {
                bids.append({QString::fromStdString(bid[0]), QString::fromStdString(bid[1])});
            }

            for (const auto& ask : json["asks"]) {
                asks.append({QString::fromStdString(ask[0]), QString::fromStdString(ask[1])});
            }

            emit orderBookUpdated(bids, asks);

        } catch (...) {}
    });
}

void OrderBookBridge::start() {
    timer_.start(50); // poll every 50ms
}
