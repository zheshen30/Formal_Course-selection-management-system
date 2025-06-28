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
#include <stdexcept>

enum class ErrorType {
    // 数据错误
    DATA_NOT_FOUND,         // 请求的数据不存在
    DATA_ALREADY_EXISTS,    // 尝试创建已存在的数据
    DATA_INVALID,           // 数据格式或内容无效

    // 文件错误
    FILE_NOT_FOUND,         // 文件不存在
    FILE_ACCESS_DENIED,     // 文件访问被拒绝
    FILE_CORRUPTED,         // 文件已损坏

    // 权限错误
    PERMISSION_DENIED,      // 操作权限不足
    AUTHENTICATION_FAILED,  // 认证失败

    // 业务逻辑错误
    COURSE_FULL,            // 课程已满
    ALREADY_ENROLLED,       // 已经选修此课程
    NOT_ENROLLED,           // 未选修此课程

    // 并发错误
    LOCK_TIMEOUT,           // 锁定超时
    LOCK_FAILURE,           // 锁定失败
    CONCURRENT_MODIFICATION, // 并发修改冲突
    
    // 其他错误
    UNKNOWN_ERROR,          // 未知错误
    INITIALIZATION_FAILED,  // 初始化失败
    INVALID_INPUT,          // 无效输入
    OPERATION_FAILED        // 操作失败
};

class SystemException : public std::runtime_error {
public:
    SystemException(ErrorType type, const std::string& message);
    
    ErrorType getType() const { return type_; }
    
    std::string getTypeString() const;

    std::string getFormattedMessage() const;

    static std::string errorTypeToString(ErrorType type);
    
private:
    ErrorType type_;     // 错误类型
}; 