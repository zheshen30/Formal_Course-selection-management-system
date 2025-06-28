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
    static DataManager instance; // Meyer's单例模式
    return instance;
}

DataManager::DataManager() {
    // 默认数据目录为当前目录下的data子目录
    dataDirectory_ = "./data";
}

std::string DataManager::loadJsonFromFile(const std::string& filename) {
    // 先不加锁，检查文件是否存在
    std::string filePath = getDataFilePath(filename);
    Logger::getInstance().debug("尝试从文件加载JSON: " + filePath);

    if (!fileExists(filePath)) {
        Logger::getInstance().warning("文件不存在: " + filePath);
        return "";
    }
    
    try {
        std::string jsonContent; // 存储文件内容
        {
            LockGuard lock(mutex_, 1000); 
            if (!lock.isLocked()) {
                Logger::getInstance().warning("获取数据管理器锁超时，尝试无锁读取");
            }
            
            std::ifstream file(filePath);
            if (!file.is_open()) {
                Logger::getInstance().error("无法打开文件: " + filePath);
                throw SystemException(ErrorType::FILE_ACCESS_DENIED, "无法打开文件: " + filePath);
            }
            
            jsonContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
        } 
        
        Logger::getInstance().debug("文件读取成功，内容大小: " + std::to_string(jsonContent.size()) + " 字节");
        
        Logger::getInstance().info("成功加载文件: " + filePath);
        return jsonContent;
    } catch (const SystemException&) {
        throw; // 重新抛出系统异常
    } catch (const std::exception& e) {
        Logger::getInstance().error("加载文件失败: " + filePath + " - " + e.what());
        throw SystemException(ErrorType::FILE_CORRUPTED, std::string("加载文件失败: ") + e.what());
    }
}

bool DataManager::saveJsonToFile(const std::string& filename, const std::string& jsonData) {
    
    // 使用getDataFilePath获取正确的文件路径
    std::string filePathStr = getDataFilePath(filename);
    std::string tempFilePath = filePathStr + ".tmp";
    
    // 确保目录存在
    fs::path parentPath = fs::path(filePathStr).parent_path();
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
        
        // 重命名临时文件为目标文件
        try {
            if (fs::exists(filePathStr)) {
                fs::remove(filePathStr);
            }
            fs::rename(tempFilePath, filePathStr);
        } catch (const std::exception& e) {
            Logger::getInstance().error(std::string("重命名临时文件失败: ") + e.what());
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, std::string("重命名临时文件失败: ") + e.what());
        }
        
        Logger::getInstance().info("成功保存文件: " + filePathStr);
        return true;
    } catch (const SystemException&) {
        throw; // 重新抛出系统异常
    } catch (const std::exception& e) {
        Logger::getInstance().error("保存文件失败: " + filePathStr + " - " + e.what());
        throw SystemException(ErrorType::FILE_ACCESS_DENIED, std::string("保存文件失败: ") + e.what());
    }
    
    return false; //（通常不会执行到这里）
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
    // 检查filename 第一次出现 dataDirectory_ 的位置是否为0
    if (filename.find(dataDirectory_) == 0) {
        return filename;
    }   
    
    // 否则拼接路径
    fs::path filePath = fs::path(dataDirectory_) / filename;
    return filePath.string();
}

void DataManager::setDataDirectory(const std::string& dataDir) {
    LockGuard lock(mutex_);
    dataDirectory_ = dataDir;
    
    if (!createDirectory(dataDirectory_)) {
        throw SystemException(ErrorType::FILE_ACCESS_DENIED, "无法创建数据目录: " + dataDirectory_);
    }
    
    Logger::getInstance().info("设置数据目录: " + dataDirectory_);
}

const std::string& DataManager::getDataDirectory() const {
    return dataDirectory_;
}
