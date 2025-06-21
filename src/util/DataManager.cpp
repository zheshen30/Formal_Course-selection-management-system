#include "../../include/util/DataManager.h"
#include "../../include/system/SystemException.h"
#include "../../include/util/Logger.h"

#include <fstream>
#include <filesystem>
#include <stdexcept>

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
    LockGuard lock(mutex_, 5000);
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取数据管理器锁超时");
    }
    
    std::string filePath = getDataFilePath(filename);
    
    if (!fileExists(filePath)) {
        Logger::getInstance().warning("文件不存在: " + filePath);
        return "";
    }
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            Logger::getInstance().error("无法打开文件: " + filePath);
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, "无法打开文件: " + filePath);
        }
        
        std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        Logger::getInstance().info("成功加载文件: " + filePath);
        return jsonContent;
    } catch (const SystemException&) {
        throw; // 重新抛出系统异常
    } catch (const std::exception& e) {
        Logger::getInstance().error("加载文件失败: " + filePath + " - " + e.what());
        throw SystemException(ErrorType::FILE_CORRUPTED, "加载文件失败: " + e.what());
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
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, "创建目录失败: " + e.what());
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
            Logger::getInstance().error("重命名临时文件失败: " + e.what());
            throw SystemException(ErrorType::FILE_ACCESS_DENIED, "重命名临时文件失败: " + e.what());
        }
        
        Logger::getInstance().info("成功保存文件: " + filePath);
        return true;
    } catch (const SystemException&) {
        throw; // 重新抛出系统异常
    } catch (const std::exception& e) {
        Logger::getInstance().error("保存文件失败: " + filePath + " - " + e.what());
        throw SystemException(ErrorType::FILE_ACCESS_DENIED, "保存文件失败: " + e.what());
    }
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
