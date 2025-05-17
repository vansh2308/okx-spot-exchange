#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace core {

struct FeeTier {
    std::string name;
    double maker;
    double taker;
};

struct Exchange {
    std::string name;
    std::vector<FeeTier> feeTiers;
    std::vector<std::string> spotAssets;
};

class Config {
public:
    Config() = default;
    ~Config() = default;

    bool load(const std::string& filePath);
    bool save(const std::string& filePath) const;

    // WebSocket settings
    std::string getWebSocketEndpoint() const;
    void setWebSocketEndpoint(const std::string& endpoint);
    int getReconnectIntervalMs() const;
    int getPingIntervalMs() const;

    // Exchange settings
    std::vector<Exchange> getExchanges() const;
    Exchange getExchange(const std::string& name) const;
    std::vector<FeeTier> getFeeTiers(const std::string& exchangeName) const;
    double getMakerFee(const std::string& exchangeName, const std::string& tierName) const;
    double getTakerFee(const std::string& exchangeName, const std::string& tierName) const;

    // Simulator settings
    double getDefaultQuantityUsd() const;
    double getDefaultVolatility() const;
    std::string getDefaultFeeTier() const;
    std::string getDefaultExchange() const;
    std::string getDefaultAsset() const;
    std::string getDefaultOrderType() const;
    int getUpdateIntervalMs() const;

    // Logging settings
    std::string getLogLevel() const;
    bool isConsoleOutputEnabled() const;
    bool isFileOutputEnabled() const;
    std::string getLogFilePath() const;
    int getMaxFileSizeMb() const;
    int getMaxFiles() const;

    // Performance settings
    bool isMeasureLatencyEnabled() const;
    int getBufferSize() const;
    int getProcessingThreads() const;

private:
    nlohmann::json configData_;
    std::map<std::string, Exchange> exchanges_;

    void parseExchanges();
};

} // namespace core 