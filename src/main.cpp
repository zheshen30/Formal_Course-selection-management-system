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
#include <iostream>
#include <string>
#include <filesystem>

int main() {
    // 默认目录
    std::string dataDir = "./data";
    std::string logDir = "./log";
    
    // 创建必要的目录
    std::filesystem::create_directories(dataDir);
    std::filesystem::create_directories(logDir);
    
    // 获取CourseSystem单例
    CourseSystem& system = CourseSystem::getInstance();
    
    // 初始化系统
    try {
        bool initSuccess = system.initialize(dataDir, logDir);
        if (!initSuccess) {
            std::cerr << "系统初始化失败" << std::endl;
            return 1;
        }
        
        // 运行系统主循环
        return system.run();
    } catch (const std::exception& e) {
        std::cerr << "系统发生错误：" << e.what() << std::endl;
        return 1;
    }
}
