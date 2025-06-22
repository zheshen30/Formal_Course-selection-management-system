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
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <iomanip>

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
    std::cout << "I18nManager初始化开始，数据目录: " << dataDir << std::endl;
    
    // 确保数据目录存在
    DataManager& dataManager = DataManager::getInstance();
    if (!dataManager.createDirectory(dataDir_)) {
        Logger::getInstance().error("无法创建或访问语言数据目录: " + dataDir_);
        std::cout << "错误: 无法创建或访问语言数据目录: " << dataDir_ << std::endl;
        return false;
    }
    
    // 预加载所有语言文件
    bool chineseLoaded = loadLanguageFile(Language::CHINESE);
    bool englishLoaded = loadLanguageFile(Language::ENGLISH);
    
    if (!chineseLoaded && !englishLoaded) {
        Logger::getInstance().error("无法加载任何语言文件");
        std::cout << "错误: 无法加载任何语言文件" << std::endl;
        return false;
    }
    
    // 如果当前设置的语言文件加载失败，但另一种语言成功了，则切换到另一种语言
    if ((currentLanguage_ == Language::CHINESE && !chineseLoaded && englishLoaded) ||
        (currentLanguage_ == Language::ENGLISH && !englishLoaded && chineseLoaded)) {
        currentLanguage_ = (chineseLoaded) ? Language::CHINESE : Language::ENGLISH;
        Logger::getInstance().warning("切换到可用语言: " + languageToString(currentLanguage_));
        std::cout << "警告: 切换到可用语言: " << languageToString(currentLanguage_) << std::endl;
    }
    
    // 加载默认语言
    bool result = loadLanguageFile(currentLanguage_);
    if (result) {
        initialized_ = true;
        Logger::getInstance().info("国际化系统初始化成功，数据目录：" + dataDir);
        std::cout << "国际化系统初始化成功，数据目录：" << dataDir << std::endl;
    } else {
        Logger::getInstance().error("国际化系统初始化失败");
        std::cout << "错误: 国际化系统初始化失败" << std::endl;
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
    // 提供一些关键键的硬编码默认值，避免返回空值
    static const std::unordered_map<std::string, std::string> defaultTexts = {
        {"main_menu_title", "主菜单 / Main Menu"},
        {"login", "登录 / Login"},
        {"exit", "退出 / Exit"},
        {"invalid_input", "无效输入 / Invalid Input"},
        {"enter_user_id", "请输入用户ID / Please enter user ID"},
        {"enter_password", "请输入密码 / Please enter password"},
        {"login_success", "登录成功 / Login successful"},
        {"login_failed", "登录失败，用户ID或密码错误 / Login failed, incorrect user ID or password"},
        {"system_error", "系统错误 / System Error"}
    };
    
    try {
        // 先检查是否初始化
        if (!initialized_) {
            std::cout << "警告: I18nManager未初始化，使用默认文本: " << key << std::endl;
            
            // 查找默认文本
            auto defaultIt = defaultTexts.find(key);
            if (defaultIt != defaultTexts.end()) {
                return defaultIt->second;
            }
            
            return key; // 找不到则返回键本身
        }
        
        // 避免使用锁，直接访问textMap_
        // 这可能在多线程环境下不安全，但在这种情况下优先考虑可用性
        auto it = textMap_.find(key);
        if (it != textMap_.end() && !it->second.empty()) {
            return it->second;
        }
        
        // 如果在textMap_中找不到，查找默认文本
        auto defaultIt = defaultTexts.find(key);
        if (defaultIt != defaultTexts.end()) {
            return defaultIt->second;
        }
        
        // 最后返回键本身
        return key;
    }
    catch (const std::exception& e) {
        std::cerr << "getText发生异常: " << e.what() << std::endl;
        
        // 即使发生异常，也尝试返回默认文本
        auto defaultIt = defaultTexts.find(key);
        if (defaultIt != defaultTexts.end()) {
            return defaultIt->second;
        }
        
        return key;  // 出现任何异常都返回键本身
    }
}

template<typename... Args>
std::string I18nManager::getFormattedText(const std::string& key, Args... args) const {
    std::string text = getText(key);
    try {
        // 简单的字符串替换实现，替换{0}, {1}, {2}等占位符
        // 注意：这是一个简化版本，不支持fmt的所有功能
        return formatString(text, args...);
    } catch (const std::exception& e) {
        Logger::getInstance().error("格式化文本失败：" + std::string(e.what()) + " - 键：" + key);
        return text;
    }
}

