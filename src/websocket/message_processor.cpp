#include "websocket/message_processor.h"
#include "core/logger.h"
#include <iostream>

namespace processing {

MessageProcessor::MessageProcessor() : queue_(100000) {} // Queue size of 100k messages

MessageProcessor::~MessageProcessor() {
    stop();
}

void MessageProcessor::start() {
    running_ = true;
    // processor_thread_ = std::thread(&MessageProcessor::processMessages, this);
}

void MessageProcessor::stop() {
    running_ = false;
    // if (processor_thread_.joinable()) {
    //     processor_thread_.join();
    // }
}

bool MessageProcessor::enqueue(const std::string& message) {
    WebSocketMessage msg{message};
    return queue_.try_enqueue(msg);
}

WebSocketMessage MessageProcessor::dequeue() {
    WebSocketMessage msg;
    queue_.try_dequeue(msg);
    return msg;
}   

void MessageProcessor::processMessages() {
    WebSocketMessage msg;
    while (running_) {
        if (queue_.try_dequeue(msg)) {
            // Process the message here
            core::Logger::getInstance().info("Processing message: {}", msg.data);
            // Add your message processing logic here
            // Example: Parse JSON, update market data, etc.
        }

        // Optional: Small sleep to prevent busy-waiting
        // std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

} // namespace processing