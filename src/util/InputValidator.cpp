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
#include "../../include/util/InputValidator.h"
#include "../../include/util/Logger.h"
#include <regex>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iostream>

bool InputValidator::validateStringLength(const std::string& input, size_t minLength, size_t maxLength) {
    size_t length = input.length();
    return length >= minLength && length <= maxLength;
}

bool InputValidator::validateStringChars(const std::string& input, const std::string& allowedChars) {
    return input.find_first_not_of(allowedChars) == std::string::npos;
}

bool InputValidator::validateInteger(const std::string& input, int min, int max, int& result) {
    try {
        // 去除空白字符
        std::string trimmed = input;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
        
        // 检查是否为空
        if (trimmed.empty()) {
            return false;
        }
        
        // 检查是否只包含数字和正负号
        if (!std::regex_match(trimmed, std::regex("[+-]?[0-9]+"))) {
            return false;
        }
        
        // 转换为整数
        result = std::stoi(trimmed);
        
        // 检查范围
        return result >= min && result <= max;
    } catch (const std::exception&) {
        return false;
    }
}

bool InputValidator::validateDouble(const std::string& input, double min, double max, double& result) {
    try {
        // 去除空白字符
        std::string trimmed = input;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
        
        // 检查是否为空
        if (trimmed.empty()) {
            return false;
        }
        
        // 检查是否为有效的浮点数格式
        if (!std::regex_match(trimmed, std::regex("[+-]?([0-9]*[.])?[0-9]+"))) {
            return false;
        }
        
        // 转换为浮点数
        result = std::stod(trimmed);
        
        // 检查范围
        return result >= min && result <= max;
    } catch (const std::exception&) {
        return false;
    }
}

bool InputValidator::validateId(const std::string& id) {
    // ID应为6-20位字母数字
    std::regex pattern("^[a-zA-Z0-9]{6,20}$");
    return std::regex_match(id, pattern);
}

bool InputValidator::validateName(const std::string& name) {
    // 姓名应为2-50位字符，不包含特殊字符
    std::regex pattern("^[a-zA-Z0-9\\u4e00-\\u9fa5 ]{2,50}$");
    return std::regex_match(name, pattern);
}

bool InputValidator::validatePassword(const std::string& password) {
    // 密码应为8-20位字符，包含字母和数字
    std::regex pattern("^(?=.*[a-zA-Z])(?=.*[0-9])[a-zA-Z0-9!@#$%^&*]{8,20}$");
    return std::regex_match(password, pattern);
}

bool InputValidator::validateChoice(const std::string& input, int min, int max, int& result) {
    // 记录输入以便调试
    std::cout << "验证输入选择: '" << input << "', 范围: [" << min << "-" << max << "]" << std::endl;
    
    try {
        // 去除空白字符
        std::string trimmed = input;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
        
        // 检查是否为空
        if (trimmed.empty()) {
            std::cout << "输入为空" << std::endl;
            return false;
        }
        
        // 检查是否只包含数字
        for (char c : trimmed) {
            if (!std::isdigit(c)) {
                std::cout << "输入包含非数字字符: '" << c << "'" << std::endl;
                return false;
            }
        }
        
        // 转换为整数
        result = std::stoi(trimmed);
        
        // 检查范围
        bool isValid = (result >= min && result <= max);
        std::cout << "解析结果: " << result << ", 是否有效: " << (isValid ? "是" : "否") << std::endl;
        
        return isValid;
    } catch (const std::invalid_argument& e) {
        std::cout << "解析整数时出现无效参数异常: " << e.what() << std::endl;
        try {
            Logger::getInstance().warning("输入验证错误（无效参数）: " + std::string(e.what()));
        } catch (...) {
            // 忽略日志错误
        }
        return false;
    } catch (const std::out_of_range& e) {
        std::cout << "解析整数时出现超出范围异常: " << e.what() << std::endl;
        try {
            Logger::getInstance().warning("输入验证错误（超出范围）: " + std::string(e.what()));
        } catch (...) {
            // 忽略日志错误
        }
        return false;
    } catch (const std::exception& e) {
        std::cout << "解析整数时出现其他异常: " << e.what() << std::endl;
        try {
            Logger::getInstance().warning("输入验证错误（其他）: " + std::string(e.what()));
        } catch (...) {
            // 忽略日志错误
        }
        return false;
    } catch (...) {
        std::cout << "解析整数时出现未知异常" << std::endl;
        try {
            Logger::getInstance().warning("输入验证错误（未知异常）");
        } catch (...) {
            // 忽略日志错误
        }
        return false;
    }
}

bool InputValidator::isEmptyInput(const std::string& input) {
    std::string trimmed = input;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
    return trimmed.empty();
}

bool InputValidator::validateGender(const std::string& gender) {
    // 性别应为"男"、"女"或"male"、"female"
    std::string lower = gender;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower == "男" || lower == "女" || lower == "male" || lower == "female";
}

bool InputValidator::validateContact(const std::string& contact) {
    // 简单的电子邮件或电话格式验证
    std::regex emailPattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    std::regex phonePattern("^[0-9\\+\\-\\s]{5,20}$");
    
    return std::regex_match(contact, emailPattern) || std::regex_match(contact, phonePattern);
}
