#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <concurrentqueue/concurrentqueue.h>
#include "core/logger.h"

namespace processing {

struct WebSocketMessage {
    std::string data;
    std::string timestamp;
    // Add other fields as needed (type, etc.)
};

class MessageProcessor {
public:
    MessageProcessor();
    ~MessageProcessor();

    void start();
    void stop();
    bool enqueue(const WebSocketMessage& message);
    WebSocketMessage dequeue();
    bool empty() const;
    size_t size() const;

private:
    void processMessages();
    
    moodycamel::ConcurrentQueue<WebSocketMessage> messageQueue_;
    // std::thread processor_thread_;
    std::atomic<bool> running_{false};
    core::Logger& logger_;
};

} // namespace processing