// 简单的字符串格式化函数，用于替换fmt::format
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

// 特化const char*类型
template<>
std::string I18nManager::formatValue(const char* const& value) const {
    return std::string(value);
}

// 递归终止条件
std::string I18nManager::formatString(const std::string& format) const {
    return format;
}

// 递归格式化函数
template<typename T, typename... Args>
std::string I18nManager::formatString(const std::string& format, T value, Args... args) const {
    std::string result = format;
    std::string placeholder = "{" + std::to_string(sizeof...(args)) + "}";
    
    // 查找并替换占位符
    size_t pos = result.find(placeholder);
    if (pos != std::string::npos) {
        result.replace(pos, placeholder.length(), formatValue(value));
    }
    
    // 递归处理其余参数
    return formatString(result, args...);
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

bool I18nManager::createDefaultLanguageFile(Language language) {
    std::cout << "创建默认语言文件: " << languageToString(language) << std::endl;
    
    nlohmann::json defaultTexts;
    defaultTexts["main_menu_title"] = (language == Language::CHINESE) ? "主菜单" : "Main Menu";
    defaultTexts["admin_menu_title"] = (language == Language::CHINESE) ? "管理员菜单" : "Administrator Menu";
    defaultTexts["teacher_menu_title"] = (language == Language::CHINESE) ? "教师菜单" : "Teacher Menu";
    defaultTexts["student_menu_title"] = (language == Language::CHINESE) ? "学生菜单" : "Student Menu";
    
    defaultTexts["login"] = (language == Language::CHINESE) ? "登录" : "Login";
    defaultTexts["logout"] = (language == Language::CHINESE) ? "注销" : "Logout";
    defaultTexts["exit"] = (language == Language::CHINESE) ? "退出" : "Exit";
    
    defaultTexts["enter_user_id"] = (language == Language::CHINESE) ? "请输入用户ID" : "Please enter user ID";
    defaultTexts["enter_password"] = (language == Language::CHINESE) ? "请输入密码" : "Please enter password";
    defaultTexts["login_success"] = (language == Language::CHINESE) ? "登录成功" : "Login successful";
    defaultTexts["login_failed"] = (language == Language::CHINESE) ? "登录失败，用户ID或密码错误" : "Login failed, incorrect user ID or password";
    
    defaultTexts["invalid_input"] = (language == Language::CHINESE) ? "输入无效，请重新输入" : "Invalid input, please try again";
    
    defaultTexts["system_error"] = (language == Language::CHINESE) ? "系统错误" : "System Error";
    defaultTexts["save_success"] = (language == Language::CHINESE) ? "保存成功" : "Save Successful";
    defaultTexts["save_failed"] = (language == Language::CHINESE) ? "保存失败" : "Save Failed";
    
    std::string jsonData = defaultTexts.dump(4); // 格式化保存，缩进4个空格
    std::string filePath = getLanguageFilePath(language);
    
    try {
        // 直接使用文件流写入文件
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法创建语言文件: " << filePath << std::endl;
            return false;
        }
        
        file << jsonData;
        file.close();
        
        if (!std::filesystem::exists(filePath)) {
            std::cerr << "创建语言文件后文件不存在: " << filePath << std::endl;
            return false;
        }
        
        std::cout << "成功创建默认语言文件: " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "创建默认语言文件时发生异常: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "创建默认语言文件时发生未知异常" << std::endl;
        return false;
    }
}

