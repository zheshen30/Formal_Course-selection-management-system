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
#include "../../include/util/I18nManager.h"
#include "../../include/util/DataManager.h"
#include "../../include/system/LockGuard.h"
#include "../../include/system/SystemException.h"
#include "../../include/util/Logger.h"

#include "../../nlohmann/json.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;
using json = nlohmann::json;

I18nManager& I18nManager::getInstance() {
    static I18nManager instance; // Meyer's单例模式
    return instance;
}

I18nManager::I18nManager() : currentLanguage_(Language::CHINESE), initialized_(false) {}

bool I18nManager::initialize(const std::string& dataDir) {
    // 需要读取数据文件
    LockGuard lock(mutex_);
    
    dataDir_ = dataDir;
    
    // 确保数据目录存在
    if (!fs::exists(dataDir_)) {
        Logger::getInstance().critical("数据目录不存在: " + dataDir_);
        throw SystemException(ErrorType::FILE_NOT_FOUND, "数据目录不存在: " + dataDir_);
    }
    
    // 加载语言文件到哈希表textMap_
    bool result = loadLanguageFile(currentLanguage_);
    if (result) {
        initialized_ = true;
        Logger::getInstance().info("国际化系统初始化成功，数据目录：" + dataDir);
    } else {
        Logger::getInstance().critical("国际化系统初始化失败");
        throw SystemException(ErrorType::DATA_INVALID, "国际化系统初始化失败");
    }
    
    return initialized_;
}

bool I18nManager::setLanguage(Language language) {
    if (language == currentLanguage_) {
        return true; // 已经是当前语言
    }
    
    bool result = loadLanguageFile(language);
    if (result) {
        currentLanguage_ = language;
        Logger::getInstance().info("语言切换成功：" + languageToString(language));
        return true;
    } else {
        Logger::getInstance().critical("语言切换失败：" + languageToString(language));
        throw SystemException(ErrorType::DATA_INVALID, "语言切换失败：" + languageToString(language));
    }
}

Language I18nManager::getCurrentLanguage() const {
    return currentLanguage_;
}

std::string I18nManager::getText(const std::string& key) const {
    try {
        // 先检查是否初始化
        if (!initialized_) {
            Logger::getInstance().critical("I18nManager未初始化");
            throw SystemException(ErrorType::DATA_INVALID, "I18nManager未初始化");
        }

        // 不使用锁：读操作安全性高
        auto it = textMap_.find(key);
        if (it != textMap_.end() && !it->second.empty()) {
            return it->second;
        }
        
        // 未找到，返回键本身
        return key;
    }
    catch (const std::exception& e) {
        Logger::getInstance().error("getText发生异常: " + std::string(e.what()));
        return key;  // 出现任何异常都返回键本身
    }
}

template<typename... Args>
std::string I18nManager::getFormattedText(const std::string& key, Args... args) const {
    std::string text = getText(key);
    try {
        // 简单的字符串替换实现，替换{0}, {1}, {2}等占位符
        return formatString(text, args...);
    } catch (const std::exception& e) {
        Logger::getInstance().error("格式化文本失败：" + std::string(e.what()) + " - 键：" + key);
        return text;
    }
}

// 递归终止条件
std::string I18nManager::formatString(const std::string& format) const {
    return format;
}

// 格式化值为字符串
template<typename T>
std::string I18nManager::formatValue(const T& value) const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// 特化字符串类型，避免额外的转换
template<>
std::string I18nManager::formatValue(const std::string& value) const {
    return value;
}

// 递归格式化函数
template<typename T, typename... Args>
std::string I18nManager::formatString(const std::string& format, T value, Args... args) const {
    
    //　递归终止条件，可变参数包为空
    std::string result = format;
    std::string placeholder = "{0}";
    size_t pos = result.find(placeholder);
    if (pos != std::string::npos) {
        result.replace(pos, placeholder.length(), formatValue(value));
    }
    
    // 递归处理其余参数，并更新后续占位符的索引
    if constexpr (sizeof...(args) > 0) {
        // 更新后续占位符，将{1}, {2}...变为{0}, {1}...
        std::string nextFormat = result;
        for (size_t i = 1; i <= sizeof...(args); ++i) {
            std::string oldPlaceholder = "{" + std::to_string(i) + "}";
            std::string newPlaceholder = "{" + std::to_string(i - 1) + "}";
            
            size_t placeholderPos = nextFormat.find(oldPlaceholder);
            while (placeholderPos != std::string::npos) {
                nextFormat.replace(placeholderPos, oldPlaceholder.length(), newPlaceholder);
                placeholderPos = nextFormat.find(oldPlaceholder, placeholderPos + newPlaceholder.length());
            }
        }
        return formatString(nextFormat, args...);
    }
    
    return result;
}

