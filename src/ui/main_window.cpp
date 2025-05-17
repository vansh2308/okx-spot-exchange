#include "ui/main_window.h"
#include "core/logger.h"
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QDateTime>
#include <QThread>

namespace ui {

MainWindow::MainWindow(std::shared_ptr<core::Config> config, QWidget *parent)
    : QMainWindow(parent)
    , config_(config)
    , frameCount_(0)
    , lastFpsUpdate_(0.0)
{
    // Register SimulationResult type with Qt's meta-object system
    qRegisterMetaType<models::SimulationResult>("models::SimulationResult");

    setWindowTitle("Crypto Exchange Trade Simulator");
    resize(1200, 800);

    // Initialize core components
    orderBook_ = std::make_shared<core::OrderBook>();
    simulator_ = std::make_shared<models::Simulator>(config_);
    simulator_->init();

    // Setup UI first
    setupUI();

    // Initialize performance monitoring
    performanceTimer_ = new QTimer(this);
    connect(performanceTimer_, &QTimer::timeout, this, &MainWindow::updatePerformanceMetrics);
    performanceTimer_->start(1000); // Update every second
    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    // Register simulation result callback
    simulator_->registerResultCallback([this](const models::SimulationResult& result) {
        QMetaObject::invokeMethod(this, "onSimulationResult", Qt::QueuedConnection,
            Q_ARG(models::SimulationResult, result));
    });

    // Initialize WebSocket in a separate thread
    QThread* wsThread = new QThread(this);
    connect(wsThread, &QThread::started, this, &MainWindow::initializeSimulator);
    wsThread->start();
}

MainWindow::~MainWindow() {
    if (wsClient_) {
        wsClient_->disconnect();
    }
    simulator_->unregisterResultCallback();
}

void MainWindow::setupUI() {
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);

    mainLayout_ = new QHBoxLayout(centralWidget_);
    
    // Create panels
    setupInputPanel();
    setupOutputPanel();
    setupDiagnosticPanel();

    // Add panels to main layout
    mainLayout_->addWidget(inputGroup_, 1);
    mainLayout_->addWidget(outputGroup_, 1);
    mainLayout_->addWidget(diagnosticGroup_, 1);
}

void MainWindow::setupInputPanel() {
    inputGroup_ = new QGroupBox("Input Parameters", this);
    inputLayout_ = new QFormLayout(inputGroup_);

    // Exchange selection
    exchangeCombo_ = new QComboBox(this);
    for (const auto& exchange : config_->getExchanges()) {
        exchangeCombo_->addItem(QString::fromStdString(exchange.name));
    }
    connect(exchangeCombo_, &QComboBox::currentTextChanged, this, &MainWindow::onExchangeChanged);
    inputLayout_->addRow("Exchange:", exchangeCombo_);

    // Asset selection
    assetCombo_ = new QComboBox(this);
    connect(assetCombo_, &QComboBox::currentTextChanged, this, &MainWindow::onAssetChanged);
    inputLayout_->addRow("Asset:", assetCombo_);

    // Order type
    orderTypeCombo_ = new QComboBox(this);
    orderTypeCombo_->addItems({"market", "limit"});
    connect(orderTypeCombo_, &QComboBox::currentTextChanged, this, &MainWindow::onOrderTypeChanged);
    inputLayout_->addRow("Order Type:", orderTypeCombo_);

    // Quantity
    quantitySpin_ = new QDoubleSpinBox(this);
    quantitySpin_->setRange(0.0, 1000000.0);
    quantitySpin_->setValue(config_->getDefaultQuantityUsd());
    quantitySpin_->setSuffix(" USD");
    connect(quantitySpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onQuantityChanged);
    inputLayout_->addRow("Quantity:", quantitySpin_);

    // Volatility
    volatilitySpin_ = new QDoubleSpinBox(this);
    volatilitySpin_->setRange(0.0, 1.0);
    volatilitySpin_->setValue(config_->getDefaultVolatility());
    volatilitySpin_->setSingleStep(0.01);
    connect(volatilitySpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onVolatilityChanged);
    inputLayout_->addRow("Volatility:", volatilitySpin_);

    // Fee tier
    feeTierCombo_ = new QComboBox(this);
    connect(feeTierCombo_, &QComboBox::currentTextChanged, this, &MainWindow::onFeeTierChanged);
    inputLayout_->addRow("Fee Tier:", feeTierCombo_);

    // Initialize with default exchange
    onExchangeChanged(QString::fromStdString(config_->getDefaultExchange()));
}

void MainWindow::setupOutputPanel() {
    outputGroup_ = new QGroupBox("Simulation Results", this);
    outputLayout_ = new QFormLayout(outputGroup_);

    // Create labels for each metric
    slippageLabel_ = new QLabel("0.00%", this);
    feesLabel_ = new QLabel("$0.00", this);
    marketImpactLabel_ = new QLabel("0.00%", this);
    netCostLabel_ = new QLabel("$0.00", this);
    makerTakerLabel_ = new QLabel("0.00%", this);
    latencyLabel_ = new QLabel("0.00 µs", this);

    // Add to layout
    outputLayout_->addRow("Expected Slippage:", slippageLabel_);
    outputLayout_->addRow("Expected Fees:", feesLabel_);
    outputLayout_->addRow("Expected Market Impact:", marketImpactLabel_);
    outputLayout_->addRow("Net Cost:", netCostLabel_);
    outputLayout_->addRow("Maker/Taker Ratio:", makerTakerLabel_);
    outputLayout_->addRow("Internal Latency:", latencyLabel_);
}

