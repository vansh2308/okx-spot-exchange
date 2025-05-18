#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include "models/simulator.h"

namespace ui {

class SimulationPanel : public QWidget {
    Q_OBJECT

public:
    explicit SimulationPanel(QWidget* parent = nullptr);

public slots:
    void updateResults(const models::SimulationResult& result);

private:
    void setupUI();

    QLabel* slippageLabel_;
    QLabel* feesLabel_;
    QLabel* marketImpactLabel_;
    QLabel* netCostLabel_;
    QLabel* makerRatioLabel_;
    QLabel* latencyLabel_;
    QVBoxLayout* mainLayout_;
};

} // namespace ui 