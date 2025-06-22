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
#include "../../include/system/CourseSystem.h"
#include "../../include/util/DataManager.h"
#include "../../include/system/SystemException.h"
#include "../../include/util/InputValidator.h"

#include <iostream>
#include <string>
#include <limits>
#include <thread>
#include <chrono>

CourseSystem& CourseSystem::getInstance() {
    static CourseSystem instance;
    return instance;
}

CourseSystem::CourseSystem()
    : initialized_(false),
      running_(false),
      currentUser_(nullptr) {
}

bool CourseSystem::initialize(const std::string& dataDir, const std::string& logDir) {
    // 初始化日志系统
    Logger& logger = Logger::getInstance();
    if (!logger.initialize(logDir, LogLevel::INFO)) {
        std::cerr << "初始化日志系统失败" << std::endl;
        return false;
    }
    
    // 初始化数据管理器
    DataManager& dataManager = DataManager::getInstance();
    dataManager.setDataDirectory(dataDir);
    
    // 初始化国际化管理器
    I18nManager& i18n = I18nManager::getInstance();
    if (!i18n.initialize(dataDir)) {
        logger.error("初始化国际化系统失败");
        return false;
    }
    
    // 加载用户数据
    UserManager& userManager = UserManager::getInstance();
    try {
        userManager.loadData();
    } catch (const SystemException& e) {
        logger.error("加载用户数据失败: " + e.getFormattedMessage());
        return false;
    } catch (const std::exception& e) {
        logger.error("加载用户数据失败: " + std::string(e.what()));
        return false;
    }
    
    // 加载课程数据
    CourseManager& courseManager = CourseManager::getInstance();
    try {
        courseManager.loadData();
    } catch (const SystemException& e) {
        logger.error("加载课程数据失败: " + e.getFormattedMessage());
        return false;
    } catch (const std::exception& e) {
        logger.error("加载课程数据失败: " + std::string(e.what()));
        return false;
    }
    
    // 加载选课数据
    EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
    try {
        enrollmentManager.loadData();
    } catch (const SystemException& e) {
        logger.error("加载选课数据失败: " + e.getFormattedMessage());
        return false;
    } catch (const std::exception& e) {
        logger.error("加载选课数据失败: " + std::string(e.what()));
        return false;
    }
    
    logger.info("系统初始化成功");
    initialized_ = true;
    return true;
}

