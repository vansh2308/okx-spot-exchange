// orderbook_table_model.h
#pragma once

#include <QAbstractTableModel>
#include <QVector>
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

public slots:
    void updateData(const std::vector<core::OrderBookLevel>& newData);

private:
    std::vector<core::OrderBookLevel> data_;
};

} // namespace ui
