// orderbook_view.h
#pragma once

#include <QWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QLabel>
#include "ui/orderbook_table_model.h"
#include "ui/simulation_panel.h"
#include "core/orderbook.h"

namespace ui {

class View : public QWidget {
    Q_OBJECT

public:
    explicit View(QWidget* parent = nullptr);
    SimulationPanel* getSimulationPanel() const { return simulationPanel_; }

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
    SimulationPanel* simulationPanel_;
    QVBoxLayout* mainLayout_;
};

} // namespace ui
