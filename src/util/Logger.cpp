#include "../../include/util/Logger.h"
#include "../../include/system/SystemException.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : initialized_(false), logger_(nullptr) {
}

Logger::~Logger() {
    if (initialized_ && logger_) {
        logger_->flush();
    }
}

bool Logger::initialize(const std::string& logDir, LogLevel logLevel) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true; // 已经初始化过了，直接返回成功
    }
    
    try {
        // 确保日志目录存在
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        
        // 创建控制台输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        
        // 创建文件输出
        auto info_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logDir + "/info.log", 10 * 1024 * 1024, 5, false);
        
        auto warn_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logDir + "/warn.log", 10 * 1024 * 1024, 5, false);
        
        auto error_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logDir + "/error.log", 10 * 1024 * 1024, 5, false);
        
        // 设置过滤级别
        console_sink->set_level(spdlog::level::trace);
        info_file_sink->set_level(spdlog::level::info);
        warn_file_sink->set_level(spdlog::level::warn);
        error_file_sink->set_level(spdlog::level::err);
        
        // 创建多输出日志记录器
        std::vector<spdlog::sink_ptr> sinks {console_sink, info_file_sink, warn_file_sink, error_file_sink};
        logger_ = std::make_shared<spdlog::logger>("course_system", sinks.begin(), sinks.end());
        
        // 设置日志级别
        setLogLevel(logLevel);
        
        // 设置日志格式
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        
        // 注册为默认记录器
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);
        
        initialized_ = true;
        logger_->info("日志系统初始化成功，日志目录：{}", logDir);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "初始化日志系统失败：" << e.what() << std::endl;
        return false;
    }
}

void Logger::debug(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !logger_) {
        std::cerr << "[DEBUG] " << message << std::endl;
        return;
    }
    logger_->debug(message);
}

void Logger::info(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !logger_) {
        std::cerr << "[INFO] " << message << std::endl;
        return;
    }
    logger_->info(message);
}

void Logger::warning(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !logger_) {
        std::cerr << "[WARNING] " << message << std::endl;
        return;
    }
    logger_->warn(message);
}

void Logger::error(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !logger_) {
        std::cerr << "[ERROR] " << message << std::endl;
        return;
    }
    logger_->error(message);
}

void Logger::critical(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !logger_) {
        std::cerr << "[CRITICAL] " << message << std::endl;
        return;
    }
    logger_->critical(message);
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !logger_) {
        return;
    }
    
    switch (level) {
        case LogLevel::DEBUG:
            logger_->set_level(spdlog::level::debug);
            break;
        case LogLevel::INFO:
            logger_->set_level(spdlog::level::info);
            break;
        case LogLevel::WARNING:
            logger_->set_level(spdlog::level::warn);
            break;
        case LogLevel::ERROR:
            logger_->set_level(spdlog::level::err);
            break;
        case LogLevel::CRITICAL:
            logger_->set_level(spdlog::level::critical);
            break;
        default:
            logger_->set_level(spdlog::level::info);
            break;
    }
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

LogLevel Logger::stringToLogLevel(const std::string& levelStr) {
    if (levelStr == "DEBUG") {
        return LogLevel::DEBUG;
    } else if (levelStr == "INFO") {
        return LogLevel::INFO;
    } else if (levelStr == "WARNING") {
        return LogLevel::WARNING;
    } else if (levelStr == "ERROR") {
        return LogLevel::ERROR;
    } else if (levelStr == "CRITICAL") {
        return LogLevel::CRITICAL;
    } else {
        return LogLevel::INFO; // 默认为INFO级别
    }
}
