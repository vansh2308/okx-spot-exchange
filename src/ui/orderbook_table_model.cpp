// orderbook_table_model.cpp
#include "ui/orderbook_table_model.h"
#include <QColor>
#include <QBrush>
#include <QString>

namespace ui {

OrderBookTableModel::OrderBookTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , isBids_(false)
{
}

int OrderBookTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return levels_.size();
}

int OrderBookTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return 2;  // Price and Size
}

QVariant OrderBookTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= levels_.size()) {
        return QVariant();
    }

    const auto& level = levels_[index.row()];

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return QString::number(level.price, 'f', 2);
        } else if (index.column() == 1) {
            return QString::number(level.quantity, 'f', 8);
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 0) {  // Only color the price column
            return isBids_ ? QColor(0, 180, 0) : QColor(180, 0, 0);  // Green for bids, Red for asks
        }
    } else if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if (role == Qt::BackgroundRole) {
        return QBrush(index.row() % 2 == 0 ? QColor(0, 0, 0) : QColor(0, 0, 0));
    }

    return QVariant();
}

QVariant OrderBookTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "Price";
            case 1: return "Size";
            default: return QVariant();
        }
    }

    return QVariant();
}

void OrderBookTableModel::updateData(const std::vector<core::OrderBookLevel>& levels) {
    beginResetModel();
    levels_ = levels;
    endResetModel();
}

} // namespace ui
