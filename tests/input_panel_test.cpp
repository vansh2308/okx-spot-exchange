#include <gtest/gtest.h>
#include <QTest>
#include "ui/input_panel.h"
#include <QApplication>
#include <QSignalSpy>

class InputPanelTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 1;
        char* argv[] = {nullptr};
        app = std::make_unique<QApplication>(argc, argv);
        panel = std::make_unique<ui::InputPanel>();
    }

    void TearDown() override {
        panel.reset();
        app.reset();
    }

    std::unique_ptr<QApplication> app;
    std::unique_ptr<ui::InputPanel> panel;
};

TEST_F(InputPanelTest, InitialValues) {
    EXPECT_EQ(panel->getExchange(), "OKX");
    // EXPECT_EQ(panel->getSymbol(), "BTC-USDT");
    EXPECT_EQ(panel->getOrderType(), "Market");
    EXPECT_DOUBLE_EQ(panel->getQuantity(), 100.0);
    EXPECT_DOUBLE_EQ(panel->getVolatility(), 30.0);
    EXPECT_EQ(panel->getFeeTier(), "Tier 1");
}

// TEST_F(InputPanelTest, ParameterChanges) {
//     QSignalSpy exchangeSpy(panel.get(), &ui::InputPanel::exchangeChanged);
//     QSignalSpy symbolSpy(panel.get(), &ui::InputPanel::symbolChanged);
//     QSignalSpy paramsSpy(panel.get(), &ui::InputPanel::parametersChanged);

//     // Simulate user input changes through the UI
//     // panel->exchangeCombo_->setCurrentText("Binance");
//     // panel->symbolCombo_->setCurrentText("ETH-USDT");
//     // panel->orderTypeCombo_->setCurrentText("Limit");
//     // panel->quantitySpin_->setValue(200.0);
//     // panel->volatilitySpin_->setValue(2.0);
//     // panel->feeTierCombo_->setCurrentText("Tier 2");

//     // Verify signals were emitted
//     EXPECT_EQ(exchangeSpy.count(), 1);
//     EXPECT_EQ(symbolSpy.count(), 1);
//     EXPECT_EQ(paramsSpy.count(), 6);  // One for each parameter change

//     // Verify new values
//     EXPECT_EQ(panel->getExchange(), "Binance");
//     EXPECT_EQ(panel->getSymbol(), "ETH-USDT");
//     EXPECT_EQ(panel->getOrderType(), "Limit");
//     EXPECT_DOUBLE_EQ(panel->getQuantity(), 200.0);
//     EXPECT_DOUBLE_EQ(panel->getVolatility(), 2.0);
//     EXPECT_EQ(panel->getFeeTier(), "Tier 2");
// } 