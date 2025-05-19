#include <gtest/gtest.h>
#include <QTest>
#include "websocket/websocket_client.h"
#include "core/config.h"
#include "websocket/message_processor.h"
#include <QApplication>

class WebSocketTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 1;
        char* argv[] = {nullptr};
        app = std::make_unique<QApplication>(argc, argv);
        
        config = std::make_shared<core::Config>();
        config->load("/Users/vanshu/Desktop/demo/config.json");
        msgProcessor = std::make_shared<processing::MessageProcessor>();
        client = std::make_unique<websocket::WebSocketClient>(config, msgProcessor);
    }

    void TearDown() override {
        client.reset();
        msgProcessor.reset();
        config.reset();
        app.reset();
    }

    std::unique_ptr<QApplication> app;
    std::shared_ptr<core::Config> config;
    std::shared_ptr<processing::MessageProcessor> msgProcessor;
    std::unique_ptr<websocket::WebSocketClient> client;
};

TEST_F(WebSocketTest, InitialState) {
    EXPECT_FALSE(client->isConnected());
    EXPECT_TRUE(msgProcessor->empty());
}

TEST_F(WebSocketTest, ConnectDisconnect) {
    client->connect();
    QTest::qWait(1000);
    
    EXPECT_TRUE(client->isConnected());

    client->disconnect();
    QTest::qWait(1000);
    
    EXPECT_FALSE(client->isConnected());
}

TEST_F(WebSocketTest, MessageHandling) {
    client->connect();
    QTest::qWait(1000);
    
    // Wait for messages
    QTest::qWait(1000);
    
    // Check if messages were queued
    EXPECT_FALSE(msgProcessor->empty());
    EXPECT_GT(msgProcessor->size(), 0);
} 