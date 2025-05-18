// orderbook_view.h
#pragma once

#include <QWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QLabel>
#include "ui/orderbook_table_model.h"
#include "core/orderbook.h"

namespace ui {

class OrderBookView : public QWidget {
    Q_OBJECT

public:
    explicit OrderBookView(QWidget* parent = nullptr);

public slots:
    void updateOrderBook(const std::vector<core::OrderBookLevel>& bids, 
                        const std::vector<core::OrderBookLevel>& asks);

private:
    void setupUI();

    QTableView* bidTable_;
    QTableView* askTable_;
    ui::OrderBookTableModel* bidModel_;
    ui::OrderBookTableModel* askModel_;
    QLabel* lastUpdateLabel_;
    QLabel* spreadLabel_;
    QVBoxLayout* mainLayout_;
};

} // namespace ui
