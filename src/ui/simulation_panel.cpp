#include "ui/simulation_panel.h"
#include <QFont>

namespace ui {

SimulationPanel::SimulationPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void SimulationPanel::setupUI() {
    mainLayout_ = new QVBoxLayout(this);

    // Create a group box for simulation results
    auto* groupBox = new QGroupBox("Simulation Results", this);
    auto* groupLayout = new QVBoxLayout(groupBox);

    // Create labels with monospace font for better number alignment
    QFont monoFont("Monospace");
    monoFont.setStyleHint(QFont::Monospace);

    slippageLabel_ = new QLabel("Expected Slippage: -", this);
    feesLabel_ = new QLabel("Expected Fees: -", this);
    marketImpactLabel_ = new QLabel("Expected Market Impact: -", this);
    netCostLabel_ = new QLabel("Net Cost: -", this);
    makerRatioLabel_ = new QLabel("Maker Ratio: -", this);
    latencyLabel_ = new QLabel("Internal Latency: -", this);

    // Set monospace font for all labels
    slippageLabel_->setFont(monoFont);
    feesLabel_->setFont(monoFont);
    marketImpactLabel_->setFont(monoFont);
    netCostLabel_->setFont(monoFont);
    makerRatioLabel_->setFont(monoFont);
    latencyLabel_->setFont(monoFont);

    // Add labels to group layout
    groupLayout->addWidget(slippageLabel_);
    groupLayout->addWidget(feesLabel_);
    groupLayout->addWidget(marketImpactLabel_);
    groupLayout->addWidget(netCostLabel_);
    groupLayout->addWidget(makerRatioLabel_);
    groupLayout->addWidget(latencyLabel_);

    // Add group box to main layout
    mainLayout_->addWidget(groupBox);
    setLayout(mainLayout_);
}

void SimulationPanel::updateResults(const models::SimulationResult& result) {
    slippageLabel_->setText(QString("Expected Slippage: %1%")
        .arg(QString::number(result.expectedSlippage, 'f', 4)));
    
    feesLabel_->setText(QString("Expected Fees: $%1")
        .arg(QString::number(result.expectedFees, 'f', 4)));
    
    marketImpactLabel_->setText(QString("Expected Market Impact: %1%")
        .arg(QString::number(result.expectedMarketImpact, 'f', 4)));
    
    netCostLabel_->setText(QString("Net Cost: $%1")
        .arg(QString::number(result.netCost, 'f', 4)));
    
    makerRatioLabel_->setText(QString("Maker Ratio: %1")
        .arg(QString::number(result.makerRatio, 'f', 4)));
    
    latencyLabel_->setText(QString("Internal Latency: %1 µs")
        .arg(QString::number(result.internalLatency, 'f', 2)));
}

} // namespace ui 