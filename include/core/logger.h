#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace core {

class Logger {
public:
    static Logger& getInstance();

    void init();
    void setLevel(const std::string& level);
    void setConsoleOutput(bool enabled);
    void setFileOutput(bool enabled, const std::string& filePath, int maxFileSizeMb, int maxFiles);

    // Logging methods
    template<typename... Args>
    void trace(const char* fmt, const Args&... args) {
        if (logger_) {
            logger_->trace(fmt, args...);
        }
    }

    template<typename... Args>
    void debug(const char* fmt, const Args&... args) {
        if (logger_) {
            logger_->debug(fmt, args...);
        }
    }

    template<typename... Args>
    void info(const char* fmt, const Args&... args) {
        if (logger_) {
            logger_->info(fmt, args...);
        }
    }

    template<typename... Args>
    void warn(const char* fmt, const Args&... args) {
        if (logger_) {
            logger_->warn(fmt, args...);
        }
    }

    template<typename... Args>
    void error(const char* fmt, const Args&... args) {
        if (logger_) {
            logger_->error(fmt, args...);
        }
    }

    template<typename... Args>
    void critical(const char* fmt, const Args&... args) {
        if (logger_) {
            logger_->critical(fmt, args...);
        }
    }

private:
    Logger();
    ~Logger();

    // Prevent copy and assignment
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::shared_ptr<spdlog::logger> logger_;
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> consoleSink_;
    std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> fileSink_;

    bool consoleEnabled_;
    bool fileEnabled_;
};

} // namespace core 