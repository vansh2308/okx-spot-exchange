#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include <vector>
#include "core/orderbook.h"
#include "websocket/message_processor.h"

namespace ui {

class OrderBookBridge : public QObject {
    Q_OBJECT

public:
    explicit OrderBookBridge(std::shared_ptr<processing::MessageProcessor> processor, QObject* parent = nullptr);
    ~OrderBookBridge();

    void start();
    void stop();

signals:
    void orderBookUpdated(const std::vector<core::OrderBookLevel>& bids, 
                         const std::vector<core::OrderBookLevel>& asks);

private slots:
    void processMessage();

private:
    std::shared_ptr<processing::MessageProcessor> processor_;
    QTimer* timer_;
    core::Logger& logger_;
};

} // namespace ui 