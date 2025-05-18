// orderbook_table_model.h
#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <vector>
#include "core/orderbook.h"

namespace ui {

class OrderBookTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit OrderBookTableModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Custom methods
    void updateData(const std::vector<core::OrderBookLevel>& levels);
    void setIsBids(bool isBids) { isBids_ = isBids; }

private:
    std::vector<core::OrderBookLevel> levels_;
    bool isBids_;
};

} // namespace ui
