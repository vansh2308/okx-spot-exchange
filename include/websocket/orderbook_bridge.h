// orderbook_bridge.h
#pragma once
#include <QObject>
#include <QTimer>
#include "websocket/message_processor.h"

class OrderBookBridge : public QObject {
    Q_OBJECT

public:
    OrderBookBridge(std::shared_ptr<processing::MessageProcessor> processor, QObject* parent = nullptr);

public slots:
    void start();

signals:
    void orderBookUpdated(QVector<OrderLevel> bids, QVector<OrderLevel> asks);

private:
    QTimer timer_;
    std::shared_ptr<processing::MessageProcessor> processor_;
};
