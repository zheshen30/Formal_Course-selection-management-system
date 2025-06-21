#include "../include/system/CourseSystem.h"
#include <iostream>
#include <string>
#include <filesystem>

int main(int argc, char* argv[]) {
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
