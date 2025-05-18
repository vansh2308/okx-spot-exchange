#include "websocket/message_processor.h"
#include "core/logger.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace processing {

MessageProcessor::MessageProcessor() : messageQueue_(100000), logger_(core::Logger::getInstance()) {
    // Queue size of 100k messages
}

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

bool MessageProcessor::enqueue(const WebSocketMessage& message) {
    return messageQueue_.try_enqueue(message);
}

WebSocketMessage MessageProcessor::dequeue() {
    WebSocketMessage msg;
    messageQueue_.try_dequeue(msg);
    return msg;
}

bool MessageProcessor::empty() const {
    return messageQueue_.size_approx() == 0;
}

size_t MessageProcessor::size() const {
    return messageQueue_.size_approx();
}

void MessageProcessor::processMessages() {
    while (running_) {
        WebSocketMessage msg;
        if (messageQueue_.try_dequeue(msg)) {
            // Process message
            logger_.info("Processing message: {}", msg.data);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

} // namespace processing