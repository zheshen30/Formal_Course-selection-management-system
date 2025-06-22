/*
 * Copyright (C) 2025 哲神
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "../../include/util/Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

// 获取当前日期时间的字符串表示
std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : initialized_(false), logLevel_(LogLevel::INFO) {
    // 默认构造，什么都不做
    std::cout << "Logger实例创建" << std::endl;
}

Logger::~Logger() {
    // 确保所有文件都关闭
    if (infoFile_.is_open()) infoFile_.close();
    if (warnFile_.is_open()) warnFile_.close();
    if (errorFile_.is_open()) errorFile_.close();
    std::cout << "Logger实例销毁" << std::endl;
}

bool Logger::initialize(const std::string& logDir, LogLevel logLevel) {
    try {
        std::cout << "开始初始化Logger..." << std::endl;
        
        // 防止重复初始化
        if (initialized_) {
            std::cout << "Logger已经初始化，直接返回" << std::endl;
            return true;
        }
        
        // 保存日志级别
        logLevel_ = logLevel;
        
        // 确保日志目录存在
        if (!fs::exists(logDir)) {
            std::cout << "创建日志目录: " << logDir << std::endl;
            fs::create_directories(logDir);
        }
        
        // 构建日志文件路径
        std::string infoPath = logDir + "/simple_info.log";
        std::string warnPath = logDir + "/simple_warn.log";
        std::string errorPath = logDir + "/simple_error.log";
        
        std::cout << "打开日志文件:\n" << 
            "  Info: " << infoPath << "\n" <<
            "  Warn: " << warnPath << "\n" <<
            "  Error: " << errorPath << std::endl;
        
        // 打开日志文件，使用追加模式
        infoFile_.open(infoPath, std::ios::app);
        warnFile_.open(warnPath, std::ios::app);
        errorFile_.open(errorPath, std::ios::app);
        
        // 检查文件是否成功打开
        if (!infoFile_.is_open() || !warnFile_.is_open() || !errorFile_.is_open()) {
            std::cerr << "无法打开一个或多个日志文件" << std::endl;
            return false;
        }
        
        // 初始化成功
        initialized_ = true;
        
        // 写入初始化成功的日志
        std::string initMsg = "日志系统初始化成功，日志目录：" + logDir;
        info(initMsg);
        
        std::cout << "Logger初始化完成" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "初始化日志系统失败: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "初始化日志系统失败: 未知错误" << std::endl;
        return false;
    }
}

void Logger::debug(const std::string& message) {
    if (logLevel_ > LogLevel::DEBUG) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [DEBUG] " + message;
    
    // 简单模式下，调试消息只输出到控制台
    std::cout << logMsg << std::endl;
    
    // 不需要锁，因为只输出到控制台
}

void Logger::info(const std::string& message) {
    if (logLevel_ > LogLevel::INFO) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [INFO] " + message;
    
    // 输出到控制台
    std::cout << logMsg << std::endl;
    
    // 写入到日志文件
    try {
        if (initialized_ && infoFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);  // 只在文件写入时加锁
            infoFile_ << logMsg << std::endl;
        }
    } catch (...) {
        // 忽略写入错误
    }
}

void Logger::warning(const std::string& message) {
    if (logLevel_ > LogLevel::WARNING) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [WARNING] " + message;
    
    // 输出到控制台
    std::cerr << logMsg << std::endl;
    
    // 写入到日志文件
    try {
        if (initialized_ && warnFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);  // 只在文件写入时加锁
            warnFile_ << logMsg << std::endl;
            // 同时写入信息日志文件
            if (infoFile_.is_open()) {
                infoFile_ << logMsg << std::endl;
            }
        }
    } catch (...) {
        // 忽略写入错误
    }
}

void Logger::error(const std::string& message) {
    if (logLevel_ > LogLevel::ERROR) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [ERROR] " + message;
    
    // 输出到控制台
    std::cerr << logMsg << std::endl;
    
    // 写入到日志文件
    try {
        if (initialized_ && errorFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);  // 只在文件写入时加锁
            errorFile_ << logMsg << std::endl;
            // 同时写入信息和警告日志文件
            if (warnFile_.is_open()) {
                warnFile_ << logMsg << std::endl;
            }
            if (infoFile_.is_open()) {
                infoFile_ << logMsg << std::endl;
            }
        }
    } catch (...) {
        // 忽略写入错误
    }
}

void Logger::critical(const std::string& message) {
    if (logLevel_ > LogLevel::CRITICAL) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [CRITICAL] " + message;
    
    // 输出到控制台
    std::cerr << logMsg << std::endl;
    
    // 写入到日志文件
    try {
        if (initialized_ && errorFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);  // 只在文件写入时加锁
            errorFile_ << logMsg << std::endl;
            // 同时写入信息和警告日志文件
            if (warnFile_.is_open()) {
                warnFile_ << logMsg << std::endl;
            }
            if (infoFile_.is_open()) {
                infoFile_ << logMsg << std::endl;
            }
        }
    } catch (...) {
        // 忽略写入错误
    }
}

void Logger::setLogLevel(LogLevel level) {
    logLevel_ = level;
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

LogLevel Logger::stringToLogLevel(const std::string& levelStr) {
    if (levelStr == "DEBUG") return LogLevel::DEBUG;
    else if (levelStr == "INFO") return LogLevel::INFO;
    else if (levelStr == "WARNING") return LogLevel::WARNING;
    else if (levelStr == "ERROR") return LogLevel::ERROR;
    else if (levelStr == "CRITICAL") return LogLevel::CRITICAL;
    else return LogLevel::INFO; // 默认为INFO级别
}