int CourseSystem::run() {
    if (!initialized_) {
        std::cerr << "系统未初始化" << std::endl;
        return -1;
    }
    
    running_ = true;
    
    // 显示欢迎界面
    showWelcome();
    
    // 选择语言
    std::cout << "请选择语言 / Please select language:" << std::endl;
    std::cout << "1. 中文" << std::endl;
    std::cout << "2. English" << std::endl;
    
    int choice = 0;
    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!InputValidator::validateChoice(input, 1, 2, choice)) {
            std::cout << "输入无效，请重新选择 / Invalid input, please try again" << std::endl;
        }
    } while (choice < 1 || choice > 2);
    
    // 设置语言
    Language language = (choice == 1) ? Language::CHINESE : Language::ENGLISH;
    selectLanguage(language);
    
    // 主循环
    while (running_) {
        try {
            // 如果未登录，显示登录界面
            if (!currentUser_) {
                showMainMenu();
            } else {
                // 根据用户类型显示不同菜单
                switch (currentUser_->getType()) {
                    case UserType::ADMIN:
                        showAdminMenu();
                        break;
                    case UserType::TEACHER:
                        showTeacherMenu();
                        break;
                    case UserType::STUDENT:
                        showStudentMenu();
                        break;
                    default:
                        Logger::getInstance().error("未知的用户类型");
                        logout();
                        break;
                }
            }
        } catch (const SystemException& e) {
            // 处理系统异常
            Logger::getInstance().error("系统异常: " + e.getFormattedMessage());
            std::cout << getText("system_error") << ": " << e.getFormattedMessage() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        } catch (const std::exception& e) {
            // 处理标准异常
            Logger::getInstance().error("标准异常: " + std::string(e.what()));
            std::cout << getText("system_error") << ": " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    
    return 0;
}

void CourseSystem::shutdown() {
    if (running_) {
        // 保存所有数据
        try {
            UserManager::getInstance().saveData();
            CourseManager::getInstance().saveData();
            EnrollmentManager::getInstance().saveData();
            
            Logger::getInstance().info("系统数据已保存");
        } catch (const std::exception& e) {
            Logger::getInstance().error("保存数据失败: " + std::string(e.what()));
        }
        
        running_ = false;
        Logger::getInstance().info("系统已关闭");
    }
}

bool CourseSystem::login(const std::string& userId, const std::string& password) {
    if (currentUser_) {
        logout(); // 先注销当前用户
    }
    
    UserManager& userManager = UserManager::getInstance();
    User* user = userManager.authenticate(userId, password);
    
    if (user) {
        currentUser_ = user;
        Logger::getInstance().info("用户 " + userId + " 登录成功");
        return true;
    } else {
        Logger::getInstance().warning("用户 " + userId + " 登录失败");
        return false;
    }
}

void CourseSystem::logout() {
    if (currentUser_) {
        Logger::getInstance().info("用户 " + currentUser_->getId() + " 已注销");
        currentUser_ = nullptr;
    }
}

User* CourseSystem::getCurrentUser() const {
    return currentUser_;
}

bool CourseSystem::checkPermission(UserType requiredUserType) const {
    if (!currentUser_) {
        return false;
    }
    
    // 管理员有所有权限
    if (currentUser_->getType() == UserType::ADMIN) {
        return true;
    }
    
    // 其他用户只有匹配的权限
    return currentUser_->getType() == requiredUserType;
}

bool CourseSystem::selectLanguage(Language language) {
    return I18nManager::getInstance().setLanguage(language);
}

Language CourseSystem::getCurrentLanguage() const {
    return I18nManager::getInstance().getCurrentLanguage();
}

std::string CourseSystem::getText(const std::string& key) const {
    return I18nManager::getInstance().getText(key);
}

template<typename... Args>
std::string CourseSystem::getFormattedText(const std::string& key, Args... args) const {
    return I18nManager::getInstance().getFormattedText(key, args...);
}

void CourseSystem::log(LogLevel level, const std::string& message) const {
    Logger& logger = Logger::getInstance();
    
    switch (level) {
        case LogLevel::DEBUG:
            logger.debug(message);
            break;
        case LogLevel::INFO:
            logger.info(message);
            break;
        case LogLevel::WARNING:
            logger.warning(message);
            break;
        case LogLevel::ERROR:
            logger.error(message);
            break;
        case LogLevel::CRITICAL:
            logger.critical(message);
            break;
        default:
            logger.info(message);
            break;
    }
}

void CourseSystem::showWelcome() const {
    std::cout << "================================================" << std::endl;
    std::cout << "               大学选课系统                      " << std::endl;
    std::cout << "      University Course Selection System         " << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

void CourseSystem::showMainMenu() {
    std::cout << getText("main_menu_title") << std::endl;
    std::cout << "1. " << getText("login") << std::endl;
    std::cout << "2. " << getText("exit") << std::endl;
    
    int choice = 0;
    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!InputValidator::validateChoice(input, 1, 2, choice)) {
            std::cout << getText("invalid_input") << std::endl;
        }
    } while (choice < 1 || choice > 2);
    
    if (choice == 1) {
        // 登录
        std::string userId, password;
        
        std::cout << getText("enter_user_id") << ": ";
        std::getline(std::cin, userId);
        
        std::cout << getText("enter_password") << ": ";
        std::getline(std::cin, password);
        
        if (login(userId, password)) {
            std::cout << getText("login_success") << std::endl;
        } else {
            std::cout << getText("login_failed") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else {
        // 退出
        shutdown();
    }
}

void CourseSystem::showAdminMenu() {
    std::cout << getText("admin_menu_title") << std::endl;
    std::cout << "1. " << getText("user_management") << std::endl;
    std::cout << "2. " << getText("course_management") << std::endl;
    std::cout << "3. " << getText("enrollment_query") << std::endl;
    std::cout << "4. " << getText("logout") << std::endl;
    std::cout << "5. " << getText("exit") << std::endl;
    
    int choice = 0;
    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!InputValidator::validateChoice(input, 1, 5, choice)) {
            std::cout << getText("invalid_input") << std::endl;
        }
    } while (choice < 1 || choice > 5);
    
    if (choice >= 1 && choice <= 3) {
        handleAdminFunctions(choice);
    } else if (choice == 4) {
        logout();
    } else {
        shutdown();
    }
}

