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

#include <json.hpp>
#include <spdlog/fmt/fmt.h>
#include <stdexcept>
#include <fstream>

using json = nlohmann::json;

I18nManager& I18nManager::getInstance() {
    static I18nManager instance;
    return instance;
}

I18nManager::I18nManager() : currentLanguage_(Language::CHINESE), initialized_(false) {
}

bool I18nManager::initialize(const std::string& dataDir) {
    LockGuard lock(mutex_);
    
    dataDir_ = dataDir;
    initialized_ = false;
    
    // 加载默认语言
    bool result = loadLanguageFile(currentLanguage_);
    if (result) {
        initialized_ = true;
        Logger::getInstance().info("国际化系统初始化成功，数据目录：" + dataDir);
    } else {
        Logger::getInstance().error("国际化系统初始化失败");
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
        Logger::getInstance().error("语言切换失败：" + languageToString(language));
        return false;
    }
}

Language I18nManager::getCurrentLanguage() const {
    return currentLanguage_;
}

std::string I18nManager::getText(const std::string& key) const {
    LockGuard lock(mutex_);
    
    if (!initialized_) {
        return key; // 如果未初始化，直接返回键
    }
    
    auto it = textMap_.find(key);
    if (it != textMap_.end()) {
        return it->second;
    } else {
        Logger::getInstance().warning("翻译键不存在：" + key);
        return key; // 找不到则返回键本身
    }
}

template<typename... Args>
std::string I18nManager::getFormattedText(const std::string& key, Args... args) const {
    std::string text = getText(key);
    try {
        return fmt::format(text, args...);
    } catch (const fmt::format_error& e) {
        Logger::getInstance().error("格式化文本失败：" + std::string(e.what()) + " - 键：" + key);
        return text;
    }
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
        
        DataManager& dataManager = DataManager::getInstance();
        std::string jsonStr = dataManager.loadJsonFromFile(filePath);
        
        if (jsonStr.empty()) {
            Logger::getInstance().warning("语言文件为空或不存在：" + filePath);
            return false;
        }
        
        json langJson = json::parse(jsonStr);
        textMap_.clear();
        
        for (auto it = langJson.begin(); it != langJson.end(); ++it) {
            textMap_[it.key()] = it.value();
        }
        
        Logger::getInstance().info("成功加载语言文件：" + filePath + "，共 " + std::to_string(textMap_.size()) + " 个文本项");
        return true;
    } catch (const json::exception& e) {
        Logger::getInstance().error("解析语言文件JSON失败：" + std::string(e.what()));
        throw SystemException(ErrorType::DATA_INVALID, "解析语言文件失败：" + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::getInstance().error("加载语言文件失败：" + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "加载语言文件失败：" + std::string(e.what()));
    }
}

std::string I18nManager::getLanguageFilePath(Language language) const {
    switch (language) {
        case Language::CHINESE:
            return "Chinese.json";
        case Language::ENGLISH:
            return "English.json";
        default:
            return "Chinese.json"; // 默认为中文
    }
}

// 显式实例化常见的模板实例，以避免链接错误
template std::string I18nManager::getFormattedText(const std::string& key, int) const;
template std::string I18nManager::getFormattedText(const std::string& key, double) const;
template std::string I18nManager::getFormattedText(const std::string& key, const std::string&) const;
template std::string I18nManager::getFormattedText(const std::string& key, std::string) const;
template std::string I18nManager::getFormattedText(const std::string& key, const char*) const;
template std::string I18nManager::getFormattedText(const std::string& key, int, int) const;
template std::string I18nManager::getFormattedText(const std::string& key, const std::string&, const std::string&) const;
template std::string I18nManager::getFormattedText(const std::string& key, std::string, std::string) const;
template std::string I18nManager::getFormattedText(const std::string& key, const std::string&, int) const;
template std::string I18nManager::getFormattedText(const std::string& key, int, const std::string&) const;
// 添加缺失的模板实例化
template std::string I18nManager::getFormattedText(const std::string& key, const char*, const char*) const;
