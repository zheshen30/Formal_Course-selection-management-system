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
#include "../../include/util/DataManager.h"
#include "../../include/system/SystemException.h"
#include "../../include/system/LockGuard.h"
#include "../../include/util/Logger.h"

#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <iostream>

namespace fs = std::filesystem;

DataManager& DataManager::getInstance() {
    static DataManager instance;
    return instance;
}

DataManager::DataManager() {
    // 默认数据目录为当前目录下的data子目录
    dataDirectory_ = "./data";
}

std::string DataManager::loadJsonFromFile(const std::string& filename) {
    // 先不加锁，检查文件是否存在
    std::string filePath = getDataFilePath(filename);
    std::cout << "尝试从文件加载JSON: " << filePath << std::endl;

    // 先检查文件是否存在（不需要加锁）
    if (!fileExists(filePath)) {
        Logger::getInstance().warning("文件不存在: " + filePath);
        std::cout << "警告: 文件不存在: " << filePath << std::endl;
        return "";
    }
    
    try {
        // 只在读取文件时短暂加锁
        std::string jsonContent;
        {
            // 使用更短的作用域加锁
            LockGuard lock(mutex_, 1000); // 减少超时时间
            if (!lock.isLocked()) {
                std::cout << "警告: 获取数据管理器锁超时，尝试无锁读取" << std::endl;
                // 如果锁失败，尝试直接读取
            }
            
            std::ifstream file(filePath);
            if (!file.is_open()) {
                Logger::getInstance().error("无法打开文件: " + filePath);
                std::cout << "错误: 无法打开文件: " << filePath << std::endl;
                throw SystemException(ErrorType::FILE_ACCESS_DENIED, "无法打开文件: " + filePath);
            }
            
            jsonContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
        } // 锁在这里释放
        
        std::cout << "文件读取成功，内容大小: " << jsonContent.size() << " 字节" << std::endl;
        if(jsonContent.size() > 100) {
            std::cout << "前100个字符: " << jsonContent.substr(0, 100) << "..." << std::endl;
        } else {
            std::cout << "全部内容: " << jsonContent << std::endl;
        }
        
        Logger::getInstance().info("成功加载文件: " + filePath);
        return jsonContent;
    } catch (const SystemException&) {
        throw; // 重新抛出系统异常
    } catch (const std::exception& e) {
        Logger::getInstance().error("加载文件失败: " + filePath + " - " + e.what());
        std::cout << "错误: 加载文件失败: " << filePath << " - " << e.what() << std::endl;
        throw SystemException(ErrorType::FILE_CORRUPTED, std::string("加载文件失败: ") + e.what());
    }
}

bool DataManager::saveJsonToFile(const std::string& filename, const std::string& jsonData) {
    LockGuard lock(mutex_, 5000);
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取数据管理器锁超时");
    }
    
    std::string filePath = getDataFilePath(filename);
    std::string tempFilePath = filePath + ".tmp";
    
    // 确保目录存在
    fs::path parentPath = fs::path(filePath).parent_path();
    if (!parentPath.empty() && !fs::exists(parentPath)) {
        try {
            fs::create_directories(parentPath);
        } catch (const std::exception& e) {
            Logger::getInstance().error("创建目录失败: " + parentPath.string() + " - " + e.what());
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, std::string("创建目录失败: ") + e.what());
        }
    }
    
    try {
        // 先写入临时文件
        std::ofstream file(tempFilePath);
        if (!file.is_open()) {
            Logger::getInstance().error("无法打开临时文件: " + tempFilePath);
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, "无法打开临时文件: " + tempFilePath);
        }
        
        file << jsonData;
        file.close();
        
        // 检查是否成功写入
        if (!fileExists(tempFilePath)) {
            Logger::getInstance().error("写入临时文件失败: " + tempFilePath);
            return false;
        }
        
        // 重命名临时文件为目标文件
        try {
            if (fs::exists(filePath)) {
                fs::remove(filePath);
            }
            fs::rename(tempFilePath, filePath);
        } catch (const std::exception& e) {
            Logger::getInstance().error(std::string("重命名临时文件失败: ") + e.what());
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, std::string("重命名临时文件失败: ") + e.what());
        }
        
        Logger::getInstance().info("成功保存文件: " + filePath);
        return true;
    } catch (const SystemException&) {
        throw; // 重新抛出系统异常
    } catch (const std::exception& e) {
        Logger::getInstance().error("保存文件失败: " + filePath + " - " + e.what());
        throw SystemException(ErrorType::FILE_ACCESS_DENIED, std::string("保存文件失败: ") + e.what());
    }
    
    return true;  // 添加返回语句，避免警告
}

bool DataManager::fileExists(const std::string& filename) const {
    try {
        return fs::exists(filename);
    } catch (const std::exception& e) {
        Logger::getInstance().warning("检查文件存在性失败: " + filename + " - " + e.what());
        return false;
    }
}

bool DataManager::createDirectory(const std::string& dirname) const {
    try {
        if (fs::exists(dirname)) {
            return fs::is_directory(dirname);
        }
        return fs::create_directories(dirname);
    } catch (const std::exception& e) {
        Logger::getInstance().error("创建目录失败: " + dirname + " - " + e.what());
        return false;
    }
}

std::string DataManager::getDataFilePath(const std::string& filename) const {
    fs::path filePath = fs::path(dataDirectory_) / filename;
    return filePath.string();
}

void DataManager::setDataDirectory(const std::string& dataDir) {
    LockGuard lock(mutex_);
    dataDirectory_ = dataDir;
    
    // 确保目录存在
    if (!createDirectory(dataDirectory_)) {
        throw SystemException(ErrorType::FILE_ACCESS_DENIED, "无法创建或访问数据目录: " + dataDirectory_);
    }
    
    Logger::getInstance().info("设置数据目录: " + dataDirectory_);
}

const std::string& DataManager::getDataDirectory() const {
    return dataDirectory_;
}
