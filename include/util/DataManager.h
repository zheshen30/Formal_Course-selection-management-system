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
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <unordered_map>

/**
 * @brief 数据管理类，提供统一的数据访问接口
 */
class DataManager {
public:
    /**
     * @brief 获取单例实例
     * @return DataManager单例引用
     */
    static DataManager& getInstance();

    /**
     * @brief 从JSON文件加载数据
     * @param filename 文件名
     * @return JSON字符串
     */
    std::string loadJsonFromFile(const std::string& filename);

    /**
     * @brief 保存JSON数据到文件
     * @param filename 文件名
     * @param jsonData JSON字符串
     * @return 是否保存成功
     */
    bool saveJsonToFile(const std::string& filename, const std::string& jsonData);

    /**
     * @brief 检查文件是否存在
     * @param filename 文件名
     * @return 是否存在
     */
    bool fileExists(const std::string& filename) const;

    /**
     * @brief 创建目录
     * @param dirname 目录名
     * @return 是否创建成功
     */
    bool createDirectory(const std::string& dirname) const;

    /**
     * @brief 获取数据文件路径
     * @param filename 文件名
     * @return 完整路径
     */
    std::string getDataFilePath(const std::string& filename) const;

    /**
     * @brief 设置数据目录
     * @param dataDir 数据目录
     */
    void setDataDirectory(const std::string& dataDir);

    /**
     * @brief 获取数据目录
     * @return 数据目录
     */
    const std::string& getDataDirectory() const;

private:
    /**
     * @brief 私有构造函数，确保单例
     */
    DataManager();
    
    /**
     * @brief 删除拷贝构造函数
     */
    DataManager(const DataManager&) = delete;
    
    /**
     * @brief 删除赋值运算符
     */
    DataManager& operator=(const DataManager&) = delete;

    std::string dataDirectory_; ///< 数据目录
    mutable std::mutex mutex_; ///< 互斥锁
}; 