#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include <vector>
#include "core/orderbook.h"
#include "websocket/message_processor.h"
#include "models/simulator.h"

namespace ui {

class Bridge : public QObject {
    Q_OBJECT

public:
    explicit Bridge(std::shared_ptr<processing::MessageProcessor> processor,
                   std::shared_ptr<models::Simulator> simulator,
                   QObject* parent = nullptr);
    ~Bridge();

    void start();
    void stop();

signals:
    void orderBookUpdated(const std::vector<core::OrderBookLevel>& bids, 
                         const std::vector<core::OrderBookLevel>& asks);
    void simulationUpdated(const models::SimulationResult& result);

private slots:
    void processMessage();

private:
    std::shared_ptr<processing::MessageProcessor> processor_;
    std::shared_ptr<models::Simulator> simulator_;
    std::shared_ptr<core::OrderBook> orderBook_;
    QTimer* timer_;
    core::Logger& logger_;
};

} // namespace ui 