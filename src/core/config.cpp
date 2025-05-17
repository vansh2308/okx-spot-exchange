
#include "core/config.h"
#include "core/logger.h"
#include <fstream>
#include <iostream>

namespace core {


bool Config::load(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            core::Logger::getInstance().info("Here");
            return false;
        }
        
        file >> configData_;
        parseExchanges();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool Config::save(const std::string& filePath) const {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        file << configData_.dump(4); // Pretty print with 4 spaces
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

std::string Config::getWebSocketEndpoint() const {
    return configData_["websocket"]["endpoint"].get<std::string>();
}

void Config::setWebSocketEndpoint(const std::string& endpoint) {
    configData_["websocket"]["endpoint"] = endpoint;
}

int Config::getReconnectIntervalMs() const {
    return configData_["websocket"]["reconnect_interval_ms"].get<int>();
}

int Config::getPingIntervalMs() const {
    return configData_["websocket"]["ping_interval_ms"];
}

std::vector<Exchange> Config::getExchanges() const {
    std::vector<Exchange> result;
    for (const auto& [name, exchange] : exchanges_) {
        result.push_back(exchange);
    }
    return result;
}

Exchange Config::getExchange(const std::string& name) const {
    if (exchanges_.find(name) != exchanges_.end()) {
        return exchanges_.at(name);
    }
    
    // Return empty exchange if not found
    Exchange empty;
    empty.name = "UNKNOWN";
    return empty;
}

std::vector<FeeTier> Config::getFeeTiers(const std::string& exchangeName) const {
    if (exchanges_.find(exchangeName) != exchanges_.end()) {
        return exchanges_.at(exchangeName).feeTiers;
    }
    return {};
}

double Config::getMakerFee(const std::string& exchangeName, const std::string& tierName) const {
    if (exchanges_.find(exchangeName) != exchanges_.end()) {
        const auto& feeTiers = exchanges_.at(exchangeName).feeTiers;
        for (const auto& tier : feeTiers) {
            if (tier.name == tierName) {
                return tier.maker;
            }
        }
    }
    return 0.0;
}

double Config::getTakerFee(const std::string& exchangeName, const std::string& tierName) const {
    if (exchanges_.find(exchangeName) != exchanges_.end()) {
        const auto& feeTiers = exchanges_.at(exchangeName).feeTiers;
        for (const auto& tier : feeTiers) {
            if (tier.name == tierName) {
                return tier.taker;
            }
        }
    }
    return 0.0;
}

double Config::getDefaultQuantityUsd() const {
    return configData_["simulator"]["default_quantity_usd"];
}

double Config::getDefaultVolatility() const {
    return configData_["simulator"]["default_volatility"];
}

std::string Config::getDefaultFeeTier() const {
    return configData_["simulator"]["default_fee_tier"];
}

std::string Config::getDefaultExchange() const {
    return configData_["simulator"]["default_exchange"];
}

std::string Config::getDefaultAsset() const {
    return configData_["simulator"]["default_asset"];
}

std::string Config::getDefaultOrderType() const {
    return configData_["simulator"]["default_order_type"];
}

int Config::getUpdateIntervalMs() const {
    return configData_["simulator"]["update_interval_ms"];
}

std::string Config::getLogLevel() const {
    return configData_["logging"]["level"];
}

bool Config::isConsoleOutputEnabled() const {
    return configData_["logging"]["console_output"];
}

bool Config::isFileOutputEnabled() const {
    return configData_["logging"]["file_output"];
}

std::string Config::getLogFilePath() const {
    return configData_["logging"]["file_path"];
}

int Config::getMaxFileSizeMb() const {
    return configData_["logging"]["max_file_size_mb"];
}

int Config::getMaxFiles() const {
    return configData_["logging"]["max_files"];
}

bool Config::isMeasureLatencyEnabled() const {
    return configData_["performance"]["measure_latency"];
}

int Config::getBufferSize() const {
    return configData_["performance"]["buffer_size"];
}

int Config::getProcessingThreads() const {
    return configData_["performance"]["processing_threads"];
}

void Config::parseExchanges() {
    exchanges_.clear();
    
    for (const auto& exchange : configData_["exchanges"]) {
        Exchange ex;
        ex.name = exchange["name"];
        
        // Parse fee tiers
        for (const auto& tier : exchange["fee_tiers"]) {
            FeeTier feeTier;
            feeTier.name = tier["tier"];
            feeTier.maker = tier["maker"];
            feeTier.taker = tier["taker"];
            ex.feeTiers.push_back(feeTier);
        }
        
        // Parse spot assets
        for (const auto& asset : exchange["spot_assets"]) {
            ex.spotAssets.push_back(asset);
        }
        
        exchanges_[ex.name] = ex;
    }
}

} // namespace core 