// orderbook_table_model.cpp
#include "ui/orderbook_table_model.h"
#include <QColor>
#include <QBrush>

namespace ui {

OrderBookTableModel::OrderBookTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int OrderBookTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return data_.size();
}

int OrderBookTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 2; // Price and Size
}

QVariant OrderBookTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= data_.size()) {
        return QVariant();
    }

    const auto& level = data_[index.row()];

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return QString::number(level.price, 'f', 2);
        } else if (index.column() == 1) {
            return QString::number(level.quantity, 'f', 8);
        }
    } else if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if (role == Qt::BackgroundRole) {
        return QBrush(index.row() % 2 == 0 ? QColor(240, 240, 240) : QColor(255, 255, 255));
    }

    return QVariant();
}

QVariant OrderBookTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return section == 0 ? "Price" : "Size";
    }

    return QVariant();
}

void OrderBookTableModel::updateData(const std::vector<core::OrderBookLevel>& newData) {
    beginResetModel();
    data_ = newData;
    endResetModel();
}

} // namespace ui
