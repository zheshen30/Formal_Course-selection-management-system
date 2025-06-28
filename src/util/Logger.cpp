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


std::string getCurrentTimeString() {
    // 获取系统时间
    auto now = std::chrono::system_clock::now();
    
    // 转换为本地时间
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* local_tm = std::localtime(&now_time_t);
    if (!local_tm) {
        return "ERROR_TIME";
    }
    
    // 获取毫秒部分
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // 格式化时间字符串
    std::stringstream ss;
    ss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

Logger& Logger::getInstance() {
    static Logger instance; // Meyer's单例模式
    return instance;
}

Logger::Logger() : initialized_(false), logLevel_(LogLevel::INFO) {}

Logger::~Logger() {
    // 确保所有文件都关闭
    if (infoFile_.is_open()) infoFile_.close();
    if (warnFile_.is_open()) warnFile_.close();
    if (errorFile_.is_open()) errorFile_.close();
    if (criticalFile_.is_open()) criticalFile_.close();
    if (debugFile_.is_open()) debugFile_.close();   
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
        std::string infoPath = logDir + "/Info.log";
        std::string warnPath = logDir + "/Warn.log";
        std::string errorPath = logDir + "/Error.log";
        std::string criticalPath = logDir + "/Critical.log";
        std::string debugPath = logDir + "/Debug.log";
        
        // 打开日志文件，使用追加模式
        infoFile_.open(infoPath, std::ios::app);
        warnFile_.open(warnPath, std::ios::app);
        errorFile_.open(errorPath, std::ios::app);
        criticalFile_.open(criticalPath, std::ios::app);
        debugFile_.open(debugPath, std::ios::app);  
        
        // 检查文件是否成功打开
        if (!infoFile_.is_open() || !warnFile_.is_open() || !errorFile_.is_open() || !criticalFile_.is_open() || !debugFile_.is_open()) {
            return false;
        }
        
        // 初始化成功
        initialized_ = true;
        
        // 写入初始化成功的日志
        std::string initMsg = "日志系统初始化成功，日志目录：" + logDir;
        info(initMsg);
        
        return true;
    } catch (const std::exception&) {
        return false; //捕获C++标准异常
    } catch (...) {
        return false; //捕获其他异常
    }
}

void Logger::debug(const std::string& message) {
    if (logLevel_ > LogLevel::DEBUG) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [DEBUG] " + message;
    
    try {
        if (initialized_ && infoFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            debugFile_ << logMsg << std::endl;
        }
    } catch (...) {
        // 忽略写入错误，确保不影响程序运行
    }
}

void Logger::info(const std::string& message) {
    if (logLevel_ > LogLevel::INFO) return;  // 如果日志级别大于INFO，则不记录
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [INFO] " + message;
    
    try {
        if (initialized_ && infoFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            infoFile_ << logMsg << std::endl;
            // 同时写入下级日志文件
            if (debugFile_.is_open()) {
                debugFile_ << logMsg << std::endl;
            }
        }
    } catch (...) {
        // 忽略写入错误，确保不影响程序运行
    }
}

void Logger::warning(const std::string& message) {
    if (logLevel_ > LogLevel::WARNING) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [WARNING] " + message;
    
    try {
        if (initialized_ && warnFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            warnFile_ << logMsg << std::endl;
            // 同时写入下级日志文件
            if (infoFile_.is_open()) {
                infoFile_ << logMsg << std::endl;
            }
            if (debugFile_.is_open()) {
                debugFile_ << logMsg << std::endl;
            }
        }
    } catch (...) {
        // 忽略写入错误，确保不影响程序运行
    }
}

void Logger::error(const std::string& message) {
    if (logLevel_ > LogLevel::ERROR) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [ERROR] " + message;
    
    try {
        if (initialized_ && errorFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            errorFile_ << logMsg << std::endl;
            // 同时写入下级日志文件
            if (warnFile_.is_open()) {
                warnFile_ << logMsg << std::endl;
            }
            if (infoFile_.is_open()) {
                infoFile_ << logMsg << std::endl;
            }
            if (debugFile_.is_open()) {
                debugFile_ << logMsg << std::endl;
            }
        }
    } catch (...) {
        // 忽略写入错误，确保不影响程序运行
    }
}

void Logger::critical(const std::string& message) {
    if (logLevel_ > LogLevel::CRITICAL) return;
    
    std::string logMsg = "[" + getCurrentTimeString() + "] [CRITICAL] " + message;
    
    try {
        if (initialized_ && errorFile_.is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            criticalFile_ << logMsg << std::endl;
            // 同时写入下级日志文件
            if (warnFile_.is_open()) {
                warnFile_ << logMsg << std::endl;
            }
            if (infoFile_.is_open()) {
                infoFile_ << logMsg << std::endl;
            }
            if (debugFile_.is_open()) {
                debugFile_ << logMsg << std::endl;
            }
            if (errorFile_.is_open()) {
                errorFile_ << logMsg << std::endl;
            }
        }
    } catch (...) {
        // 忽略写入错误，确保不影响程序运行
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
