// view.cpp
#include "ui/view.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QDateTime>

namespace ui {

View::View(QWidget* parent)
    : QWidget(parent) {
    setupUI();
}

void View::setupUI() {
    // Create main layout
    mainLayout_ = new QVBoxLayout(this);

    // Create top horizontal layout for input panel and simulation panel
    auto* topLayout = new QHBoxLayout();
    
    // Create input panel
    inputPanel_ = new InputPanel(this);
    topLayout->addWidget(inputPanel_, 1);  // Input panel takes 1/2 of the space

    // Create simulation panel
    simulationPanel_ = new SimulationPanel(this);
    topLayout->addWidget(simulationPanel_, 1);  // Simulation panel takes 1/2 of the space

    // Add top layout to main layout
    mainLayout_->addLayout(topLayout);

    // Create labels
    lastUpdateLabel_ = new QLabel("Last Update: -");
    spreadLabel_ = new QLabel("Spread: -");
    mainLayout_->addWidget(lastUpdateLabel_);
    mainLayout_->addWidget(spreadLabel_);

    // Create order book group
    auto* orderBookGroup = new QGroupBox("Order Book", this);
    auto* orderBookLayout = new QHBoxLayout(orderBookGroup);  // Changed to horizontal layout

    // Create bid and ask groups
    auto* bidGroup = new QGroupBox("Bids", this);
    auto* askGroup = new QGroupBox("Asks", this);
    auto* bidLayout = new QVBoxLayout(bidGroup);
    auto* askLayout = new QVBoxLayout(askGroup);

    // Create tables
    bidTable_ = new QTableView(this);
    askTable_ = new QTableView(this);

    // Create models
    bidModel_ = new ui::OrderBookTableModel(this);
    askModel_ = new ui::OrderBookTableModel(this);

    // Set bid/ask flags
    bidModel_->setIsBids(true);
    askModel_->setIsBids(false);

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

    // Add tables to their respective layouts
    bidLayout->addWidget(bidTable_);
    askLayout->addWidget(askTable_);

    // Add bid and ask groups to order book layout
    orderBookLayout->addWidget(bidGroup, 1);  // Bids take 50% width
    orderBookLayout->addWidget(askGroup, 1);  // Asks take 50% width

    // Add order book group to main layout
    mainLayout_->addWidget(orderBookGroup);

    setLayout(mainLayout_);
}

void View::updateOrderBook(const std::vector<core::OrderBookLevel>& bids,
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
