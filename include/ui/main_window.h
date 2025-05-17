#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QTimer>
#include <QStatusBar>
#include <QProgressBar>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QMetaType>

#include "core/config.h"
#include "core/orderbook.h"
#include "models/simulator.h"
#include "websocket/websocket_client.h"

// Declare SimulationResult as a meta type
Q_DECLARE_METATYPE(models::SimulationResult)

namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<core::Config> config, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void connectionStatusChanged(bool connected);

private slots:
    void onExchangeChanged(const QString& exchange);
    void onAssetChanged(const QString& asset);
    void onOrderTypeChanged(const QString& orderType);
    void onQuantityChanged(double quantity);
    void onVolatilityChanged(double volatility);
    void onFeeTierChanged(const QString& feeTier);
    void onSimulationResult(const models::SimulationResult& result);
    void onConnectionStatusChanged(bool connected);
    void updatePerformanceMetrics();

private:
    void setupUI();
    void setupInputPanel();
    void setupOutputPanel();
    void setupDiagnosticPanel();
    void initializeSimulator();
    void updateAssetList(const QString& exchange);
    void updateFeeTiers(const QString& exchange);

    // UI Components
    QWidget* centralWidget_;
    QHBoxLayout* mainLayout_;
    
    // Left Panel - Input Parameters
    QGroupBox* inputGroup_;
    QFormLayout* inputLayout_;
    QComboBox* exchangeCombo_;
    QComboBox* assetCombo_;
    QComboBox* orderTypeCombo_;
    QDoubleSpinBox* quantitySpin_;
    QDoubleSpinBox* volatilitySpin_;
    QComboBox* feeTierCombo_;

    // Right Panel - Output Values
    QGroupBox* outputGroup_;
    QFormLayout* outputLayout_;
    QLabel* slippageLabel_;
    QLabel* feesLabel_;
    QLabel* marketImpactLabel_;
    QLabel* netCostLabel_;
    QLabel* makerTakerLabel_;
    QLabel* latencyLabel_;

    // Bottom Panel - Diagnostics
    QGroupBox* diagnosticGroup_;
    QFormLayout* diagnosticLayout_;
    QLabel* wsLatencyLabel_;
    QLabel* processingLatencyLabel_;
    QLabel* uiLatencyLabel_;
    QLabel* fpsLabel_;
    QLabel* throughputLabel_;
    QLabel* connectionStatusLabel_;
    QProgressBar* connectionIndicator_;

    // Performance monitoring
    QTimer* performanceTimer_;
    int frameCount_;
    double lastFpsUpdate_;
    std::chrono::high_resolution_clock::time_point lastFrameTime_;

    // Core components
    std::shared_ptr<core::Config> config_;
    std::shared_ptr<core::OrderBook> orderBook_;
    std::shared_ptr<models::Simulator> simulator_;
    std::shared_ptr<websocket::WebSocketClient> wsClient_;
};

} // namespace ui 