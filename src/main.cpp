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

        logger.info("Starting Crypto Exchange Trade Simulator...");

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

        // Register simulator callback
        simulator->registerResultCallback([&](const models::SimulationResult& result) {
            view.getSimulationPanel()->updateResults(result);
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
                            
                            std::string newSymbol = params.symbol.toLatin1().data();
                            std::string newEndpoint = "wss://ws.gomarket-cpp.goquant.io/ws/l2-orderbook/okx/" + newSymbol.substr(0, newSymbol.find('/')) + "-USDT-SWAP";

                            // wsClient->disconnect();

                            if(config->getWebSocketEndpoint() != newEndpoint){
                                config->setWebSocketEndpoint(newEndpoint);
                                
                                // Reconnect to new exchange
                                wsClient = std::make_shared<websocket::WebSocketClient>(config, msgProcessor);
                                
                                if (!wsClient->connect()) {
                                        logger.error("Failed to connect to new symbol: {}", newSymbol.c_str());
                                        return;
                                    }
                            }

                            // Update simulator configuration
                            simulator->setOrderType(params.orderType.toStdString());
                            simulator->setQuantity(params.quantity);
                            simulator->setVolatility(params.volatility);
                            simulator->setFeeTier(params.feeTier.toStdString());
                        });

        // Initial simulation with default parameters
        auto initialParams = view.getInputPanel()->getParameters();
        simulator->setOrderType(initialParams.orderType.toStdString());
        simulator->setQuantity(initialParams.quantity);
        simulator->setVolatility(initialParams.volatility);
        simulator->setFeeTier(initialParams.feeTier.toStdString());
        
        // Start continuous simulation
        simulator->startContinuousSimulation(orderBook);

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




