// HARE KRISHNA 

#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

#include <QApplication>
#include <QMainWindow>
#include "ui/view.h"
#include "ui/bridge.h"

#include "core/logger.h"
#include "core/config.h"
#include "core/orderbook.h"
#include "websocket/websocket_client.h"
#include "websocket/message_processor.h"
#include "models/simulator.h"

int main(int argc, char* argv[]){
    try {
        QApplication app(argc, argv);

        core::Logger::getInstance().init();
        auto& logger = core::Logger::getInstance();

        logger.info("Starting Crypto Exchange Trade Simulator... Hare Krishna");

        // Load configuration
        auto config = std::make_shared<core::Config>();
        if (!config->load("/Users/vanshu/Desktop/demo/config.json")) {
            logger.error("Failed to load configuration file");
            return 1;
        }

        // Initialize OrderBook and Simulator
        auto orderBook = std::make_shared<core::OrderBook>();
        auto simulator = std::make_shared<models::Simulator>(config);
        simulator->init();
 
        auto msgProcessor = std::make_shared<processing::MessageProcessor>();
        auto wsClient = std::make_shared<websocket::WebSocketClient>(config, msgProcessor);
        bool connected = wsClient->connect();
        if (!connected) {
            logger.error("Failed to connect to WebSocket server");
            return 1;
        }

        ui::View view;
        ui::Bridge bridge(msgProcessor, simulator);
        QObject::connect(&bridge, &ui::Bridge::orderBookUpdated, &view, &ui::View::updateOrderBook);
        QObject::connect(&bridge, &ui::Bridge::simulationUpdated, view.getSimulationPanel(), &ui::SimulationPanel::updateResults);

        // Connect input panel signals
        QObject::connect(view.getInputPanel(), &ui::InputPanel::exchangeChanged,
                        [&](const QString& exchange) {
                            // Update WebSocket connection
                            logger.info("Exchange changed to: {}", exchange.toLatin1().data());
                            
                            // Disconnect from current exchange
                            wsClient->disconnect();
                            
                            // Update WebSocket endpoint for new exchange
                            std::string endpoint = config->getWebSocketEndpoint();
                            config->setWebSocketEndpoint(endpoint);
                            
                            // Reconnect to new exchange
                            if (!wsClient->connect()) {
                                logger.error("Failed to connect to new exchange: {}", exchange.toLatin1().data());
                                return;
                            }
                            
                            // Send subscription message for current symbol
                            QString currentSymbol = view.getInputPanel()->getSymbol();
                            nlohmann::json subMsg = {
                                {"op", "subscribe"},
                                {"args", {currentSymbol.toStdString()}}
                            };
                            wsClient->send(subMsg.dump());
                        });
                        
        QObject::connect(view.getInputPanel(), &ui::InputPanel::symbolChanged,
                        [&](const QString& symbol) {
                            // Update WebSocket subscription
                            logger.info("Symbol changed to: {}", symbol.toLatin1().data());
                            
                            // Unsubscribe from current symbol
                            QString currentSymbol = view.getInputPanel()->getSymbol();
                            nlohmann::json unsubMsg = {
                                {"op", "unsubscribe"},
                                {"args", {currentSymbol.toStdString()}}
                            };
                            wsClient->send(unsubMsg.dump());
                            
                            // Subscribe to new symbol
                            nlohmann::json subMsg = {
                                {"op", "subscribe"},
                                {"args", {symbol.toStdString()}}
                            };
                            wsClient->send(subMsg.dump());
                            
                            // Reset order book for new symbol
                            orderBook = std::make_shared<core::OrderBook>();
                        });
                        
        QObject::connect(view.getInputPanel(), &ui::InputPanel::parametersChanged,
                        [&](const ui::InputPanel::Parameters& params) {
                            // Update simulator parameters
                            logger.info("Parameters updated - Symbol: {}, Exchange: {}, Order Type: {}, Quantity: {:.2f}, Volatility: {:.2f}%, Fee Tier: {}", 
                                      params.symbol.toLatin1().data(),
                                      params.exchange.toLatin1().data(),
                                      params.orderType.toLatin1().data(),
                                      params.quantity,
                                      params.volatility,
                                      params.feeTier.toLatin1().data());
                            
                            // Update simulator configuration
                            simulator->setOrderType(params.orderType.toStdString());
                            simulator->setQuantity(params.quantity);
                            simulator->setVolatility(params.volatility);
                            simulator->setFeeTier(params.feeTier.toStdString());
                            
                            // Run simulation with new parameters
                            auto result = simulator->simulate(orderBook);
                            view.getSimulationPanel()->updateResults(result);
                        });

        view.resize(800, 600);
        view.show();

        bridge.start();
        return app.exec();
        
    } catch(const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    } catch(...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }   
}