bool I18nManager::loadLanguageFile(Language language) {
    try {
        std::string filePath = getLanguageFilePath(language);
        std::cout << "尝试加载语言文件: " << filePath << std::endl;
        
        // 检查文件是否存在
        bool fileExists = std::filesystem::exists(filePath);
        if (!fileExists) {
            std::cout << "语言文件不存在: " << filePath << std::endl;
            std::cout << "尝试创建默认语言文件..." << std::endl;
            
            if (!createDefaultLanguageFile(language)) {
                std::cout << "创建默认语言文件失败" << std::endl;
                return false;
            }
            
            // 重新检查文件是否存在
            fileExists = std::filesystem::exists(filePath);
            if (!fileExists) {
                std::cout << "创建后文件仍不存在" << std::endl;
                return false;
            }
        }
        
        // 读取文件内容
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cout << "无法打开语言文件: " << filePath << std::endl;
            return false;
        }
        
        std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        if (jsonStr.empty()) {
            std::cout << "语言文件为空: " << filePath << std::endl;
            return false;
        }
        
        std::cout << "语言文件内容大小: " << jsonStr.size() << " 字节" << std::endl;
        
        // 解析JSON
        try {
            json langJson = json::parse(jsonStr);
            
            // 创建一个临时映射表
            std::unordered_map<std::string, std::string> tempMap;
            
            for (auto it = langJson.begin(); it != langJson.end(); ++it) {
                std::string key = it.key();
                std::string value;
                
                // 确保值是字符串类型
                if (it.value().is_string()) {
                    value = it.value().get<std::string>();
                } else {
                    value = it.value().dump();
                }
                
                tempMap[key] = value;
            }
            
            // 检查是否加载了任何键值对
            if (tempMap.empty()) {
                std::cout << "警告: 语言文件没有包含任何键值对" << std::endl;
                return false;
            }
            
            // 全部处理完成后，替换现有的映射表
            textMap_ = std::move(tempMap);
            
            std::cout << "成功加载语言文件: " << filePath << "，共 " << textMap_.size() << " 个文本项" << std::endl;
            return true;
        } catch (const json::exception& e) {
            std::cout << "解析语言文件JSON失败: " << e.what() << std::endl;
            
            // 尝试创建新的默认文件
            std::cout << "尝试创建新的默认语言文件..." << std::endl;
            if (createDefaultLanguageFile(language)) {
                // 再次尝试加载，但避免无限递归
                std::cout << "重新尝试加载默认语言文件..." << std::endl;
                
                // 直接读取新创建的文件
                std::ifstream newFile(filePath);
                if (!newFile.is_open()) {
                    std::cout << "无法打开新创建的语言文件" << std::endl;
                    return false;
                }
                
                std::string newJsonStr((std::istreambuf_iterator<char>(newFile)), std::istreambuf_iterator<char>());
                newFile.close();
                
                try {
                    json newLangJson = json::parse(newJsonStr);
                    std::unordered_map<std::string, std::string> newTempMap;
                    
                    for (auto it = newLangJson.begin(); it != newLangJson.end(); ++it) {
                        std::string key = it.key();
                        std::string value = it.value().is_string() ? it.value().get<std::string>() : it.value().dump();
                        newTempMap[key] = value;
                    }
                    
                    textMap_ = std::move(newTempMap);
                    std::cout << "成功加载默认语言文件，共 " << textMap_.size() << " 个文本项" << std::endl;
                    return true;
                } catch (...) {
                    std::cout << "解析新创建的默认语言文件失败" << std::endl;
                    return false;
                }
            }
            
            return false;
        }
    } catch (const std::exception& e) {
        std::cout << "加载语言文件时发生异常: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cout << "加载语言文件时发生未知异常" << std::endl;
        return false;
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
    
    std::cout << "语言文件路径: " << fullPath << std::endl;
    return fullPath;
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
template std::string I18nManager::getFormattedText(const std::string& key, const char*, const char*) const;

// 格式化函数的模板实例化
template std::string I18nManager::formatValue(const int&) const;
template std::string I18nManager::formatValue(const double&) const;
template std::string I18nManager::formatValue(const float&) const;
template std::string I18nManager::formatValue(const long&) const;
template std::string I18nManager::formatValue(const unsigned int&) const;
template std::string I18nManager::formatValue(const unsigned long&) const;

// 格式化字符串的模板实例化
template std::string I18nManager::formatString(const std::string&, int) const;
template std::string I18nManager::formatString(const std::string&, double) const;
template std::string I18nManager::formatString(const std::string&, const std::string&) const;
template std::string I18nManager::formatString(const std::string&, std::string) const;
template std::string I18nManager::formatString(const std::string&, const char*) const;
template std::string I18nManager::formatString(const std::string&, int, int) const;
template std::string I18nManager::formatString(const std::string&, const std::string&, const std::string&) const;
template std::string I18nManager::formatString(const std::string&, std::string, std::string) const;
template std::string I18nManager::formatString(const std::string&, const std::string&, int) const;
template std::string I18nManager::formatString(const std::string&, int, const std::string&) const;
template std::string I18nManager::formatString(const std::string&, const char*, const char*) const;
