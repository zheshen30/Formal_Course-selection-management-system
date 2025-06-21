#pragma once

#include <string>
#include <memory>
#include <mutex>

// 前置声明spdlog相关类
namespace spdlog {
    class logger;
}

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief 日志系统类，提供统一的日志记录接口
 */
class Logger {
public:
    /**
     * @brief 获取单例实例
     * @return Logger单例引用
     */
    static Logger& getInstance();
    
    /**
     * @brief 初始化日志系统
     * @param logDir 日志目录
     * @param logLevel 日志级别
     * @return 是否初始化成功
     */
    bool initialize(const std::string& logDir, LogLevel logLevel);
    
    /**
     * @brief 记录调试信息
     * @param message 日志消息
     */
    void debug(const std::string& message);
    
    /**
     * @brief 记录一般信息
     * @param message 日志消息
     */
    void info(const std::string& message);
    
    /**
     * @brief 记录警告信息
     * @param message 日志消息
     */
    void warning(const std::string& message);
    
    /**
     * @brief 记录错误信息
     * @param message 日志消息
     */
    void error(const std::string& message);
    
    /**
     * @brief 记录严重错误信息
     * @param message 日志消息
     */
    void critical(const std::string& message);
    
    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief 获取日志级别的字符串表示
     * @param level 日志级别
     * @return 级别字符串
     */
    static std::string logLevelToString(LogLevel level);

    /**
     * @brief 从字符串解析日志级别
     * @param levelStr 级别字符串
     * @return 日志级别
     */
    static LogLevel stringToLogLevel(const std::string& levelStr);

private:
    /**
     * @brief 私有构造函数，确保单例
     */
    Logger();
    
    /**
     * @brief 删除拷贝构造函数
     */
    Logger(const Logger&) = delete;
    
    /**
     * @brief 删除赋值运算符
     */
    Logger& operator=(const Logger&) = delete;
    
    /**
     * @brief 析构函数
     */
    ~Logger();
    
    bool initialized_ = false;              ///< 是否已初始化
    std::shared_ptr<spdlog::logger> logger_; ///< spdlog日志器
    std::mutex mutex_;                      ///< 互斥锁
}; 