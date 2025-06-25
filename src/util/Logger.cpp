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

// 获取当前日期时间的字符串表示（北京时间，UTC+8）
std::string getCurrentTimeString() {
    // 获取当前系统时间点
    auto now = std::chrono::system_clock::now();
    
    // 转换为time_t（秒级时间戳）
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    
    // 转换为UTC结构化时间
    std::tm* gmt_tm = std::gmtime(&now_time_t);
    if (!gmt_tm) {
        return "ERROR_TIME";
    }
    
    // 实施北京时间转换 (UTC+8)
    // 增加8小时
    gmt_tm->tm_hour += 8;
    
    // 处理时间进位
    std::mktime(gmt_tm);
    
    // 获取毫秒部分
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // 格式化时间字符串
    std::stringstream ss;
    ss << std::put_time(gmt_tm, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : initialized_(false), logLevel_(LogLevel::INFO) {
    // 默认构造，什么都不做
}

Logger::~Logger() {
    // 确保所有文件都关闭
    if (infoFile_.is_open()) infoFile_.close();
    if (warnFile_.is_open()) warnFile_.close();
    if (errorFile_.is_open()) errorFile_.close();
}

bool Logger::initialize(const std::string& logDir, LogLevel logLevel) {
    try {
        // 防止重复初始化
        if (initialized_) {
            return true;
        }
        
        // 保存日志级别
        logLevel_ = logLevel;
        
        // 确保日志目录存在
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        
        // 构建日志文件路径
        std::string infoPath = logDir + "/simple_info.log";
        std::string warnPath = logDir + "/simple_warn.log";
        std::string errorPath = logDir + "/simple_error.log";
        
        // 打开日志文件，使用追加模式
        infoFile_.open(infoPath, std::ios::app);
        warnFile_.open(warnPath, std::ios::app);
        errorFile_.open(errorPath, std::ios::app);
        
        // 检查文件是否成功打开
        if (!infoFile_.is_open() || !warnFile_.is_open() || !errorFile_.is_open()) {
            return false;
        }
        
        // 初始化成功
        initialized_ = true;
        
        // 写入初始化成功的日志
        std::string initMsg = "日志系统初始化成功，日志目录：" + logDir;
        info(initMsg);
        
        return true;
    } catch (const std::exception&) {
        return false;
    } catch (...) {
        return false;
    }
}

void Logger::debug(const std::string& message) {
    if (logLevel_ > LogLevel::DEBUG) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [DEBUG] " + message;
    
    // 只写入到文件，不输出到屏幕
    try {
        if (initialized_ && infoFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            infoFile_ << logMsg << std::endl;
        }
    } catch (...) {
        // 忽略写入错误
    }
}

void Logger::info(const std::string& message) {
    if (logLevel_ > LogLevel::INFO) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [INFO] " + message;
    
    // 只写入到日志文件，不输出到屏幕
    try {
        if (initialized_ && infoFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            infoFile_ << logMsg << std::endl;
        }
    } catch (...) {
        // 忽略写入错误
    }
}

void Logger::warning(const std::string& message) {
    if (logLevel_ > LogLevel::WARNING) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [WARNING] " + message;
    
    // 只写入到日志文件，不输出到屏幕
    try {
        if (initialized_ && warnFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
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
    
    // 只写入到日志文件，不输出到屏幕
    try {
        if (initialized_ && errorFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
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
    
    // 只写入到日志文件，不输出到屏幕
    try {
        if (initialized_ && errorFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
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
