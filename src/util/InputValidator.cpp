#include "../../include/util/InputValidator.h"
#include <regex>
#include <sstream>
#include <cctype>
#include <algorithm>

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
    return validateInteger(input, min, max, result);
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
