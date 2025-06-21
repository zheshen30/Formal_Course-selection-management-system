#pragma once

#include <string>
#include <stdexcept>

/**
 * @brief 系统错误类型枚举
 */
enum class ErrorType {
    // 数据错误
    DATA_NOT_FOUND,         ///< 请求的数据不存在
    DATA_ALREADY_EXISTS,    ///< 尝试创建已存在的数据
    DATA_INVALID,           ///< 数据格式或内容无效

    // 文件错误
    FILE_NOT_FOUND,         ///< 文件不存在
    FILE_ACCESS_DENIED,     ///< 文件访问被拒绝
    FILE_CORRUPTED,         ///< 文件已损坏

    // 权限错误
    PERMISSION_DENIED,      ///< 操作权限不足
    AUTHENTICATION_FAILED,  ///< 认证失败

    // 业务逻辑错误
    COURSE_FULL,            ///< 课程已满
    ALREADY_ENROLLED,       ///< 已经选修此课程
    NOT_ENROLLED,           ///< 未选修此课程

    // 并发错误
    LOCK_TIMEOUT,           ///< 锁定超时
    CONCURRENT_MODIFICATION, ///< 并发修改冲突
    
    // 其他错误
    UNKNOWN_ERROR,          ///< 未知错误
    INITIALIZATION_FAILED,  ///< 初始化失败
    INVALID_INPUT,          ///< 无效输入
    OPERATION_FAILED        ///< 操作失败
};

/**
 * @brief 系统异常类，统一异常处理基类
 */
class SystemException : public std::runtime_error {
public:
    /**
     * @brief 构造函数
     * @param type 错误类型
     * @param message 错误消息
     * @param errorCode 错误代码
     */
    SystemException(ErrorType type, const std::string& message, int errorCode = 0);
    
    /**
     * @brief 获取错误类型
     * @return 错误类型枚举
     */
    ErrorType getType() const { return type_; }
    
    /**
     * @brief 获取错误代码
     * @return 错误代码
     */
    int getErrorCode() const { return errorCode_; }
    
    /**
     * @brief 获取错误类型的字符串表示
     * @return 错误类型字符串
     */
    std::string getTypeString() const;

    /**
     * @brief 获取格式化的错误消息
     * @return 格式化后的错误消息
     */
    std::string getFormattedMessage() const;

    /**
     * @brief 获取错误类型的字符串表示
     * @param type 错误类型
     * @return 错误类型字符串
     */
    static std::string errorTypeToString(ErrorType type);
    
private:
    ErrorType type_;     ///< 错误类型
    int errorCode_;      ///< 错误代码
}; 