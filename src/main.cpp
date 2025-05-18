// HARE KRISHNA 

#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

#include <QApplication>
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

        // while (wsClient->isConnected()) {
        //     processing::WebSocketMessage message = msgProcessor->dequeue();
        //     if (!message.data.empty()) {
        //         try {
        //             // Parse JSON message
        //             auto json = nlohmann::json::parse(message.data);
                    
        //             // Extract orderbook data
        //             std::vector<std::pair<std::string, std::string>> bids, asks;
                    
        //             // Parse bids
        //             for (const auto& bid : json["bids"]) {
        //                 bids.emplace_back(bid[0].get<std::string>(), bid[1].get<std::string>());
        //             }
                    
        //             // Parse asks
        //             for (const auto& ask : json["asks"]) {
        //                 asks.emplace_back(ask[0].get<std::string>(), ask[1].get<std::string>());
        //             }
                    
        //             // Update orderbook
        //             orderBook->update(json["exchange"], json["symbol"], bids, asks, json["timestamp"].get<std::string>());
                    
        //             // Run simulation
        //             auto result = simulator->simulate(orderBook);
                    
        //             // Log results
        //             logger.info("Simulation Results:");
        //             logger.info("  Expected Slippage: {:.4f}%", result.expectedSlippage);
        //             logger.info("  Expected Fees: ${:.4f}", result.expectedFees);
        //             logger.info("  Expected Market Impact: {:.4f}%", result.expectedMarketImpact);
        //             logger.info("  Net Cost: ${:.4f}", result.netCost);
        //             logger.info("  Maker Ratio: {:.4f}", result.makerRatio);
        //             logger.info("  Internal Latency: {:.2f}µs", result.internalLatency);
                    
        //         } catch (const std::exception& e) {
        //             logger.error("Error processing message: {}", e.what());
        //         }
        //     }
        //     std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Small sleep to prevent busy-waiting
        // }

        ui::View view;
        ui::Bridge bridge(msgProcessor, simulator);
        QObject::connect(&bridge, &ui::Bridge::orderBookUpdated, &view, &ui::View::updateOrderBook);
        QObject::connect(&bridge, &ui::Bridge::simulationUpdated, view.getSimulationPanel(), &ui::SimulationPanel::updateResults);

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
