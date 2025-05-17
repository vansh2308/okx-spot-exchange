// HARE KRISHNA 

#include <memory>
#include <iostream>

#include "core/logger.h"
#include "core/config.h"
#include "websocket/websocket_client.h"

int main(int argc, char* argv[]){
    try {

        // WIP: Get rid of logger 
        core::Logger::getInstance().init();
        auto& logger = core::Logger::getInstance();

        logger.info("Starting Crypto Exchange Trade Simulator... Hare Krishna");

        // Load configuration
        auto config = std::make_shared<core::Config>();
        if (!config->load("/Users/vanshu/Desktop/demo/config.json")) {
            logger.error("Failed to load configuration file");
            return 1;
        }

        // WIP: Make websocket 
        auto wsClient = std::make_shared<websocket::WebSocketClient>(config);
        bool connected = wsClient->connect();
        if (!connected) {
            logger.error("Failed to connect to WebSocket server");
        }
        
        
    } catch(const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    } catch(...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }   
}
