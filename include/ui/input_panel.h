#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>

namespace ui {

class InputPanel : public QWidget {
    Q_OBJECT

public:
    struct Parameters {
        QString exchange;
        QString symbol;
        QString orderType;
        double quantity;
        double volatility;
        QString feeTier;
    };

    explicit InputPanel(QWidget* parent = nullptr);

    // Getters for input values
    QString getExchange() const;
    QString getSymbol() const;
    QString getOrderType() const;
    double getQuantity() const;
    double getVolatility() const;
    QString getFeeTier() const;

    // Get all parameters
    Parameters getParameters() const;

signals:
    void exchangeChanged(const QString& exchange);
    void symbolChanged(const QString& symbol);
    void parametersChanged(const Parameters& params);

private slots:
    void onExchangeChanged(const QString& exchange);
    void updateSymbols(const QString& exchange);

private:
    void setupUI();
    void loadExchangeData();

    QComboBox* exchangeCombo_;
    QComboBox* symbolCombo_;
    QComboBox* orderTypeCombo_;
    QDoubleSpinBox* quantitySpin_;
    QDoubleSpinBox* volatilitySpin_;
    QComboBox* feeTierCombo_;

    QFormLayout* mainLayout_;
    QGroupBox* groupBox_;
};

} // namespace ui 