#include "ui/input_panel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

namespace ui {

InputPanel::InputPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    loadExchangeData();
}

void InputPanel::setupUI() {
    mainLayout_ = new QFormLayout(this);
    groupBox_ = new QGroupBox("Simulation Parameters", this);

    // Create form layout for the group box
    auto* formLayout = new QFormLayout(groupBox_);

    // Exchange selection
    exchangeCombo_ = new QComboBox(this);
    exchangeCombo_->addItem("OKX");
    formLayout->addRow("Exchange:", exchangeCombo_);

    // Symbol selection
    symbolCombo_ = new QComboBox(this);
    formLayout->addRow("Symbol:", symbolCombo_);

    // Order type selection
    orderTypeCombo_ = new QComboBox(this);
    orderTypeCombo_->addItem("Market");
    formLayout->addRow("Order Type:", orderTypeCombo_);

    // Quantity input
    quantitySpin_ = new QDoubleSpinBox(this);
    quantitySpin_->setRange(0.0, 1000000.0);
    quantitySpin_->setValue(100.0);
    quantitySpin_->setSuffix(" USD");
    quantitySpin_->setDecimals(2);
    formLayout->addRow("Quantity:", quantitySpin_);

    // Volatility input
    volatilitySpin_ = new QDoubleSpinBox(this);
    volatilitySpin_->setRange(0.0, 100.0);
    volatilitySpin_->setValue(30);
    volatilitySpin_->setSuffix("%");
    volatilitySpin_->setDecimals(2);
    formLayout->addRow("Volatility:", volatilitySpin_);

    // Fee tier selection
    feeTierCombo_ = new QComboBox(this);
    feeTierCombo_->addItems({"Tier 1", "Tier 2", "Tier 3", "Tier 4", "Tier 5"});
    formLayout->addRow("Fee Tier:", feeTierCombo_);

    // Add group box to main layout
    mainLayout_->addWidget(groupBox_);

    // Connect signals
    connect(exchangeCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &InputPanel::onExchangeChanged);
    connect(symbolCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            [this](const QString& symbol) { emit symbolChanged(symbol); emit parametersChanged(getParameters()); });
    connect(orderTypeCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            [this](const QString&) { emit parametersChanged(getParameters()); });
    connect(quantitySpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double) { emit parametersChanged(getParameters()); });
    connect(volatilitySpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double) { emit parametersChanged(getParameters()); });
    connect(feeTierCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            [this](const QString&) { emit parametersChanged(getParameters()); });
}

void InputPanel::loadExchangeData() {
    // Load available symbols from exchange
    // For now, we'll add some common pairs
    symbolCombo_->addItems({
        "BTC/USDT",
        "ETH/USDT",
        "SOL/USDT",
        "XRP/USDT",
        "ADA/USDT",
        "DOGE/USDT",
        "DOT/USDT",
        "LINK/USDT",
        "MATIC/USDT",
        "AVAX/USDT"
    });
}

void InputPanel::onExchangeChanged(const QString& exchange) {
    updateSymbols(exchange);
    emit exchangeChanged(exchange);
    emit parametersChanged(getParameters());
}

void InputPanel::updateSymbols(const QString& exchange) {
    // In a real application, this would fetch available symbols from the exchange API
    // For now, we'll just use the pre-populated list
}

QString InputPanel::getExchange() const {
    return exchangeCombo_->currentText();
}

QString InputPanel::getSymbol() const {
    return symbolCombo_->currentText();
}

QString InputPanel::getOrderType() const {
    return orderTypeCombo_->currentText();
}

double InputPanel::getQuantity() const {
    return quantitySpin_->value();
}

double InputPanel::getVolatility() const {
    return volatilitySpin_->value();
}

QString InputPanel::getFeeTier() const {
    return feeTierCombo_->currentText();
}

InputPanel::Parameters InputPanel::getParameters() const {
    Parameters params;
    params.exchange = getExchange();
    params.symbol = getSymbol();
    params.orderType = getOrderType();
    params.quantity = getQuantity();
    params.volatility = getVolatility();
    params.feeTier = getFeeTier();
    return params;
}

} // namespace ui 