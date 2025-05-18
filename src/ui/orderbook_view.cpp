// orderbook_view.cpp
#include "ui/orderbook_view.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QDateTime>

namespace ui {

OrderBookView::OrderBookView(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void OrderBookView::setupUI() {
    // Create main layout
    mainLayout_ = new QVBoxLayout(this);

    // Create labels
    lastUpdateLabel_ = new QLabel("Last Update: -");
    spreadLabel_ = new QLabel("Spread: -");
    mainLayout_->addWidget(lastUpdateLabel_);
    mainLayout_->addWidget(spreadLabel_);

    // Create tables
    bidTable_ = new QTableView(this);
    askTable_ = new QTableView(this);

    // Create models
    bidModel_ = new ui::OrderBookTableModel(this);
    askModel_ = new ui::OrderBookTableModel(this);

    // Set models
    bidTable_->setModel(bidModel_);
    askTable_->setModel(askModel_);

    // Configure tables
    bidTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bidTable_->verticalHeader()->setVisible(false);
    bidTable_->setAlternatingRowColors(true);
    bidTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    bidTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    askTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    askTable_->verticalHeader()->setVisible(false);
    askTable_->setAlternatingRowColors(true);
    askTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    askTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Add tables to layout
    mainLayout_->addWidget(bidTable_);
    mainLayout_->addWidget(askTable_);

    setLayout(mainLayout_);
}

void OrderBookView::updateOrderBook(const std::vector<core::OrderBookLevel>& bids,
                                  const std::vector<core::OrderBookLevel>& asks) {
    // Update models
    bidModel_->updateData(bids);
    askModel_->updateData(asks);

    // Update labels
    lastUpdateLabel_->setText(QString("Last Update: %1")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));

    // Calculate and display spread
    if (!bids.empty() && !asks.empty()) {
        double spread = asks.front().price - bids.front().price;
        double spreadPercent = (spread / bids.front().price) * 100.0;
        spreadLabel_->setText(QString("Spread: %1 (%2%)")
            .arg(QString::number(spread, 'f', 2))
            .arg(QString::number(spreadPercent, 'f', 4)));
    }
}

} // namespace ui
