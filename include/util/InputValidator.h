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
#include <functional>
#include <vector>
#include <limits>

/**
 * @brief 输入验证类，提供各种输入验证功能
 */
class InputValidator {
public:
    /**
     * @brief 验证字符串长度
     * @param input 输入字符串
     * @param minLength 最小长度
     * @param maxLength 最大长度
     * @return 是否有效
     */
    static bool validateStringLength(const std::string& input, size_t minLength, size_t maxLength);

    /**
     * @brief 验证字符串是否只包含允许的字符
     * @param input 输入字符串
     * @param allowedChars 允许的字符集合
     * @return 是否有效
     */
    static bool validateStringChars(const std::string& input, const std::string& allowedChars);

    /**
     * @brief 验证整数
     * @param input 输入字符串
     * @param min 最小值
     * @param max 最大值
     * @param result 解析结果输出参数
     * @return 是否有效
     */
    static bool validateInteger(const std::string& input, int min, int max, int& result);

    /**
     * @brief 验证浮点数
     * @param input 输入字符串
     * @param min 最小值
     * @param max 最大值
     * @param result 解析结果输出参数
     * @return 是否有效
     */
    static bool validateDouble(const std::string& input, double min, double max, double& result);

    /**
     * @brief 验证ID格式
     * @param id ID字符串
     * @return 是否有效
     */
    static bool validateId(const std::string& id);

    /**
     * @brief 验证姓名格式
     * @param name 姓名字符串
     * @return 是否有效
     */
    static bool validateName(const std::string& name);

    /**
     * @brief 验证密码格式
     * @param password 密码字符串
     * @return 是否有效
     */
    static bool validatePassword(const std::string& password);

    /**
     * @brief 验证选择项
     * @param input 输入选择
     * @param min 最小选项
     * @param max 最大选项
     * @param result 解析结果输出参数
     * @return 是否有效
     */
    static bool validateChoice(const std::string& input, int min, int max, int& result);

    /**
     * @brief 验证空输入
     * @param input 输入字符串
     * @return 是否为空
     */
    static bool isEmptyInput(const std::string& input);

    /**
     * @brief 验证性别
     * @param gender 性别字符串
     * @return 是否有效
     */
    static bool validateGender(const std::string& gender);

    /**
     * @brief 验证联系方式
     * @param contact 联系方式字符串
     * @return 是否有效
     */
    static bool validateContact(const std::string& contact);
}; 