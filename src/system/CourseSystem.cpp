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
#include "../../include/manager/UserManager.h"
#include "../../include/manager/CourseManager.h"
#include "../../include/manager/EnrollmentManager.h"

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
    try {
        std::cout << "==== CourseSystem::initialize开始 ====" << std::endl;
    
        // 初始化国际化管理器（这是初始化最关键的部分，因为UI文本都依赖于此）
        std::cout << "初始化国际化管理器..." << std::endl;
        I18nManager& i18n = I18nManager::getInstance();
        if (!i18n.initialize(dataDir)) {
            std::cerr << "初始化国际化系统失败" << std::endl;
            return false;
        }
        std::cout << "国际化管理器初始化成功" << std::endl;
        
        // 先尝试设置语言，确保可以显示UI
        bool langResult = selectLanguage(Language::CHINESE);
        std::cout << "设置默认语言: " << (langResult ? "成功" : "失败") << std::endl;
        
        // 在这里初始化日志系统，避免与CourseSystem产生循环依赖
        std::cout << "初始化日志系统..." << std::endl;
        Logger& logger = Logger::getInstance();
        bool logResult = logger.initialize(logDir, LogLevel::DEBUG);
        std::cout << "日志系统初始化: " << (logResult ? "成功" : "失败") << std::endl;
        
        // 测试日志功能
        if (logResult) {
            logger.debug("这是一条调试日志消息");
            logger.info("CourseSystem正在初始化");
            std::cout << "已写入测试日志记录" << std::endl;
        }
        
        // 初始化数据管理器
        std::cout << "初始化数据管理器..." << std::endl;
        DataManager& dataManager = DataManager::getInstance();
        dataManager.setDataDirectory(dataDir);
        std::cout << "设置数据目录: " << dataDir << std::endl;
        
        try {
            // 加载用户数据
            std::cout << "加载用户数据..." << std::endl;
            UserManager& userManager = UserManager::getInstance();
            bool userDataLoaded = userManager.loadData();
            std::cout << "用户数据加载: " << (userDataLoaded ? "成功" : "失败") << std::endl;
            
            if (!userDataLoaded) {
                std::cerr << "警告: 用户数据加载失败，系统可能无法正常工作" << std::endl;
                if (logResult) {
                    logger.warning("用户数据加载失败，系统可能无法正常工作");
                }
            }
            
            // 加载课程数据
            std::cout << "加载课程数据..." << std::endl;
            CourseManager& courseManager = CourseManager::getInstance();
            bool courseDataLoaded = courseManager.loadData();
            std::cout << "课程数据加载: " << (courseDataLoaded ? "成功" : "失败") << std::endl;
            
            // 加载选课数据
            std::cout << "加载选课数据..." << std::endl;
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            bool enrollmentDataLoaded = enrollmentManager.loadData();
            std::cout << "选课数据加载: " << (enrollmentDataLoaded ? "成功" : "失败") << std::endl;
            
            std::cout << "CourseSystem初始化完成，标记为已初始化" << std::endl;
            initialized_ = true;
            
            if (logResult) {
                logger.info("系统初始化成功");
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "初始化CourseSystem时发生异常: " << e.what() << std::endl;
            if (logResult) {
                logger.error("初始化CourseSystem时发生异常: " + std::string(e.what()));
            }
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "初始化CourseSystem时发生严重异常: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "初始化CourseSystem时发生未知异常" << std::endl;
        return false;
    }
}

int CourseSystem::run() {
    if (!initialized_) {
        std::cerr << "系统未初始化" << std::endl;
        return -1;
    }
    
    running_ = true;
    std::cout << "系统启动成功，准备显示欢迎界面" << std::endl;
    
    // 显示欢迎界面
    showWelcome();
    std::cout << "欢迎界面显示完成，准备选择语言" << std::endl;
    
    try {
        // 选择语言
        std::cout << "请选择语言 / Please select language:" << std::endl;
        std::cout << "1. 中文" << std::endl;
        std::cout << "2. English" << std::endl;
        
        int choice = 0;
        std::string input;
        do {
            std::cout << "> ";
            std::getline(std::cin, input);
            std::cout << "收到输入: " << input << std::endl;
            
            if (!InputValidator::validateChoice(input, 1, 2, choice)) {
                std::cout << "输入无效，请重新选择 / Invalid input, please try again" << std::endl;
            }
        } while (choice < 1 || choice > 2);
        
        std::cout << "选择了选项: " << choice << std::endl;
        
        // 设置语言
        Language language = (choice == 1) ? Language::CHINESE : Language::ENGLISH;
        std::cout << "尝试设置语言: " << I18nManager::languageToString(language) << std::endl;
        if (!selectLanguage(language)) {
            std::cout << "语言设置失败，使用默认语言" << std::endl;
        }
        
        std::cout << "准备进入主循环" << std::endl;
        
        // 主循环
        while (running_) {
            try {
                std::cout << "检查用户登录状态..." << std::endl;
                // 如果未登录，显示登录界面
                if (!currentUser_) {
                    std::cout << "用户未登录，显示主菜单" << std::endl;
                    showMainMenu();
                } else {
                    std::cout << "用户已登录，检查用户类型" << std::endl;
                    // 根据用户类型显示不同菜单
                    switch (currentUser_->getType()) {
                        case UserType::ADMIN:
                            std::cout << "显示管理员菜单" << std::endl;
                            showAdminMenu();
                            break;
                        case UserType::TEACHER:
                            std::cout << "显示教师菜单" << std::endl;
                            showTeacherMenu();
                            break;
                        case UserType::STUDENT:
                            std::cout << "显示学生菜单" << std::endl;
                            showStudentMenu();
                            break;
                        default:
                            std::cout << "未知的用户类型，注销登录" << std::endl;
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
            } catch (...) {
                // 处理未知异常
                Logger::getInstance().error("发生未知异常");
                std::cout << "系统发生未知异常，请重试" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "运行时异常: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "未知异常" << std::endl;
        return -1;
    }
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
    try {
        std::cout << "正在设置语言: " << I18nManager::languageToString(language) << std::endl;
        bool result = I18nManager::getInstance().setLanguage(language);
        
        if (result) {
            std::cout << "语言设置成功: " << I18nManager::languageToString(language) << std::endl;
            
            // 验证一些关键键是否可获取
            std::vector<std::string> testKeys = {"main_menu_title", "login", "exit"};
            bool allKeysFound = true;
            
            for (const auto& key : testKeys) {
                std::string text = getText(key);
                std::cout << "测试键 [" << key << "] = \"" << text << "\"" << std::endl;
                
                if (text == key) {  // 如果getText返回键本身，说明没找到对应的文本
                    allKeysFound = false;
                }
            }
            
            if (!allKeysFound) {
                std::cout << "警告: 部分关键文本键无法获取，菜单可能无法正常显示" << std::endl;
                Logger::getInstance().warning("部分关键文本键无法获取，菜单可能无法正常显示");
            }
            
            return true;
        } else {
            std::cout << "语言设置失败: " << I18nManager::languageToString(language) << std::endl;
            Logger::getInstance().error("语言设置失败: " + I18nManager::languageToString(language));
            
            // 尝试回退到另一种语言
            Language fallbackLanguage = (language == Language::CHINESE) ? Language::ENGLISH : Language::CHINESE;
            std::cout << "尝试回退到: " << I18nManager::languageToString(fallbackLanguage) << std::endl;
            
            return I18nManager::getInstance().setLanguage(fallbackLanguage);
        }
    } catch (const std::exception& e) {
        std::cout << "设置语言时发生异常: " << e.what() << std::endl;
        Logger::getInstance().error("设置语言时发生异常: " + std::string(e.what()));
        return false;
    }
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
    std::cout << "开始显示主菜单..." << std::endl;
    
    try {
        // 显示一个固定的标题，避免依赖I18n
        std::cout << "================================================" << std::endl;
        std::cout << "                  主菜单 / Main Menu             " << std::endl;
        std::cout << "================================================" << std::endl;
        
        // 尝试获取翻译文本，但提供后备选项
        std::string menuTitle;
        try {
            menuTitle = getText("main_menu_title");
            std::cout << "获取到主菜单标题: " << menuTitle << std::endl;
        } catch (...) {
            menuTitle = "主菜单 / Main Menu";
            std::cout << "获取主菜单标题失败，使用默认值" << std::endl;
        }
        
        // 获取登录和退出选项文本，提供后备选项
        std::string loginText, exitText;
        try {
            loginText = getText("login");
            std::cout << "获取到登录选项: " << loginText << std::endl;
        } catch (...) {
            loginText = "登录 / Login";
            std::cout << "获取登录选项失败，使用默认值" << std::endl;
        }
        
        try {
            exitText = getText("exit");
            std::cout << "获取到退出选项: " << exitText << std::endl;
        } catch (...) {
            exitText = "退出 / Exit";
            std::cout << "获取退出选项失败，使用默认值" << std::endl;
        }
        
        // 显示菜单选项
        std::cout << "1. " << loginText << std::endl;
        std::cout << "2. " << exitText << std::endl;
    
        // 获取用户输入
        int choice = 0;
        std::string input;
        do {
            std::cout << "> ";
            std::getline(std::cin, input);
            std::cout << "收到输入: " << input << std::endl;
            
            // 简化输入验证，避免依赖InputValidator
            try {
                choice = std::stoi(input);
                if (choice < 1 || choice > 2) {
                    std::cout << "输入超出范围，请输入1或2 / Input out of range, please enter 1 or 2" << std::endl;
                    choice = 0;
                }
            } catch (...) {
                std::cout << "输入无效，请输入数字 / Invalid input, please enter a number" << std::endl;
                choice = 0;
            }
        } while (choice < 1 || choice > 2);
        
        std::cout << "选择了选项: " << choice << std::endl;
        
        if (choice == 1) {
            // 登录
            std::string userId, password;
            
            // 获取提示文本，提供后备选项
            std::string userPrompt, passPrompt;
            try {
                userPrompt = getText("enter_user_id");
            } catch (...) {
                userPrompt = "请输入用户ID / Please enter user ID";
            }
            
            try {
                passPrompt = getText("enter_password");
            } catch (...) {
                passPrompt = "请输入密码 / Please enter password";
            }
            
            std::cout << userPrompt << ": ";
            std::getline(std::cin, userId);
            std::cout << "输入的用户ID: " << userId << std::endl;
            
            std::cout << passPrompt << ": ";
            std::getline(std::cin, password);
            std::cout << "密码输入完成" << std::endl;
            
            std::cout << "尝试登录..." << std::endl;
            try {
                if (login(userId, password)) {
                    std::string successMsg;
                    try {
                        successMsg = getText("login_success");
                    } catch (...) {
                        successMsg = "登录成功 / Login successful";
                    }
                    std::cout << successMsg << std::endl;
                } else {
                    std::string failMsg;
                    try {
                        failMsg = getText("login_failed");
                    } catch (...) {
                        failMsg = "登录失败，用户ID或密码错误 / Login failed, incorrect user ID or password";
                    }
                    std::cout << failMsg << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            } catch (const std::exception& e) {
                std::cerr << "登录过程中发生异常: " << e.what() << std::endl;
                std::cout << "登录失败，系统错误 / Login failed, system error" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else {
            // 退出
            std::cout << "退出系统... / Exiting system..." << std::endl;
            shutdown();
        }
    } catch (const std::exception& e) {
        std::cerr << "在主菜单中发生异常: " << e.what() << std::endl;
        std::cout << "系统错误，请重试 / System error, please try again" << std::endl;
    } catch (...) {
        std::cerr << "在主菜单中发生未知异常" << std::endl;
        std::cout << "系统发生未知错误 / Unknown system error occurred" << std::endl;
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
