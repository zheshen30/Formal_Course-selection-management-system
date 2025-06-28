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
#include <unordered_map>
#include <mutex>
#include <memory>

enum class Language {
    CHINESE,
    ENGLISH
};

class I18nManager {
public:
    static I18nManager& getInstance();
    
    bool initialize(const std::string& dataDir);
    
    bool setLanguage(Language language);
    
    Language getCurrentLanguage() const;
    
    std::string getText(const std::string& key) const;
    
    template<typename... Args>
    std::string getFormattedText(const std::string& key, Args... args) const;
    
    static std::string languageToString(Language language);
    
    static Language stringToLanguage(const std::string& languageStr);

private:
    I18nManager();
    
    I18nManager(const I18nManager&) = delete;
    
    I18nManager& operator=(const I18nManager&) = delete;
    
    bool loadLanguageFile(Language language);
    
    std::string getLanguageFilePath(Language language) const;
    
    template<typename T>
    std::string formatValue(const T& value) const;

    std::string formatString(const std::string& format) const;
    
    template<typename T, typename... Args>
    std::string formatString(const std::string& format, T value, Args... args) const;

    std::string dataDir_;                                   // 数据目录
    Language currentLanguage_ = Language::CHINESE;          // 当前语言
    std::unordered_map<std::string, std::string> textMap_;  // 文本映射表
    mutable std::mutex mutex_;                              // 互斥锁
    bool initialized_ = false;                              // 是否已初始化
}; 