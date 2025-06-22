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

/**
 * @brief 语言枚举
 */
enum class Language {
    CHINESE,
    ENGLISH
};

/**
 * @brief 国际化管理类，提供多语言支持
 */
class I18nManager {
public:
    /**
     * @brief 获取单例实例
     * @return I18nManager单例引用
     */
    static I18nManager& getInstance();
    
    /**
     * @brief 初始化语言资源
     * @param dataDir 数据目录
     * @return 是否初始化成功
     */
    bool initialize(const std::string& dataDir);
    
    /**
     * @brief 设置当前语言
     * @param language 语言枚举
     * @return 是否设置成功
     */
    bool setLanguage(Language language);
    
    /**
     * @brief 获取当前语言
     * @return 当前语言枚举
     */
    Language getCurrentLanguage() const;
    
    /**
     * @brief 获取翻译文本
     * @param key 文本键
     * @return 翻译后的文本，找不到时返回键本身
     */
    std::string getText(const std::string& key) const;
    
    /**
     * @brief 获取带参数的翻译文本
     * @param key 文本键
     * @param args 参数值
     * @return 翻译后的文本，找不到时返回键本身
     */
    template<typename... Args>
    std::string getFormattedText(const std::string& key, Args... args) const;
    
    /**
     * @brief 获取语言的字符串表示
     * @param language 语言枚举
     * @return 语言字符串
     */
    static std::string languageToString(Language language);
    
    /**
     * @brief 从字符串解析语言
     * @param languageStr 语言字符串
     * @return 语言枚举
     */
    static Language stringToLanguage(const std::string& languageStr);

private:
    /**
     * @brief 私有构造函数，确保单例
     */
    I18nManager();
    
    /**
     * @brief 删除拷贝构造函数
     */
    I18nManager(const I18nManager&) = delete;
    
    /**
     * @brief 删除赋值运算符
     */
    I18nManager& operator=(const I18nManager&) = delete;
    
    /**
     * @brief 加载语言文件
     * @param language 语言枚举
     * @return 是否加载成功
     */
    bool loadLanguageFile(Language language);
    
    /**
     * @brief 获取语言文件路径
     * @param language 语言枚举
     * @return 文件路径
     */
    std::string getLanguageFilePath(Language language) const;

    std::string dataDir_; ///< 数据目录
    Language currentLanguage_ = Language::CHINESE; ///< 当前语言
    std::unordered_map<std::string, std::string> textMap_; ///< 文本映射表
    mutable std::mutex mutex_; ///< 互斥锁
    bool initialized_ = false; ///< 是否已初始化
}; 