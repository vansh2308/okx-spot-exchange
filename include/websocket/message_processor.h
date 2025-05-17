#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <concurrentqueue.h>

namespace processing {

struct WebSocketMessage {
    std::string data;
    // Add other fields as needed (timestamp, type, etc.)
};

class MessageProcessor {
public:
    MessageProcessor();
    ~MessageProcessor();

    void start();
    void stop();
    bool enqueue(const std::string& message);
    WebSocketMessage dequeue();

private:
    void processMessages();
    
    moodycamel::ConcurrentQueue<WebSocketMessage> queue_;
    // std::thread processor_thread_;
    std::atomic<bool> running_{false};
};

} // namespace processing