std::string I18nManager::languageToString(Language language) {
    switch (language) {
        case Language::CHINESE:
            return "Chinese";
        case Language::ENGLISH:
            return "English";
        default:
            return "Unknown";
    }
}

Language I18nManager::stringToLanguage(const std::string& languageStr) {
    if (languageStr == "Chinese") {
        return Language::CHINESE;
    } else if (languageStr == "English") {
        return Language::ENGLISH;
    } else {
        return Language::CHINESE; // 默认为中文
    }
}

bool I18nManager::loadLanguageFile(Language language) {
    try {
        std::string filePath = getLanguageFilePath(language);
        Logger::getInstance().debug("尝试加载语言文件: " + filePath);
        
        // 检查文件是否存在
        bool fileExists = fs::exists(filePath);
        if (!fileExists) {
            Logger::getInstance().critical("语言文件不存在: " + filePath);
            throw SystemException(ErrorType::FILE_NOT_FOUND, "语言数据文件不存在: " + filePath);
        }
        
        // 读取文件内容
        std::ifstream file(filePath);
        if (!file.is_open()) {
            Logger::getInstance().critical("无法打开语言文件: " + filePath);
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, "语言数据文件无法打开: " + filePath);
        }
        
        std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        if (jsonStr.empty()) {
            Logger::getInstance().critical("语言文件为空: " + filePath);
            throw SystemException(ErrorType::DATA_INVALID, "语言数据文件为空: " + filePath);
        }
        
        Logger::getInstance().debug("语言文件内容大小: " + std::to_string(jsonStr.size()) + " 字节");
        
        // 解析JSON
        try {
            json langJson = json::parse(jsonStr); // JSON字符串转换为JSON对象
            
            // 创建一个临时映射表
            std::unordered_map<std::string, std::string> tempMap;
            
            for (auto it = langJson.begin(); it != langJson.end(); ++it) {
                std::string key = it.key();
                std::string value;
                
                // 确保值是字符串类型
                if (it.value().is_string()) {
                    value = it.value().get<std::string>();
                } else {
                    value = it.value().dump(); //将非字符串类型转换为字符串
                }
                
                tempMap[key] = value;
            }
            
            // 检查是否加载了任何键值对
            if (tempMap.empty()) {
                Logger::getInstance().critical("语言文件没有包含任何键值对");
                throw SystemException(ErrorType::DATA_INVALID, "语言数据文件没有包含任何键值对: " + filePath);
            }
            
            // 全部处理完成后，替换现有的映射表
            textMap_ = std::move(tempMap);
            
            Logger::getInstance().info("成功加载语言文件: " + filePath + "，共 " + std::to_string(textMap_.size()) + " 个文本项");
            return true;
        } catch (const json::exception& e) {
            Logger::getInstance().critical("解析语言文件JSON失败: " + std::string(e.what()));
            throw SystemException(ErrorType::DATA_INVALID, "语言数据文件解析失败: " + filePath);
        }
    } catch (...) {
        Logger::getInstance().error("加载语言文件时发生未知异常");
        throw SystemException(ErrorType::UNKNOWN_ERROR, "加载语言文件时发生未知异常");
    }
}

std::string I18nManager::getLanguageFilePath(Language language) const {
    
    std::string filename;
    
    switch (language) {
        case Language::CHINESE:
            filename = "Chinese.json";
            break;
        case Language::ENGLISH:
            filename = "English.json";
            break;
        default:
            filename = "Chinese.json"; // 默认为中文
            break;
    }
    
    // 构建完整路径
    std::string fullPath;
    if (dataDir_.empty()) {
        fullPath = filename; // 如果dataDir_为空，直接使用文件名
    } else {
        if (dataDir_.back() == '/' || dataDir_.back() == '\\') {
            fullPath = dataDir_ + filename; // 如果dataDir_以分隔符结尾，直接连接
        } else {
            fullPath = dataDir_ + "/" + filename; // 否则添加分隔符
        }
    }
    
    return fullPath;
}

// 显式实例化常见的模板实例
template std::string I18nManager::getFormattedText(const std::string& key, int) const;
template std::string I18nManager::getFormattedText(const std::string& key, std::string) const;

template std::string I18nManager::formatValue(const int&) const;
template std::string I18nManager::formatValue(const std::string&) const;

template std::string I18nManager::formatString(const std::string&, int) const;
template std::string I18nManager::formatString(const std::string&, std::string) const;
