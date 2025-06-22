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

// 获取主程序的数据目录：主目录下的data目录
std::string getDataDir() {
    // 主程序应该在build目录下运行，因此数据目录是../data（相对于build目录）
    std::string dataDir = "../data";
    
    // 获取绝对路径
    std::filesystem::path absolutePath = std::filesystem::absolute(dataDir);
    std::cout << "数据目录绝对路径: " << absolutePath.string() << std::endl;
    
    // 确保目录存在
    try {
        if (!std::filesystem::exists(absolutePath)) {
            std::cout << "数据目录不存在，正在创建..." << std::endl;
            std::filesystem::create_directories(absolutePath);
        }
    } catch (const std::exception& e) {
        std::cerr << "创建数据目录失败: " << e.what() << std::endl;
    }
    
    // 检查关键文件
    if (!std::filesystem::exists(absolutePath / "Chinese.json") || 
        !std::filesystem::exists(absolutePath / "English.json")) {
        std::cout << "警告: 关键语言文件不存在，需要创建默认语言文件" << std::endl;
    }
    
    return absolutePath.string();
}

// 获取主程序的日志目录：主目录下的log目录
std::string getLogDir() {
    // 主程序应该在build目录下运行，因此日志目录是../log（相对于build目录）
    std::string logDir = "../log";
    
    // 获取绝对路径
    std::filesystem::path absolutePath = std::filesystem::absolute(logDir);
    std::cout << "日志目录绝对路径: " << absolutePath.string() << std::endl;
    
    // 确保目录存在
    try {
        if (!std::filesystem::exists(absolutePath)) {
            std::cout << "日志目录不存在，正在创建..." << std::endl;
            std::filesystem::create_directories(absolutePath);
        }
    } catch (const std::exception& e) {
        std::cerr << "创建日志目录失败: " << e.what() << std::endl;
    }
    
    return absolutePath.string();
}

int main() {
    // 显示当前工作目录
    try {
        std::cout << "当前工作目录: " << std::filesystem::current_path().string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "获取当前工作目录异常: " << e.what() << std::endl;
    }
    
    // 获取数据目录和日志目录
    std::string dataDir = getDataDir();
    std::string logDir = getLogDir();
    
    // 显示选择的目录
    std::cout << "使用的数据目录: " << dataDir << std::endl;
    std::cout << "使用的日志目录: " << logDir << std::endl;
    
    // 检查关键文件
    try {
        bool hasChineseJson = std::filesystem::exists(dataDir + "/Chinese.json");
        bool hasEnglishJson = std::filesystem::exists(dataDir + "/English.json");
        
        std::cout << "Chinese.json 存在: " << (hasChineseJson ? "是" : "否") << std::endl;
        std::cout << "English.json 存在: " << (hasEnglishJson ? "是" : "否") << std::endl;
        
        // 如果文件不存在，创建简单的默认内容
        if (!hasChineseJson) {
            std::cout << "尝试创建默认的Chinese.json文件..." << std::endl;
            std::ofstream chineseFile(dataDir + "/Chinese.json");
            chineseFile << R"({
  "main_menu_title": "主菜单",
  "login": "登录",
  "exit": "退出",
  "invalid_input": "输入无效，请重新输入",
  "enter_user_id": "请输入用户ID",
  "enter_password": "请输入密码",
  "login_success": "登录成功",
  "login_failed": "登录失败，用户ID或密码错误",
  "system_error": "系统错误"
})";
            chineseFile.close();
        }
        
        if (!hasEnglishJson) {
            std::cout << "尝试创建默认的English.json文件..." << std::endl;
            std::ofstream englishFile(dataDir + "/English.json");
            englishFile << R"({
  "main_menu_title": "Main Menu",
  "login": "Login",
  "exit": "Exit",
  "invalid_input": "Invalid input, please try again",
  "enter_user_id": "Please enter user ID",
  "enter_password": "Please enter password",
  "login_success": "Login successful",
  "login_failed": "Login failed, incorrect user ID or password",
  "system_error": "System Error"
})";
            englishFile.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "检查或创建语言文件时异常: " << e.what() << std::endl;
    }
    
    // 初始化系统日志
    std::cout << "初始化日志系统..." << std::endl;
    Logger& logger = Logger::getInstance();
    try {
        if (!logger.initialize(logDir, LogLevel::DEBUG)) {
            std::cerr << "日志系统初始化失败！继续执行但日志功能可能不可用" << std::endl;
        } else {
            std::cout << "日志系统初始化成功" << std::endl;
            logger.info("日志系统初始化成功");
            logger.info("数据目录: " + dataDir);
            logger.info("日志目录: " + logDir);
        }
    } catch (const std::exception& e) {
        std::cerr << "初始化日志系统时异常: " << e.what() << std::endl;
        // 即使日志初始化失败，也继续执行
    }
    
    // 获取CourseSystem单例
    std::cout << "获取CourseSystem实例..." << std::endl;
    CourseSystem& system = CourseSystem::getInstance();
    
    // 初始化系统
    try {
        std::cout << "开始初始化系统..." << std::endl;
        bool initSuccess = system.initialize(dataDir, logDir);
        if (!initSuccess) {
            std::cerr << "系统初始化失败" << std::endl;
            if (logger.initialize(logDir, LogLevel::ERROR)) { // 再次尝试初始化日志
                logger.error("系统初始化失败");
            }
            // 暂停以便查看错误信息
            std::cout << "按回车键退出...";
            std::cin.get();
            return 1;
        }
        
        std::cout << "初始化成功，开始运行系统..." << std::endl;
        // 运行系统主循环
        return system.run();
    } catch (const std::exception& e) {
        std::cerr << "系统发生严重错误: " << e.what() << std::endl;
        try {
            if (logger.initialize(logDir, LogLevel::CRITICAL)) {
                logger.critical("系统崩溃: " + std::string(e.what()));
            }
        } catch (...) {
            // 忽略日志错误，直接显示到控制台
        }
        // 暂停以便查看错误信息
        std::cout << "系统崩溃。按回车键退出...";
        std::cin.get();
        return 1;
    } catch (...) {
        std::cerr << "系统发生未知严重错误！" << std::endl;
        // 暂停以便查看错误信息
        std::cout << "系统崩溃。按回车键退出...";
        std::cin.get();
        return 1;
    }
}
