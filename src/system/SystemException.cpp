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
#include "../../include/system/SystemException.h"

SystemException::SystemException(ErrorType type, const std::string& message)
    : std::runtime_error(message), // 调用基类构造函数，传入错误消息
      type_(type) {
}

std::string SystemException::getTypeString() const {
    return errorTypeToString(type_);
}

std::string SystemException::getFormattedMessage() const {
    return "[" + getTypeString() + "] " + std::string(what()); 
}

std::string SystemException::errorTypeToString(ErrorType type) {
    switch (type) {
        // 数据错误
        case ErrorType::DATA_NOT_FOUND:
            return "数据不存在";
        case ErrorType::DATA_ALREADY_EXISTS:
            return "数据已存在";
        case ErrorType::DATA_INVALID:
            return "数据无效";

        // 文件错误
        case ErrorType::FILE_NOT_FOUND:
            return "文件不存在";
        case ErrorType::FILE_ACCESS_DENIED:
            return "文件访问被拒绝";
        case ErrorType::FILE_CORRUPTED:
            return "文件已损坏";

        // 权限错误
        case ErrorType::PERMISSION_DENIED:
            return "权限不足";
        case ErrorType::AUTHENTICATION_FAILED:
            return "认证失败";

        // 业务逻辑错误
        case ErrorType::COURSE_FULL:
            return "课程已满";
        case ErrorType::ALREADY_ENROLLED:
            return "已选课程";
        case ErrorType::NOT_ENROLLED:
            return "未选课程";

        // 并发错误
        case ErrorType::LOCK_TIMEOUT:
            return "锁定超时";
        case ErrorType::LOCK_FAILURE:
            return "锁定失败";
        case ErrorType::CONCURRENT_MODIFICATION:
            return "并发修改冲突";

        // 其他错误
        case ErrorType::UNKNOWN_ERROR:
            return "未知错误";
        case ErrorType::INITIALIZATION_FAILED:
            return "初始化失败";
        case ErrorType::INVALID_INPUT:
            return "输入无效";
        case ErrorType::OPERATION_FAILED:
            return "操作失败";

        default:
            return "未知错误类型";
    }
}
