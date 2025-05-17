// HARE KRISHNA 

#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <QApplication>

#include "core/logger.h"
#include "core/config.h"
#include "core/orderbook.h"
#include "websocket/websocket_client.h"
#include "websocket/message_processor.h"
#include "models/simulator.h"
#include "ui/main_window.h"

int main(int argc, char* argv[]) {
    try {
        // Initialize Qt Application
        QApplication app(argc, argv);

        // Initialize logger
        core::Logger::getInstance().init();
        auto& logger = core::Logger::getInstance();

        logger.info("Starting Crypto Exchange Trade Simulator... Hare Krishna");

        // Load configuration
        auto config = std::make_shared<core::Config>();
        if (!config->load("/Users/vanshu/Desktop/demo/config.json")) {
            logger.error("Failed to load configuration file");
            return 1;
        }

        // Create and show main window
        ui::MainWindow mainWindow(config);
        mainWindow.show();

        // Start Qt event loop
        return app.exec();
        
    } catch(const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    } catch(...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }   
}
