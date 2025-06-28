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

bool InputValidator::validateInteger(const std::string& input, int min, int max, int& result) {
    try {
        // 检查是否只包含数字和正负号
        if (!std::regex_match(input, std::regex("[+-]?[0-9]+"))) {
            return false;
        }
        // 转换为整数
        result = std::stoi(input);
        // 检查范围
        return result >= min && result <= max;
    } catch (const std::exception&) {
        return false;
    }
}

bool InputValidator::validateDouble(const std::string& input, double min, double max, double& result) {
    try {
        // 检查是否为有效的浮点数格式
        if (!std::regex_match(input, std::regex("[+-]?([0-9]*[.])?[0-9]+"))) {
            return false;
        }
        
        // 转换为浮点数
        result = std::stod(input);
        
        // 检查范围
        return result >= min && result <= max;
    } catch (const std::exception&) {
        return false;
    }
}

bool InputValidator::validateChoice(const std::string& input, int min, int max, int& result) {
    try {
        // 检查是否为空
        if (input.empty()) {
            return false;
        }
        
        // 检查是否只包含数字
        for (char c : input) {
            if (!std::isdigit(c)) {
                return false;
            }
        }
        
        // 转换为整数
        result = std::stoi(input);
        
        // 检查范围
        bool isValid = (result >= min && result <= max);
        return isValid;
    } catch (const std::invalid_argument& e) {
        Logger::getInstance().warning("输入验证错误（无效参数）: " + std::string(e.what()));
        return false;
    } catch (const std::out_of_range& e) {
        Logger::getInstance().warning("输入验证错误（超出范围）: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        Logger::getInstance().warning("输入验证错误（其他）: " + std::string(e.what()));
        return false;
    } catch (...) {
        Logger::getInstance().warning("输入验证错误（未知异常）");
        return false;
    }
}

bool InputValidator::isEmptyInput(const std::string& input) {
    std::string trimmed = input;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
    return trimmed.empty();
}