void CourseSystem::showTeacherMenu() {
    std::cout << getText("teacher_menu_title") << std::endl;
    std::cout << "1. " << getText("view_courses") << std::endl;
    std::cout << "2. " << getText("view_students") << std::endl;
    std::cout << "3. " << getText("logout") << std::endl;
    std::cout << "4. " << getText("exit") << std::endl;
    
    int choice = 0;
    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!InputValidator::validateChoice(input, 1, 4, choice)) {
            std::cout << getText("invalid_input") << std::endl;
        }
    } while (choice < 1 || choice > 4);
    
    if (choice >= 1 && choice <= 2) {
        handleTeacherFunctions(choice);
    } else if (choice == 3) {
        logout();
    } else {
        shutdown();
    }
}

void CourseSystem::showStudentMenu() {
    std::cout << getText("student_menu_title") << std::endl;
    std::cout << "1. " << getText("query_courses") << std::endl;
    std::cout << "2. " << getText("select_course") << std::endl;
    std::cout << "3. " << getText("drop_course") << std::endl;
    std::cout << "4. " << getText("view_selected_courses") << std::endl;
    std::cout << "5. " << getText("logout") << std::endl;
    std::cout << "6. " << getText("exit") << std::endl;
    
    int choice = 0;
    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!InputValidator::validateChoice(input, 1, 6, choice)) {
            std::cout << getText("invalid_input") << std::endl;
        }
    } while (choice < 1 || choice > 6);
    
    if (choice >= 1 && choice <= 4) {
        handleStudentFunctions(choice);
    } else if (choice == 5) {
        logout();
    } else {
        shutdown();
    }
}

void CourseSystem::handleAdminFunctions(int choice) {
    // 这里只是示例，实际应该实现完整的管理功能
    switch (choice) {
        case 1: // 用户管理
            std::cout << getText("user_management_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        case 2: // 课程管理
            std::cout << getText("course_management_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        case 3: // 选课查询
            std::cout << getText("enrollment_query_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        default:
            break;
    }
}

void CourseSystem::handleTeacherFunctions(int choice) {
    // 这里只是示例，实际应该实现完整的教师功能
    switch (choice) {
        case 1: // 查看课程
            std::cout << getText("view_courses_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        case 2: // 查看学生
            std::cout << getText("view_students_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        default:
            break;
    }
}

void CourseSystem::handleStudentFunctions(int choice) {
    // 这里只是示例，实际应该实现完整的学生功能
    switch (choice) {
        case 1: // 查询课程
            std::cout << getText("query_courses_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        case 2: // 选择课程
            std::cout << getText("select_course_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        case 3: // 退选课程
            std::cout << getText("drop_course_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        case 4: // 查看已选课程
            std::cout << getText("view_selected_courses_function") << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
            
        default:
            break;
    }
}

// 显式实例化常用的模板函数，解决链接错误
template std::string CourseSystem::getFormattedText(const std::string& key, int) const;
template std::string CourseSystem::getFormattedText(const std::string& key, const std::string&) const;
template std::string CourseSystem::getFormattedText(const std::string& key, const char*) const;
