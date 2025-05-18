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

    // Create input panel
    inputPanel_ = new InputPanel(this);
    mainLayout_->addWidget(inputPanel_);

    // Create labels
    lastUpdateLabel_ = new QLabel("Last Update: -");
    spreadLabel_ = new QLabel("Spread: -");
    mainLayout_->addWidget(lastUpdateLabel_);
    mainLayout_->addWidget(spreadLabel_);

    // Create horizontal layout for order book and simulation panel
    auto* horizontalLayout = new QHBoxLayout();

    // Create order book group
    auto* orderBookGroup = new QGroupBox("Order Book", this);
    auto* orderBookLayout = new QVBoxLayout(orderBookGroup);

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

    // Add tables to order book layout
    orderBookLayout->addWidget(bidTable_);
    orderBookLayout->addWidget(askTable_);

    // Create simulation panel
    simulationPanel_ = new SimulationPanel(this);

    // Add widgets to horizontal layout
    horizontalLayout->addWidget(orderBookGroup, 2);  // Order book takes 2/3 of the space
    horizontalLayout->addWidget(simulationPanel_, 1);  // Simulation panel takes 1/3 of the space

    // Add horizontal layout to main layout
    mainLayout_->addLayout(horizontalLayout);

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
