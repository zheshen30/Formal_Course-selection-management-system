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
#pragma once

#include <string>
#include <mutex>
#include <fstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static Logger& getInstance();
    
    bool initialize(const std::string& logDir, LogLevel logLevel);
    
    void debug(const std::string& message);
    
    void info(const std::string& message);
    
    void warning(const std::string& message);
    
    void error(const std::string& message);
    
    void critical(const std::string& message);
    
    void setLogLevel(LogLevel level);

    static std::string logLevelToString(LogLevel level);


    static LogLevel stringToLogLevel(const std::string& levelStr);

private:
    Logger();
    
    Logger(const Logger&) = delete;
    
    Logger& operator=(const Logger&) = delete;
    
    ~Logger();
    
    bool initialized_ = false;    // 是否已初始化
    LogLevel logLevel_;           // 日志级别
    std::mutex mutex_;            // 互斥锁
    
    // 日志文件流
    std::ofstream infoFile_;      // 信息日志文件流
    std::ofstream warnFile_;      // 警告日志文件流
    std::ofstream errorFile_;     // 错误日志文件流
    std::ofstream criticalFile_;  // 严重错误日志文件流
    std::ofstream debugFile_;     // 调试日志文件流
}; 