void MainWindow::setupDiagnosticPanel() {
    diagnosticGroup_ = new QGroupBox("Diagnostics & Performance", this);
    diagnosticLayout_ = new QFormLayout(diagnosticGroup_);

    // Create labels for metrics
    wsLatencyLabel_ = new QLabel("0.00 ms", this);
    processingLatencyLabel_ = new QLabel("0.00 ms", this);
    uiLatencyLabel_ = new QLabel("0.00 ms", this);
    fpsLabel_ = new QLabel("0 FPS", this);
    throughputLabel_ = new QLabel("0 msgs/s", this);
    connectionStatusLabel_ = new QLabel("Disconnected", this);
    connectionIndicator_ = new QProgressBar(this);
    connectionIndicator_->setRange(0, 2);
    connectionIndicator_->setValue(0);
    connectionIndicator_->setTextVisible(false);

    // Add to layout
    diagnosticLayout_->addRow("WebSocket Latency:", wsLatencyLabel_);
    diagnosticLayout_->addRow("Processing Latency:", processingLatencyLabel_);
    diagnosticLayout_->addRow("UI Update Latency:", uiLatencyLabel_);
    diagnosticLayout_->addRow("FPS:", fpsLabel_);
    diagnosticLayout_->addRow("Throughput:", throughputLabel_);
    diagnosticLayout_->addRow("Connection:", connectionStatusLabel_);
    diagnosticLayout_->addRow("", connectionIndicator_);
}

void MainWindow::onExchangeChanged(const QString& exchange) {
    updateAssetList(exchange);
    updateFeeTiers(exchange);
    simulator_->setExchange(exchange.toStdString());
}

void MainWindow::onAssetChanged(const QString& asset) {
    simulator_->setAsset(asset.toStdString());
}

void MainWindow::onOrderTypeChanged(const QString& orderType) {
    simulator_->setOrderType(orderType.toStdString());
}

void MainWindow::onQuantityChanged(double quantity) {
    simulator_->setQuantity(quantity);
}

void MainWindow::onVolatilityChanged(double volatility) {
    simulator_->setVolatility(volatility);
}

void MainWindow::onFeeTierChanged(const QString& feeTier) {
    simulator_->setFeeTier(feeTier.toStdString());
}

void MainWindow::onSimulationResult(const models::SimulationResult& result) {
    // Update output labels
    slippageLabel_->setText(QString::asprintf("%.2f%%", result.expectedSlippage));
    feesLabel_->setText(QString::asprintf("$%.2f", result.expectedFees));
    marketImpactLabel_->setText(QString::asprintf("%.2f%%", result.expectedMarketImpact));
    netCostLabel_->setText(QString::asprintf("$%.2f", result.netCost));
    makerTakerLabel_->setText(QString::asprintf("%.2f%%", result.makerRatio * 100.0));
    latencyLabel_->setText(QString::asprintf("%.2f µs", result.internalLatency));

    // Update frame count for FPS calculation
    frameCount_++;
}

void MainWindow::onConnectionStatusChanged(bool connected) {
    connectionStatusLabel_->setText(connected ? "Connected" : "Disconnected");
    connectionIndicator_->setValue(connected ? 2 : 0);
}

void MainWindow::updatePerformanceMetrics() {
    // Calculate FPS
    double currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    double fps = frameCount_ / (currentTime - lastFpsUpdate_);
    fpsLabel_->setText(QString::asprintf("%.1f FPS", fps));
    frameCount_ = 0;
    lastFpsUpdate_ = currentTime;

    // Update latencies
    auto now = std::chrono::high_resolution_clock::now();
    double uiLatency = std::chrono::duration_cast<std::chrono::microseconds>(
        now - lastFrameTime_).count() / 1000.0;
    uiLatencyLabel_->setText(QString::asprintf("%.2f ms", uiLatency));
    lastFrameTime_ = now;
}

void MainWindow::updateAssetList(const QString& exchange) {
    assetCombo_->clear();
    auto ex = config_->getExchange(exchange.toStdString());
    for (const auto& asset : ex.spotAssets) {
        assetCombo_->addItem(QString::fromStdString(asset));
    }
}

void MainWindow::updateFeeTiers(const QString& exchange) {
    feeTierCombo_->clear();
    auto tiers = config_->getFeeTiers(exchange.toStdString());
    for (const auto& tier : tiers) {
        feeTierCombo_->addItem(QString::fromStdString(tier.name));
    }
}

void MainWindow::initializeSimulator() {
    // Initialize WebSocket client
    auto msgProcessor = std::make_shared<processing::MessageProcessor>();
    wsClient_ = std::make_shared<websocket::WebSocketClient>(config_, msgProcessor);

    // Connect WebSocket signals
    connect(wsClient_.get(), &websocket::WebSocketClient::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged, Qt::QueuedConnection);

    // Start WebSocket connection
    bool connected = wsClient_->connect();
    if (!connected) {
        core::Logger::getInstance().error("Failed to connect to WebSocket server");
        emit connectionStatusChanged(false);
    }
}

} // namespace ui 