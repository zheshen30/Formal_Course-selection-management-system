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
#include "../include/system/CourseSystem.h"
#include "../include/util/Logger.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

// 获取主程序的数据目录：主目录下的data目录
std::string getDataDir() {
    // 主程序应该在build目录下运行，因此数据目录是../data（相对于build目录）
    std::string dataDir = "../data";
    
    // 获取绝对路径
    fs::path absolutePath = fs::absolute(dataDir);
    
    // 确保目录存在
    try {
        if (!fs::exists(absolutePath)) {
            Logger::getInstance().critical("数据目录不存在: " + absolutePath.string());
            throw std::runtime_error("数据目录不存在: " + absolutePath.string());
        }
    } catch (const std::exception& e) {
        std::cerr << "数据目录错误: " << e.what() << std::endl;
        throw;
    }
    
    return absolutePath.string();
}

// 获取主程序的日志目录：主目录下的log目录
std::string getLogDir() {
    // 主程序应该在build目录下运行，因此日志目录是../log（相对于build目录）
    std::string logDir = "../log";
    
    // 获取绝对路径
    std::filesystem::path absolutePath = std::filesystem::absolute(logDir);
    
    // 确保目录存在
    try {
        if (!std::filesystem::exists(absolutePath)) {
            std::filesystem::create_directories(absolutePath);
        }
    } catch (const std::exception& e) {
        std::cerr << "创建日志目录失败: " << e.what() << std::endl;
    }
    
    return absolutePath.string();
}

int main() {
    // 获取数据目录和日志目录
    std::string dataDir = getDataDir();
    std::string logDir = getLogDir();
    
    // 检查关键文件
    try {
        bool hasChineseJson = std::filesystem::exists(dataDir + "/Chinese.json");
        bool hasEnglishJson = std::filesystem::exists(dataDir + "/English.json");
        
        // 如果文件不存在，返回错误
        if (!hasChineseJson) {
            throw std::runtime_error("缺少必要文件：Chinese.json");
        }
        
        if (!hasEnglishJson) {
            throw std::runtime_error("缺少必要文件：English.json");
        }
    } catch (const std::exception& e) {
        std::cerr << "检查语言文件时异常: " << e.what() << std::endl;
    }
    
    // 初始化系统日志
    Logger& logger = Logger::getInstance();
    try {
        if (!logger.initialize(logDir, LogLevel::DEBUG)) { //此处根据实际使用需求过滤不同的日志等级
            std::cerr << "日志系统初始化失败！继续执行但日志功能可能不可用" << std::endl;
        } else {
            logger.info("日志系统初始化成功");
            logger.info("数据目录: " + dataDir);
            logger.info("日志目录: " + logDir);
        }
    } catch (const std::exception& e) {
        std::cerr << "初始化日志系统时异常: " << e.what() << std::endl;
        // 即使日志初始化失败，也继续执行
    }
    
    // 获取CourseSystem单例
    CourseSystem& system = CourseSystem::getInstance();
    
    // 初始化系统
    try {
        bool initSuccess = system.initialize(dataDir);
        if (!initSuccess) {
            std::cerr << "系统初始化失败" << std::endl;
            if (logger.initialize(logDir, LogLevel::CRITICAL)) { // 确保日志系统正常运行
                logger.critical("系统初始化失败");
            }
            // 暂停以便查看错误信息
            std::cout << "按回车键退出...";
            std::cin.get();
            return 1;
        }
        
        // 运行系统主循环
        return system.run();
    } catch (const std::exception& e) {
        std::cerr << "系统发生严重错误: " << e.what() << std::endl;
        try {
            if (logger.initialize(logDir, LogLevel::CRITICAL)) {
                logger.critical("系统崩溃: " + std::string(e.what()));
            }
        } catch (...) {}    // 忽略日志错误
        // 暂停以便查看错误信息
        std::cerr << "系统崩溃。按回车键退出...";
        std::cin.get();
        return 1;
    } catch (...) {
        std::cerr << "系统发生未知严重错误！" << std::endl;
        // 暂停以便查看错误信息
        std::cerr << "系统崩溃。按回车键退出...";
        std::cin.get();
        return 1;
    }
}
