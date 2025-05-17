#include <filesystem>
#include <iostream>

#include "core/logger.h"

namespace core {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : consoleEnabled_(true), fileEnabled_(false) {
    // Private constructor
}

Logger::~Logger() {
    // Destructor
}

void Logger::init() {
    try {
        // Create console sink
        consoleSink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        
        // Create logger with console sink
        std::vector<spdlog::sink_ptr> sinks{consoleSink_};
        logger_ = std::make_shared<spdlog::logger>("crypto_exchange", sinks.begin(), sinks.end());
        
        // Set default log level
        logger_->set_level(spdlog::level::info);
        
        // Set pattern
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
        
        // Register logger
        spdlog::register_logger(logger_);
        
        logger_->info("Logger initialized");
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
    }
}

void Logger::setLevel(const std::string& level) {
    if (!logger_) {
        return;
    }
    
    spdlog::level::level_enum logLevel = spdlog::level::info;
    
    if (level == "trace") {
        logLevel = spdlog::level::trace;
    } else if (level == "debug") {
        logLevel = spdlog::level::debug;
    } else if (level == "info") {
        logLevel = spdlog::level::info;
    } else if (level == "warn") {
        logLevel = spdlog::level::warn;
    } else if (level == "error") {
        logLevel = spdlog::level::err;
    } else if (level == "critical") {
        logLevel = spdlog::level::critical;
    }
    
    logger_->set_level(logLevel);
}

void Logger::setConsoleOutput(bool enabled) {
    consoleEnabled_ = enabled;
    
    if (!logger_) {
        return;
    }
    
    if (enabled && std::find(logger_->sinks().begin(), logger_->sinks().end(), consoleSink_) == logger_->sinks().end()) {
        logger_->sinks().push_back(consoleSink_);
    } else if (!enabled) {
        auto& sinks = logger_->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), consoleSink_), sinks.end());
    }
}

void Logger::setFileOutput(bool enabled, const std::string& filePath, int maxFileSizeMb, int maxFiles) {
    fileEnabled_ = enabled;
    
    if (!logger_) {
        return;
    }
    
    if (enabled) {
        try {
            // Create directory if it doesn't exist
            std::filesystem::path path(filePath);
            std::filesystem::create_directories(path.parent_path());
            
            // Create file sink
            fileSink_ = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                filePath, maxFileSizeMb * 1024 * 1024, maxFiles);
            
            // Add file sink if not already present
            if (std::find(logger_->sinks().begin(), logger_->sinks().end(), fileSink_) == logger_->sinks().end()) {
                logger_->sinks().push_back(fileSink_);
            }
            
            logger_->info("File logging enabled: {}", filePath);
        } catch (const spdlog::spdlog_ex& ex) {
            logger_->error("File sink initialization failed: {}", ex.what());
        }
    } else if (fileSink_) {
        // Remove file sink
        auto& sinks = logger_->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), fileSink_), sinks.end());
        
        logger_->info("File logging disabled");
    }
}

} // namespace core 