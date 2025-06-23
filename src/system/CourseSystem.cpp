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
#include <unistd.h>

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
        // 检查输入是否来自终端
        bool isTerminal = isatty(fileno(stdin));
        std::cout << "输入来源 / Input source: " << (isTerminal ? "终端 / Terminal" : "非终端 / Non-terminal (pipe/redirect)") << std::endl;
        
        // 选择语言
        std::cout << "请选择语言 / Please select language:" << std::endl;
        std::cout << "1. 中文 / Chinese" << std::endl;
        std::cout << "2. English / 英文" << std::endl;
        
        int choice = 0;
        std::string input;
        int attempts = 0;
        const int MAX_ATTEMPTS = 5; // 最大尝试次数
        
        // 如果输入不是来自终端，直接使用默认选择
        if (!isTerminal) {
            std::cout << "检测到非终端输入，使用默认选择: 中文" << std::endl;
            choice = 1;
        } else {
            do {
                std::cout << "> ";
                std::getline(std::cin, input);
                std::cout << "收到输入: [" << input << "]" << std::endl;
                
                // 去除输入两端的空白字符
                input.erase(0, input.find_first_not_of(" \t\r\n"));
                input.erase(input.find_last_not_of(" \t\r\n") + 1);
                
                attempts++;
                
                if (input.empty()) {
                    std::cout << "输入为空，请输入数字 / Empty input, please enter a number" << std::endl;
                    continue;
                }
                
                // 直接检查输入是否为"1"或"2"
                if (input == "1") {
                    choice = 1;
                    std::cout << "检测到有效输入: 1" << std::endl;
                    break;
                } else if (input == "2") {
                    choice = 2;
                    std::cout << "检测到有效输入: 2" << std::endl;
                    break;
                } else {
                    std::cout << "输入无效，请输入数字 / Invalid input, please enter a number" << std::endl;
                    
                    if (attempts >= MAX_ATTEMPTS) {
                        std::cout << "多次输入无效，默认选择中文 / Multiple invalid inputs, defaulting to Chinese" << std::endl;
                        choice = 1; // 默认选择中文
                        break;
                    }
                }
            } while (true);
        }
        
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
        // 检查输入是否来自终端
        bool isTerminal = isatty(fileno(stdin));
        std::cout << "输入来源: " << (isTerminal ? "终端" : "非终端(管道/重定向)") << std::endl;
        
        // 确保输入流处于良好状态
        if (std::cin.fail() || !std::cin.good()) {
            std::cout << "输入流状态异常，正在重置..." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
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
        int attempts = 0;
        const int MAX_ATTEMPTS = 5; // 最大尝试次数
        
        // 如果输入不是来自终端，直接使用默认选择
        if (!isTerminal) {
            std::cout << "检测到非终端输入，使用默认选择: 退出" << std::endl;
            choice = 2; // 默认退出
        } else {
            do {
                std::cout << "> ";
                std::getline(std::cin, input);
                std::cout << "收到输入: [" << input << "]" << std::endl;
                
                // 去除输入两端的空白字符
                input.erase(0, input.find_first_not_of(" \t\r\n"));
                input.erase(input.find_last_not_of(" \t\r\n") + 1);
                
                attempts++;
                
                if (input.empty()) {
                    std::cout << "输入为空，请输入数字 / Empty input, please enter a number" << std::endl;
                    continue;
                }
                
                // 直接检查输入是否为"1"或"2"
                if (input == "1") {
                    choice = 1;
                    std::cout << "检测到有效输入: 1" << std::endl;
                    break;
                } else if (input == "2") {
                    choice = 2;
                    std::cout << "检测到有效输入: 2" << std::endl;
                    break;
                } else {
                    std::cout << "输入无效，请输入数字 / Invalid input, please enter a number" << std::endl;
                    
                    if (attempts >= MAX_ATTEMPTS) {
                        std::cout << "多次输入无效，默认选择退出 / Multiple invalid inputs, defaulting to exit" << std::endl;
                        choice = 2; // 默认选择退出
                        break;
                    }
                }
            } while (true);
        }
        
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
    std::cout << "4. " << getText("change_password") << std::endl;
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
    
    if (choice >= 1 && choice <= 3) {
        handleAdminFunctions(choice);
    } else if (choice == 4) {
        handlePasswordChange(); // 处理密码修改
    } else if (choice == 5) {
        logout();
    } else {
        shutdown();
    }
}

void CourseSystem::showTeacherMenu() {
    std::cout << getText("teacher_menu_title") << std::endl;
    std::cout << "1. " << getText("view_courses") << std::endl;
    std::cout << "2. " << getText("view_students") << std::endl;
    std::cout << "3. " << getText("change_password") << std::endl;
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
    
    if (choice >= 1 && choice <= 2) {
        handleTeacherFunctions(choice);
    } else if (choice == 3) {
        handlePasswordChange(); // 处理密码修改
    } else if (choice == 4) {
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
    std::cout << "5. " << getText("change_password") << std::endl;
    std::cout << "6. " << getText("logout") << std::endl;
    std::cout << "7. " << getText("exit") << std::endl;
    
    int choice = 0;
    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!InputValidator::validateChoice(input, 1, 7, choice)) {
            std::cout << getText("invalid_input") << std::endl;
        }
    } while (choice < 1 || choice > 7);
    
    if (choice >= 1 && choice <= 4) {
        handleStudentFunctions(choice);
    } else if (choice == 5) {
        handlePasswordChange(); // 处理密码修改
    } else if (choice == 6) {
        logout();
    } else {
        shutdown();
    }
}

void CourseSystem::handleAdminFunctions(int choice) {
    // 这里只是示例，实际应该实现完整的管理功能
    switch (choice) {
        case 1: { // 用户管理
            std::cout << getText("user_management_function") << std::endl;
            // 实现更完整的用户管理功能
            bool subMenuRunning = true;
            while (subMenuRunning && running_) {
                std::cout << "1. " << getText("add_user") << std::endl;
                std::cout << "2. " << getText("delete_user") << std::endl;
                std::cout << "3. " << getText("query_user") << std::endl;
                std::cout << "4. " << getText("return_to_parent_menu") << std::endl;
                
                int subChoice = 0;
                std::string input;
                std::cout << "> ";
                std::getline(std::cin, input);
                
                if (!InputValidator::validateChoice(input, 1, 4, subChoice)) {
                    std::cout << getText("invalid_input") << std::endl;
                    continue;
                }
                
                switch (subChoice) {
                    case 1: { // 添加用户
                        // 获取用户管理器
                        UserManager& userManager = UserManager::getInstance();
                        
                        // 选择用户类型
                        std::cout << getText("select_user_type") << "：" << std::endl;
                        std::cout << "1. " << getText("student_type") << std::endl;
                        std::cout << "2. " << getText("teacher_type") << std::endl;
                        std::cout << "3. " << getText("admin_type") << std::endl;
                        
                        int userType = 0;
                        std::string userTypeInput;
                        std::cout << "> ";
                        std::getline(std::cin, userTypeInput);
                        
                        if (!InputValidator::validateChoice(userTypeInput, 1, 3, userType)) {
                            std::cout << getText("invalid_user_type") << std::endl;
                            break;
                        }
                        
                        // 收集通用用户信息
                        std::string userId, name, password, gender;
                        
                        std::cout << getText("enter_user_id_prompt") << "：";
                        std::getline(std::cin, userId);
                        
                        // 检查用户ID是否已存在
                        if (userManager.getUser(userId) != nullptr) {
                            std::cout << getText("user_id_exists") << std::endl;
                            break;
                        }
                        
                        std::cout << getText("enter_username") << "：";
                        std::getline(std::cin, name);
                        
                        std::cout << getText("enter_user_password") << "：";
                        std::getline(std::cin, password);
                        
                        std::cout << getText("enter_user_gender") << "：";
                        std::getline(std::cin, gender);
                        
                        // 根据用户类型创建不同的用户对象
                        if (userType == 1) {  // 学生
                            std::string age, department, classInfo, email;
                            
                            std::cout << getText("enter_student_age") << "：";
                            std::getline(std::cin, age);
                            
                            int ageValue = 0;
                            if (!InputValidator::validateInteger(age, 15, 100, ageValue)) {
                                std::cout << getText("invalid_age") << std::endl;
                                break;
                            }
                            
                            std::cout << getText("enter_department") << "：";
                            std::getline(std::cin, department);
                            
                            std::cout << getText("enter_class_info") << "：";
                            std::getline(std::cin, classInfo);
                            
                            std::cout << getText("enter_email") << "：";
                            std::getline(std::cin, email);
                            
                            // 创建学生对象
                            std::unique_ptr<Student> student = std::make_unique<Student>(
                                userId, name, password, gender, ageValue,
                                department, classInfo, email
                            );
                            
                            // 添加学生
                            if (userManager.addStudent(std::move(student))) {
                                std::cout << getText("add_student_success") << std::endl;
                            } else {
                                std::cout << getText("add_student_failed") << std::endl;
                            }
                            
                        } else if (userType == 2) {  // 教师
                            std::string title, department, email;
                            
                            std::cout << getText("enter_teacher_title") << "：";
                            std::getline(std::cin, title);
                            
                            std::cout << getText("enter_teacher_department") << "：";
                            std::getline(std::cin, department);
                            
                            std::cout << getText("enter_email") << "：";
                            std::getline(std::cin, email);
                            
                            // 创建教师对象
                            std::unique_ptr<Teacher> teacher = std::make_unique<Teacher>(
                                userId, name, password, department, title, email
                            );
                            
                            // 添加教师
                            if (userManager.addTeacher(std::move(teacher))) {
                                std::cout << getText("add_teacher_success") << std::endl;
                            } else {
                                std::cout << getText("add_teacher_failed") << std::endl;
                            }
                            
                        } else {  // 管理员
                            std::string email, role;
                            
                            std::cout << getText("enter_email") << "：";
                            std::getline(std::cin, email);
                            
                            std::cout << getText("enter_admin_role") << "：";
                            std::getline(std::cin, role);
                            
                            // 创建管理员对象
                            std::unique_ptr<Admin> admin = std::make_unique<Admin>(
                                userId, name, password
                            );
                            
                            // 添加管理员
                            if (userManager.addAdmin(std::move(admin))) {
                                std::cout << getText("add_admin_success") << std::endl;
                            } else {
                                std::cout << getText("add_admin_failed") << std::endl;
                            }
                        }
                        break;
                    }
                    
                    case 2: { // 删除用户
                        // 获取用户管理器
                        UserManager& userManager = UserManager::getInstance();
                        
                        // 获取用户ID
                        std::string userId;
                        std::cout << getText("enter_delete_user_id") << "：";
                        std::getline(std::cin, userId);
                        
                        // 检查用户是否存在
                        User* user = userManager.getUser(userId);
                        if (!user) {
                            std::cout << getText("user_id_not_exists") << std::endl;
                            break;
                        }
                        
                        // 确认删除
                        std::string confirm;
                        std::cout << getText("confirm_delete_user") << " \"" << user->getName() << "\" " << getText("confirm_delete_prompt") << " ";
                        std::getline(std::cin, confirm);
                        
                        if (confirm == "y" || confirm == "Y") {
                            if (userManager.removeUser(userId)) {
                                std::cout << getText("delete_user_success") << std::endl;
                            } else {
                                std::cout << getText("delete_user_failed") << std::endl;
                            }
                        } else {
                            std::cout << getText("cancel_delete") << std::endl;
                        }
                        break;
                    }
                    
                    case 3: { // 查询用户
                        // 获取用户管理器
                        UserManager& userManager = UserManager::getInstance();
                        
                        // 显示查询选项
                        std::cout << getText("select_query_method") << "：" << std::endl;
                        std::cout << "1. " << getText("query_by_user_id") << std::endl;
                        std::cout << "2. " << getText("view_all_students") << std::endl;
                        std::cout << "3. " << getText("view_all_teachers") << std::endl;
                        std::cout << "4. " << getText("view_all_admins") << std::endl;
                        std::cout << "5. " << getText("return") << std::endl;
                        
                        int queryChoice = 0;
                        std::string queryInput;
                        std::cout << "> ";
                        std::getline(std::cin, queryInput);
                        
                        if (!InputValidator::validateChoice(queryInput, 1, 5, queryChoice)) {
                            std::cout << getText("invalid_choice") << std::endl;
                            break;
                        }
                        
                        if (queryChoice == 1) {  // 按用户ID查询
                            std::string userId;
                            std::cout << getText("enter_user_id") << "：";
                            std::getline(std::cin, userId);
                            
                            User* user = userManager.getUser(userId);
                            if (!user) {
                                std::cout << getText("user_id_not_exists") << std::endl;
                                break;
                            }
                            
                            // 显示用户信息
                            std::cout << getText("user_info") << "：" << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getText("user_id") << ": " << user->getId() << std::endl;
                            std::cout << getText("user_name") << ": " << user->getName() << std::endl;
                            std::cout << getText("user_type") << ": ";
                            switch (user->getType()) {
                                case UserType::STUDENT:
                                    std::cout << getText("student");
                                    break;
                                case UserType::TEACHER:
                                    std::cout << getText("teacher");
                                    break;
                                case UserType::ADMIN:
                                    std::cout << getText("admin");
                                    break;
                                default:
                                    std::cout << getText("unknown_type");
                                    break;
                            }
                            std::cout << std::endl;
                            
                            // 根据用户类型显示不同的信息
                            if (user->getType() == UserType::STUDENT) {
                                Student* student = static_cast<Student*>(user);
                                std::cout << getText("age") << ": " << student->getAge() << std::endl;
                                std::cout << getText("department") << ": " << student->getDepartment() << std::endl;
                                std::cout << getText("class") << ": " << student->getClassInfo() << std::endl;
                                // 学生类可能没有email属性，需要检查模型类定义
                            } else if (user->getType() == UserType::TEACHER) {
                                Teacher* teacher = static_cast<Teacher*>(user);
                                std::cout << getText("title") << ": " << teacher->getTitle() << std::endl;
                                std::cout << getText("department") << ": " << teacher->getDepartment() << std::endl;
                                // 教师类可能没有email属性，需要检查模型类定义
                            } else if (user->getType() == UserType::ADMIN) {
                                // 管理员类可能没有email和role属性，需要检查模型类定义
                            }
                            std::cout << "--------------------------------" << std::endl;
                            
                        } else if (queryChoice == 2) {  // 查看所有学生
                            std::vector<std::string> studentIds = userManager.getAllStudentIds();
                            
                            if (studentIds.empty()) {
                                std::cout << getText("no_students") << std::endl;
                                break;
                            }
                            
                            std::cout << getText("student_list") << "：" << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getText("user_id") << "\t" << getText("user_name") << "\t" 
                                      << getText("department") << "\t" << getText("class") << std::endl;
                            
                            for (const std::string& studentId : studentIds) {
                                Student* student = userManager.getStudent(studentId);
                                if (student) {
                                    std::cout << student->getId() << "\t"
                                              << student->getName() << "\t"
                                              << student->getDepartment() << "\t"
                                              << student->getClassInfo() << std::endl;
                                }
                            }
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getFormattedText("student_count_total", static_cast<int>(studentIds.size())) << std::endl;
                            
                        } else if (queryChoice == 3) {  // 查看所有教师
                            std::vector<std::string> teacherIds = userManager.getAllTeacherIds();
                            
                            if (teacherIds.empty()) {
                                std::cout << getText("no_teachers") << std::endl;
                                break;
                            }
                            
                            std::cout << getText("teacher_list") << "：" << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getText("user_id") << "\t" << getText("user_name") << "\t" 
                                      << getText("title") << "\t" << getText("department") << std::endl;
                            
                            for (const std::string& teacherId : teacherIds) {
                                Teacher* teacher = userManager.getTeacher(teacherId);
                                if (teacher) {
                                    std::cout << teacher->getId() << "\t"
                                              << teacher->getName() << "\t"
                                              << teacher->getTitle() << "\t"
                                              << teacher->getDepartment() << std::endl;
                                }
                            }
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getFormattedText("teacher_count_total", static_cast<int>(teacherIds.size())) << std::endl;
                            
                        } else if (queryChoice == 4) {  // 查看所有管理员
                            std::vector<std::string> adminIds = userManager.getAllAdminIds();
                            
                            if (adminIds.empty()) {
                                std::cout << getText("no_admins") << std::endl;
                                break;
                            }
                            
                            std::cout << getText("admin_list") << "：" << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getText("user_id") << "\t" << getText("user_name") << "\t" 
                                      << getText("role") << std::endl;
                            
                            for (const std::string& adminId : adminIds) {
                                Admin* admin = userManager.getAdmin(adminId);
                                if (admin) {
                                    std::cout << admin->getId() << "\t"
                                              << admin->getName() << "\t"
                                              << getText("admin") << std::endl;
                                }
                            }
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getFormattedText("admin_count_total", static_cast<int>(adminIds.size())) << std::endl;
                        }
                        break;
                    }
                    
                    case 4: // 返回上级菜单
                        subMenuRunning = false;
                        break;
                }
                
                if (subMenuRunning && subChoice != 4) {
                    std::cout << getText("press_enter_to_continue") << std::endl;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            }
            break;
        }
            
        case 2: { // 课程管理
            std::cout << getText("course_management_function") << std::endl;
            // 实现更完整的课程管理功能
            bool subMenuRunning = true;
            while (subMenuRunning && running_) {
                std::cout << "1. " << getText("add_course") << std::endl;
                std::cout << "2. " << getText("delete_course") << std::endl;
                std::cout << "3. " << getText("modify_course") << std::endl;
                std::cout << "4. " << getText("query_course") << std::endl;
                std::cout << "5. " << getText("return_to_parent_menu") << std::endl;
                
                int subChoice = 0;
                std::string input;
                std::cout << "> ";
                std::getline(std::cin, input);
                
                if (!InputValidator::validateChoice(input, 1, 5, subChoice)) {
                    std::cout << getText("invalid_input") << std::endl;
                    continue;
                }
                
                switch (subChoice) {
                    case 1: { // 添加课程
                        // 获取课程管理器
                        CourseManager& courseManager = CourseManager::getInstance();
                        
                        // 收集课程信息
                        std::string courseId, name, typeStr, creditStr, hoursStr, semester, teacherId, maxCapacityStr;
                        
                        std::cout << getText("enter_course_id") << "：";
                        std::getline(std::cin, courseId);
                        
                        // 检查课程ID是否已存在
                        if (courseManager.hasCourse(courseId)) {
                            std::cout << getText("course_id_exists") << std::endl;
                            break;
                        }
                        
                        std::cout << getText("enter_course_name") << "：";
                        std::getline(std::cin, name);
                        
                        // 选择课程类型
                        std::cout << getText("select_course_type") << "：" << std::endl;
                        std::cout << "1. " << getText("required_course") << std::endl;
                        std::cout << "2. " << getText("elective_course") << std::endl;
                        
                        int typeChoice = 0;
                        std::string typeInput;
                        std::cout << "> ";
                        std::getline(std::cin, typeInput);
                        
                        if (!InputValidator::validateChoice(typeInput, 1, 2, typeChoice)) {
                            std::cout << getText("invalid_course_type") << std::endl;
                            break;
                        }
                        
                        CourseType type;
                        switch (typeChoice) {
                            case 1:
                                type = CourseType::REQUIRED;
                                break;
                            case 2:
                                type = CourseType::ELECTIVE;
                                break;
                            default:
                                type = CourseType::ELECTIVE;
                                break;
                        }
                        
                        std::cout << getText("enter_credit") << "：";
                        std::getline(std::cin, creditStr);
                        
                        double credit = 0.0;
                        if (!InputValidator::validateDouble(creditStr, 0.0, 10.0, credit)) {
                            std::cout << getText("invalid_credit") << std::endl;
                            break;
                        }
                        
                        std::cout << getText("enter_hours") << "：";
                        std::getline(std::cin, hoursStr);
                        
                        int hours = 0;
                        if (!InputValidator::validateInteger(hoursStr, 0, 200, hours)) {
                            std::cout << getText("invalid_hours") << std::endl;
                            break;
                        }
                        
                        std::cout << getText("enter_semester") << "：";
                        std::getline(std::cin, semester);
                        
                        std::cout << getText("enter_teacher_id") << "：";
                        std::getline(std::cin, teacherId);
                        
                        // 验证教师ID是否存在
                        UserManager& userManager = UserManager::getInstance();
                        if (!userManager.getTeacher(teacherId)) {
                            std::cout << getText("teacher_id_not_exists") << std::endl;
                            break;
                        }
                        
                        std::cout << getText("enter_max_capacity") << "：";
                        std::getline(std::cin, maxCapacityStr);
                        
                        int maxCapacity = 0;
                        if (!InputValidator::validateInteger(maxCapacityStr, 1, 1000, maxCapacity)) {
                            std::cout << getText("invalid_max_capacity") << std::endl;
                            break;
                        }
                        
                        try {
                            // 创建课程对象
                            std::unique_ptr<Course> course = std::make_unique<Course>(
                                courseId, name, type, credit, hours, 
                                semester, teacherId, maxCapacity
                            );
                            
                            // 添加课程
                            if (courseManager.addCourse(std::move(course))) {
                                std::cout << getText("add_course_success") << std::endl;
                                // 保存数据
                                courseManager.saveData();
                            } else {
                                std::cout << getText("add_course_failed") << std::endl;
                            }
                        } catch (const std::exception& e) {
                            std::cout << getText("adding_course_error") << ": " << e.what() << std::endl;
                        }
                        break;
                    }
                    case 2: { // 删除课程
                        // 获取课程管理器
                        CourseManager& courseManager = CourseManager::getInstance();
                        EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
                        
                        // 获取课程ID
                        std::string courseId;
                        std::cout << getText("enter_delete_course_id") << "：";
                        std::getline(std::cin, courseId);
                        
                        // 检查课程是否存在
                        Course* course = courseManager.getCourse(courseId);
                        if (!course) {
                            std::cout << getText("course_id_not_exists") << std::endl;
                            break;
                        }
                        
                        // 显示课程信息
                        std::cout << getText("course_to_delete") << "：" << std::endl;
                        std::cout << getText("course_id") << ": " << course->getId() << std::endl;
                        std::cout << getText("course_name") << ": " << course->getName() << std::endl;
                        std::cout << getText("course_type") << ": " << course->getTypeString() << std::endl;
                        std::cout << getText("credit") << ": " << course->getCredit() << std::endl;
                        std::cout << getText("current_enrollment") << ": " << course->getCurrentEnrollment() << std::endl;
                        
                        // 检查课程是否已有学生选修
                        if (course->getCurrentEnrollment() > 0) {
                            std::cout << getText("course_has_students") << std::endl;
                        }
                        
                        // 确认删除
                        std::string confirm;
                        std::cout << getText("confirm_delete_course") << " \"" << course->getName() << "\" " << getText("confirm_delete_prompt") << " ";
                        std::getline(std::cin, confirm);
                        
                        if (confirm == "y" || confirm == "Y") {
                            // 获取选修该课程的学生列表
                            std::vector<Enrollment*> enrollments = enrollmentManager.getCourseEnrollments(courseId);
                            
                            // 先处理选课记录
                            for (Enrollment* enrollment : enrollments) {
                                enrollmentManager.dropCourse(enrollment->getStudentId(), courseId);
                            }
                            
                            // 删除课程
                            if (courseManager.removeCourse(courseId)) {
                                std::cout << getText("delete_course_success") << std::endl;
                                // 保存数据
                                courseManager.saveData();
                                enrollmentManager.saveData();
                            } else {
                                std::cout << getText("delete_course_failed") << std::endl;
                            }
                        } else {
                            std::cout << getText("cancel_delete") << std::endl;
                        }
                        break;
                    }
                    case 3: { // 修改课程
                        // 获取课程管理器
                        CourseManager& courseManager = CourseManager::getInstance();
                        
                        // 获取课程ID
                        std::string courseId;
                        std::cout << "请输入要修改的课程ID：";
                        std::getline(std::cin, courseId);
                        
                        // 检查课程是否存在
                        Course* course = courseManager.getCourse(courseId);
                        if (!course) {
                            std::cout << "课程ID不存在！" << std::endl;
                            break;
                        }
                        
                        // 显示当前课程信息
                        std::cout << getText("current_course_info") << "：" << std::endl;
                        std::cout << getText("course_id") << ": " << course->getId() << std::endl;
                        std::cout << getText("course_name") << ": " << course->getName() << std::endl;
                        std::cout << getText("course_type") << ": " << course->getTypeString() << std::endl;
                        std::cout << getText("credit") << ": " << course->getCredit() << std::endl;
                        std::cout << getText("hours") << ": " << course->getHours() << std::endl;
                        std::cout << getText("semester") << ": " << course->getSemester() << std::endl;
                        std::cout << getText("teacher_id") << ": " << course->getTeacherId() << std::endl;
                        std::cout << getText("max_capacity") << ": " << course->getMaxCapacity() << std::endl;
                        std::cout << getText("current_enrollment") << ": " << course->getCurrentEnrollment() << std::endl;
                        
                        // 显示修改选项
                        std::cout << getText("select_modify_course_content") << "：" << std::endl;
                        std::cout << "1. " << getText("modify_course_name") << std::endl;
                        std::cout << "2. " << getText("modify_course_type") << std::endl;
                        std::cout << "3. " << getText("modify_course_credit") << std::endl;
                        std::cout << "4. " << getText("modify_course_hours") << std::endl;
                        std::cout << "5. " << getText("modify_course_semester") << std::endl;
                        std::cout << "6. " << getText("modify_teacher_id") << std::endl;
                        std::cout << "7. " << getText("modify_max_capacity") << std::endl;
                        std::cout << "8. " << getText("return") << std::endl;
                        
                        int modifyChoice = 0;
                        std::string modifyInput;
                        std::cout << "> ";
                        std::getline(std::cin, modifyInput);
                        
                        if (!InputValidator::validateChoice(modifyInput, 1, 8, modifyChoice)) {
                            std::cout << getText("invalid_choice") << std::endl;
                            break;
                        }
                        
                        // 处理不同的修改选项
                        switch (modifyChoice) {
                            case 1: { // 修改课程名称
                                std::string newName;
                                std::cout << getText("enter_new_course_name") << "：";
                                std::getline(std::cin, newName);
                                
                                course->setName(newName);
                                std::cout << getText("course_name_modify_success") << std::endl;
                                break;
                            }
                            case 2: { // 修改课程类型
                                std::cout << getText("select_new_course_type") << "：" << std::endl;
                                std::cout << "1. " << getText("required_course") << std::endl;
                                std::cout << "2. " << getText("elective_course") << std::endl;
                                
                                int typeChoice = 0;
                                std::string typeInput;
                                std::cout << "> ";
                                std::getline(std::cin, typeInput);
                                
                                if (!InputValidator::validateChoice(typeInput, 1, 2, typeChoice)) {
                                    std::cout << getText("invalid_course_type") << std::endl;
                                    break;
                                }
                                
                                CourseType type;
                                switch (typeChoice) {
                                    case 1:
                                        type = CourseType::REQUIRED;
                                        break;
                                    case 2:
                                        type = CourseType::ELECTIVE;
                                        break;
                                    default:
                                        type = CourseType::ELECTIVE;
                                        break;
                                }
                                
                                course->setType(type);
                                std::cout << getText("course_type_modify_success") << std::endl;
                                break;
                            }
                            case 3: { // 修改课程学分
                                std::string creditStr;
                                std::cout << getText("enter_new_credit") << "：";
                                std::getline(std::cin, creditStr);
                                
                                double credit = 0.0;
                                if (!InputValidator::validateDouble(creditStr, 0.0, 10.0, credit)) {
                                    std::cout << getText("invalid_credit") << std::endl;
                                    break;
                                }
                                
                                course->setCredit(credit);
                                std::cout << getText("course_credit_modify_success") << std::endl;
                                break;
                            }
                            case 4: { // 修改课程学时
                                std::string hoursStr;
                                std::cout << getText("enter_new_hours") << "：";
                                std::getline(std::cin, hoursStr);
                                
                                int hours = 0;
                                if (!InputValidator::validateInteger(hoursStr, 0, 200, hours)) {
                                    std::cout << getText("invalid_hours") << std::endl;
                                    break;
                                }
                                
                                course->setHours(hours);
                                std::cout << getText("course_hours_modify_success") << std::endl;
                                break;
                            }
                            case 5: { // 修改课程学期
                                std::string newSemester;
                                std::cout << getText("enter_new_semester") << "：";
                                std::getline(std::cin, newSemester);
                                
                                course->setSemester(newSemester);
                                std::cout << getText("course_semester_modify_success") << std::endl;
                                break;
                            }
                            case 6: { // 修改教师ID
                                std::string newTeacherId;
                                std::cout << getText("enter_new_teacher_id") << "：";
                                std::getline(std::cin, newTeacherId);
                                
                                // 验证教师ID是否存在
                                UserManager& userManager = UserManager::getInstance();
                                if (!userManager.getTeacher(newTeacherId)) {
                                    std::cout << getText("teacher_id_not_exists") << std::endl;
                                    break;
                                }
                                
                                course->setTeacherId(newTeacherId);
                                std::cout << getText("teacher_id_modify_success") << std::endl;
                                break;
                            }
                            case 7: { // 修改最大容量
                                std::string maxCapacityStr;
                                std::cout << getText("enter_new_max_capacity") << "：";
                                std::getline(std::cin, maxCapacityStr);
                                
                                int maxCapacity = 0;
                                if (!InputValidator::validateInteger(maxCapacityStr, 1, 1000, maxCapacity)) {
                                    std::cout << getText("invalid_max_capacity") << std::endl;
                                    break;
                                }
                                
                                // 检查当前选课人数是否超过新的最大容量
                                if (course->getCurrentEnrollment() > maxCapacity) {
                                    std::cout << getText("capacity_lt_enrollment") << std::endl;
                                    break;
                                }
                                
                                course->setMaxCapacity(maxCapacity);
                                std::cout << getText("max_capacity_modify_success") << std::endl;
                                break;
                            }
                            case 8: // 返回
                                break;
                        }
                        
                        // 保存数据
                        if (modifyChoice >= 1 && modifyChoice <= 7) {
                            courseManager.saveData();
                        }
                        break;
                    }
                    case 4: { // 查询课程
                        // 获取课程管理器
                        CourseManager& courseManager = CourseManager::getInstance();
                        
                        // 显示查询选项
                        std::cout << getText("select_query_method") << "：" << std::endl;
                        std::cout << "1. " << getText("view_all_courses") << std::endl;
                        std::cout << "2. " << getText("query_by_course_id") << std::endl;
                        std::cout << "3. " << getText("query_by_course_name") << std::endl;
                        std::cout << "4. " << getText("query_by_teacher") << std::endl;
                        std::cout << "5. " << getText("query_by_course_type") << std::endl;
                        std::cout << "6. " << getText("return") << std::endl;
                        
                        int queryChoice = 0;
                        std::string queryInput;
                        std::cout << "> ";
                        std::getline(std::cin, queryInput);
                        
                        if (!InputValidator::validateChoice(queryInput, 1, 6, queryChoice)) {
                            std::cout << getText("invalid_choice") << std::endl;
                            break;
                        }
                        
                        std::vector<std::string> courseIds;
                        
                        switch (queryChoice) {
                            case 1: // 查看所有课程
                                courseIds = courseManager.getAllCourseIds();
                                break;
                            case 2: { // 按课程ID查询
                                std::cout << getText("enter_course_id") << "：";
                                std::string courseId;
                                std::getline(std::cin, courseId);
                                
                                if (courseManager.hasCourse(courseId)) {
                                    courseIds.push_back(courseId);
                                }
                                break;
                            }
                            case 3: { // 按课程名称查询
                                std::cout << getText("enter_course_name") << "：";
                                std::string courseName;
                                std::getline(std::cin, courseName);
                                
                                courseIds = courseManager.findCourses(
                                    [courseName](const Course& c) { 
                                        return c.getName().find(courseName) != std::string::npos; 
                                    }
                                );
                                break;
                            }
                            case 4: { // 按教师查询
                                std::cout << getText("enter_teacher_id") << "：";
                                std::string teacherId;
                                std::getline(std::cin, teacherId);
                                
                                courseIds = courseManager.findCourses(
                                    [teacherId](const Course& c) { return c.getTeacherId() == teacherId; }
                                );
                                break;
                            }
                            case 5: { // 按课程类型查询
                                std::cout << getText("select_course_type") << "：" << std::endl;
                                std::cout << "1. " << getText("required_course") << std::endl;
                                std::cout << "2. " << getText("elective_course") << std::endl;
                                
                                int typeChoice = 0;
                                std::string typeInput;
                                std::cout << "> ";
                                std::getline(std::cin, typeInput);
                                
                                if (!InputValidator::validateChoice(typeInput, 1, 2, typeChoice)) {
                                    std::cout << getText("invalid_course_type") << std::endl;
                                    break;
                                }
                                
                                CourseType type;
                                switch (typeChoice) {
                                    case 1:
                                        type = CourseType::REQUIRED;
                                        break;
                                    case 2:
                                        type = CourseType::ELECTIVE;
                                        break;
                                    default:
                                        type = CourseType::ELECTIVE;
                                        break;
                                }
                                
                                courseIds = courseManager.findCourses(
                                    [type](const Course& c) { return c.getType() == type; }
                                );
                                break;
                            }
                            case 6: // 返回
                                break;
                        }
                        
                        // 显示查询结果
                        if (queryChoice != 6) {
                            if (courseIds.empty()) {
                                std::cout << getText("no_courses") << std::endl;
                            } else {
                                std::cout << getText("query_result") << "：" << std::endl;
                                std::cout << "--------------------------------" << std::endl;
                                std::cout << "课程ID\t课程名称\t类型\t学分\t学时\t教师ID\t容量\t已选人数" << std::endl;
                                
                                for (const std::string& courseId : courseIds) {
                                    Course* course = courseManager.getCourse(courseId);
                                    if (course) {
                                        std::cout << course->getId() << "\t"
                                                << course->getName() << "\t"
                                                << course->getTypeString() << "\t"
                                                << course->getCredit() << "\t"
                                                << course->getHours() << "\t"
                                                << course->getTeacherId() << "\t"
                                                << course->getMaxCapacity() << "\t"
                                                << course->getCurrentEnrollment() << std::endl;
                                    }
                                }
                                                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getFormattedText("course_count_total", static_cast<int>(courseIds.size())) << std::endl;
                            }
                            
                            std::cout << getText("press_enter_to_continue") << std::endl;
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        }
                        break;
                    }
                    case 5: // 返回上级菜单
                        subMenuRunning = false;
                        break;
                }
            }
            break;
        }
            
        case 3: { // 选课查询
            std::cout << getText("enrollment_query_function") << std::endl;
            
            // 获取选课管理器和课程管理器
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            CourseManager& courseManager = CourseManager::getInstance();
            
            bool subMenuRunning = true;
            while (subMenuRunning && running_) {
                std::cout << "1. " << getText("query_by_student") << std::endl;
                std::cout << "2. " << getText("query_by_course") << std::endl;
                std::cout << "3. " << getText("return_to_parent_menu") << std::endl;
                
                int subChoice = 0;
                std::string input;
                std::cout << "> ";
                std::getline(std::cin, input);
                
                if (!InputValidator::validateChoice(input, 1, 3, subChoice)) {
                    std::cout << getText("invalid_input") << std::endl;
                    continue;
                }
                
                switch (subChoice) {
                    case 1: { // 按学生查询
                        std::string studentId;
                        std::cout << getText("enter_user_id_prompt") << "：";
                        std::getline(std::cin, studentId);
                        
                        // 获取学生的所有选课记录
                        std::vector<Enrollment*> enrollments = enrollmentManager.getStudentEnrollments(studentId);
                        
                        if (enrollments.empty()) {
                            std::cout << getText("no_selected_courses") << std::endl;
                        } else {
                            std::cout << getFormattedText("student_selected_courses", studentId) << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << "课程ID\t课程名称\t学分\t教师ID\t选课时间" << std::endl;
                            
                            for (Enrollment* enrollment : enrollments) {
                                std::string courseId = enrollment->getCourseId();
                                Course* course = courseManager.getCourse(courseId);
                                
                                if (course) {
                                    std::cout << course->getId() << "\t"
                                              << course->getName() << "\t"
                                              << course->getCredit() << "\t"
                                              << course->getTeacherId() << "\t"
                                              << enrollment->getEnrollmentTime() << std::endl;
                                }
                            }
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getFormattedText("selected_courses_count", static_cast<int>(enrollments.size())) << std::endl;
                        }
                        
                        std::cout << getText("press_enter_to_continue") << std::endl;
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        break;
                    }
                    case 2: { // 按课程查询
                        std::string courseId;
                        std::cout << "请输入课程ID：";
                        std::getline(std::cin, courseId);
                        
                        // 获取该课程的所有选课记录
                        std::vector<Enrollment*> enrollments = enrollmentManager.getCourseEnrollments(courseId);
                        
                        if (enrollments.empty()) {
                            std::cout << "该课程还没有学生选修。" << std::endl;
                        } else {
                            std::cout << getFormattedText("course_students", courseId) << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << "学生ID\t姓名\t班级\t专业" << std::endl;
                            
                            UserManager& userManager = UserManager::getInstance();
                            for (Enrollment* enrollment : enrollments) {
                                std::string studentId = enrollment->getStudentId();
                                Student* student = static_cast<Student*>(userManager.getStudent(studentId));
                                
                                if (student) {
                                    std::cout << student->getId() << "\t"
                                              << student->getName() << "\t"
                                              << student->getClassInfo() << "\t"
                                              << student->getDepartment() << std::endl;
                                }
                            }
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getFormattedText("enrolled_student_count", static_cast<int>(enrollments.size())) << std::endl;
                        }
                        
                        std::cout << getText("press_enter_to_continue") << std::endl;
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        break;
                    }
                    case 3: // 返回上级菜单
                        subMenuRunning = false;
                        break;
                }
            }
            break;
        }
            
        default:
            break;
    }
}

void CourseSystem::handleTeacherFunctions(int choice) {
    // 这里只是示例，实际应该实现完整的教师功能
    switch (choice) {
        case 1: { // 查看课程
            std::cout << getText("view_courses_function") << std::endl;
            // 实现更完整的查看课程功能
            
            // 获取教师ID
            std::string teacherId = currentUser_->getId();
            
            // 获取课程管理器
            CourseManager& courseManager = CourseManager::getInstance();
            
            // 查询该教师的所有课程
            std::vector<std::string> teacherCourseIds = courseManager.findCourses(
                [teacherId](const Course& c) { return c.getTeacherId() == teacherId; }
            );
            
            // 显示课程列表
            if (teacherCourseIds.empty()) {
                std::cout << "您还没有任教的课程。" << std::endl;
            } else {
                std::cout << "您的课程列表：" << std::endl;
                std::cout << "--------------------------------" << std::endl;
                std::cout << "课程ID\t课程名称\t学分\t课时\t学期\t容量" << std::endl;
                
                for (const std::string& courseId : teacherCourseIds) {
                    Course* course = courseManager.getCourse(courseId);
                    if (course) {
                        std::cout << course->getId() << "\t"
                                  << course->getName() << "\t"
                                  << course->getCredit() << "\t"
                                  << course->getHours() << "\t"
                                  << course->getSemester() << "\t"
                                  << course->getMaxCapacity() << std::endl;
                    }
                }
                std::cout << "--------------------------------" << std::endl;
            }
            
            std::cout << getText("press_enter_to_continue") << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
            
        case 2: { // 查看学生
            std::cout << getText("view_students_function") << std::endl;
            
            // 获取教师ID
            std::string teacherId = currentUser_->getId();
            
            // 获取课程和选课管理器
            CourseManager& courseManager = CourseManager::getInstance();
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            UserManager& userManager = UserManager::getInstance();
            
            // 查询该教师的所有课程
            std::vector<std::string> teacherCourseIds = courseManager.findCourses(
                [teacherId](const Course& c) { return c.getTeacherId() == teacherId; }
            );
            
            if (teacherCourseIds.empty()) {
                std::cout << "您还没有任教的课程。" << std::endl;
            } else {
                // 先列出所有课程
                std::cout << "您的课程列表：" << std::endl;
                for (std::size_t i = 0; i < teacherCourseIds.size(); ++i) {
                    Course* course = courseManager.getCourse(teacherCourseIds[i]);
                    if (course) {
                        std::cout << (i+1) << ". " << course->getId() << " - " << course->getName() << std::endl;
                    }
                }
                
                // 选择要查看的课程
                int courseIndex = 0;
                std::string input;
                do {
                    std::cout << "请选择要查看的课程(1-" << teacherCourseIds.size() << ")，或输入0返回：";
                    std::getline(std::cin, input);
                    
                    if (input == "0") {
                        return; // 用户选择返回
                    }
                    
                } while (!InputValidator::validateChoice(input, 1, teacherCourseIds.size(), courseIndex));
                
                // 获取选定的课程
                std::string selectedCourseId = teacherCourseIds[courseIndex-1];
                Course* selectedCourse = courseManager.getCourse(selectedCourseId);
                
                if (selectedCourse) {
                    // 获取该课程的所有选课记录
                    std::vector<Enrollment*> enrollments = enrollmentManager.getCourseEnrollments(selectedCourseId);
                    
                    if (enrollments.empty()) {
                        std::cout << "该课程还没有学生选修。" << std::endl;
                    } else {
                        std::cout << getFormattedText("course_students", selectedCourse->getName()) << std::endl;
                        std::cout << "--------------------------------" << std::endl;
                        std::cout << "学生ID\t姓名\t班级\t专业" << std::endl;
                        
                        for (Enrollment* enrollment : enrollments) {
                            std::string studentId = enrollment->getStudentId();
                            Student* student = static_cast<Student*>(userManager.getStudent(studentId));
                            
                            if (student) {
                                std::cout << student->getId() << "\t"
                                          << student->getName() << "\t"
                                          << student->getClassInfo() << "\t"
                                          << student->getDepartment() << std::endl;
                            }
                        }
                        std::cout << "--------------------------------" << std::endl;
                    }
                }
            }
            
            std::cout << getText("press_enter_to_continue") << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
            
        default:
            break;
    }
}

void CourseSystem::handleStudentFunctions(int choice) {
    // 这里只是示例，实际应该实现完整的学生功能
    switch (choice) {
        case 1: { // 查询课程
            std::cout << getText("query_courses_function") << std::endl;
            
            // 获取课程管理器
            CourseManager& courseManager = CourseManager::getInstance();
            
            // 显示查询选项
            bool subMenuRunning = true;
            while (subMenuRunning && running_) {
                std::cout << "1. " << getText("view_all_courses") << std::endl;
                std::cout << "2. " << getText("query_by_course_id") << std::endl;
                std::cout << "3. " << getText("query_by_course_name") << std::endl;
                std::cout << "4. " << getText("query_by_teacher") << std::endl;
                std::cout << "5. " << getText("return_to_parent_menu") << std::endl;
                
                int subChoice = 0;
                std::string input;
                std::cout << "> ";
                std::getline(std::cin, input);
                
                if (!InputValidator::validateChoice(input, 1, 5, subChoice)) {
                    std::cout << getText("invalid_input") << std::endl;
                    continue;
                }
                
                std::vector<std::string> courseIds;
                
                switch (subChoice) {
                    case 1: // 查看所有课程
                        courseIds = courseManager.getAllCourseIds();
                        break;
                    case 2: { // 按课程ID查询
                        std::cout << "请输入课程ID：";
                        std::string courseId;
                        std::getline(std::cin, courseId);
                        
                        if (courseManager.hasCourse(courseId)) {
                            courseIds.push_back(courseId);
                        }
                        break;
                    }
                    case 3: { // 按课程名称查询
                        std::cout << "请输入课程名称（可以是部分名称）：";
                        std::string courseName;
                        std::getline(std::cin, courseName);
                        
                        courseIds = courseManager.findCourses(
                            [courseName](const Course& c) { 
                                return c.getName().find(courseName) != std::string::npos; 
                            }
                        );
                        break;
                    }
                    case 4: { // 按教师查询
                        std::cout << "请输入教师ID：";
                        std::string teacherId;
                        std::getline(std::cin, teacherId);
                        
                        courseIds = courseManager.findCourses(
                            [teacherId](const Course& c) { return c.getTeacherId() == teacherId; }
                        );
                        break;
                    }
                    case 5: // 返回上级菜单
                        subMenuRunning = false;
                        continue;
                }
                
                // 显示查询结果
                if (courseIds.empty()) {
                    std::cout << "未找到符合条件的课程。" << std::endl;
                } else {
                    std::cout << "查询结果：" << std::endl;
                    std::cout << "--------------------------------" << std::endl;
                    std::cout << "课程ID\t课程名称\t学分\t课时\t教师ID\t容量" << std::endl;
                    
                    for (const std::string& courseId : courseIds) {
                        Course* course = courseManager.getCourse(courseId);
                        if (course) {
                            std::cout << course->getId() << "\t"
                                      << course->getName() << "\t"
                                      << course->getCredit() << "\t"
                                      << course->getHours() << "\t"
                                      << course->getTeacherId() << "\t"
                                      << course->getMaxCapacity() << std::endl;
                        }
                    }
                    std::cout << "--------------------------------" << std::endl;
                }
                
                std::cout << getText("press_enter_to_continue") << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            break;
        }
            
        case 2: { // 选择课程
            std::cout << getText("select_course_function") << std::endl;
            
            std::string studentId = currentUser_->getId();
            
            // 获取课程ID
            std::cout << getText("query_by_course_id") << ": ";
            std::string courseId;
            std::getline(std::cin, courseId);
            
            // 获取选课管理器
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            
            // 尝试选课
            try {
                if (enrollmentManager.enrollCourse(studentId, courseId)) {
                    std::cout << getText("operation_success") << std::endl;
                } else {
                    std::cout << getText("operation_failed") << std::endl;
                }
            } catch (const SystemException& e) {
                std::cout << getText("operation_failed") << ": " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cout << getText("system_error") << ": " << e.what() << std::endl;
            }
            
            std::cout << getText("press_enter_to_continue") << "..." << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
            
        case 3: { // 退选课程
            std::cout << getText("drop_course_function") << std::endl;
            
            std::string studentId = currentUser_->getId();
            
            // 获取课程ID
            std::cout << getText("query_by_course_id") << ": ";
            std::string courseId;
            std::getline(std::cin, courseId);
            
            // 获取选课管理器
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            
            // 尝试退课
            try {
                if (enrollmentManager.dropCourse(studentId, courseId)) {
                    std::cout << getText("operation_success") << std::endl;
                } else {
                    std::cout << getText("operation_failed") << std::endl;
                }
            } catch (const SystemException& e) {
                std::cout << getText("operation_failed") << ": " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cout << getText("system_error") << ": " << e.what() << std::endl;
            }
            
            std::cout << getText("press_enter_to_continue") << "..." << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
            
        case 4: { // 查看已选课程
            std::cout << getText("view_selected_courses_function") << std::endl;
            
            std::string studentId = currentUser_->getId();
            
            // 获取选课管理器和课程管理器
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            CourseManager& courseManager = CourseManager::getInstance();
            
            // 获取学生的所有选课记录
            std::vector<Enrollment*> enrollments = enrollmentManager.getStudentEnrollments(studentId);
            
            if (enrollments.empty()) {
                std::cout << getText("operation_failed") << std::endl;
            } else {
                std::cout << getText("view_selected_courses") << "：" << std::endl;
                std::cout << "--------------------------------" << std::endl;
                std::cout << getText("course_id") << "\t" 
                          << getText("course_name") << "\t" 
                          << getText("credit") << "\t" 
                          << getText("teacher_id") << "\t"
                          << getText("enrollment_time") << std::endl;
                
                for (Enrollment* enrollment : enrollments) {
                    std::string courseId = enrollment->getCourseId();
                    Course* course = courseManager.getCourse(courseId);
                    
                    if (course) {
                        std::cout << course->getId() << "\t"
                                  << course->getName() << "\t"
                                  << course->getCredit() << "\t"
                                  << course->getTeacherId() << "\t"
                                  << enrollment->getEnrollmentTime() << std::endl;
                    }
                }
                std::cout << "--------------------------------" << std::endl;
                std::cout << getFormattedText("enrollment_count_total", static_cast<int>(enrollments.size())) << std::endl;
            }
            
            std::cout << getText("press_enter_to_continue") << "..." << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
            
        default:
            break;
    }
}

bool CourseSystem::changePassword(const std::string& userId, const std::string& oldPassword,
                                 const std::string& newPassword, const std::string& confirmPassword) {
    try {
        // 权限检查：用户只能修改自己的密码
        if (currentUser_ == nullptr) {
            Logger::getInstance().warning("修改密码失败：用户未登录");
            return false;
        }
        
        // 检查是否是用户修改自己的密码
        if (currentUser_->getId() != userId) {
            Logger::getInstance().warning("用户 " + currentUser_->getId() + " 尝试修改其他用户 " + userId + " 的密码，权限不足");
            return false;
        }
        
        // 检查新密码与确认密码是否一致
        if (newPassword != confirmPassword) {
            Logger::getInstance().warning("用户 " + userId + " 修改密码失败：新密码与确认密码不一致");
            return false;
        }
        
        // 密码有效性检查 - 密码长度必须大于等于6位
        if (newPassword.length() < 6) {
            Logger::getInstance().warning("用户 " + userId + " 修改密码失败：新密码长度不足6位");
            return false;
        }
        
        // 调用UserManager修改密码
        bool result = UserManager::getInstance().changeUserPassword(userId, oldPassword, newPassword);
        
        if (result) {
            Logger::getInstance().info("用户 " + userId + " 密码修改成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::getInstance().error("修改密码出现异常: " + std::string(e.what()));
        return false;
    } catch (...) {
        Logger::getInstance().error("修改密码出现未知异常");
        return false;
    }
}

// 添加在其他函数实现之后，比如在changePassword方法后面

void CourseSystem::handlePasswordChange() {
    if (!currentUser_) {
        std::cout << getText("operation_failed") << ": " << getText("password_change_failed") << std::endl;
        return;
    }
    
    std::string userId = currentUser_->getId();
    std::string oldPassword, newPassword, confirmPassword;
    
    std::cout << getText("change_password") << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    std::cout << getText("old_password") << ": ";
    std::getline(std::cin, oldPassword);
    
    std::cout << getText("new_password") << "（" << getText("password_min_length") << "）: ";
    std::getline(std::cin, newPassword);
    
    std::cout << getText("confirm_password") << ": ";
    std::getline(std::cin, confirmPassword);
    
    if (changePassword(userId, oldPassword, newPassword, confirmPassword)) {
        std::cout << getText("password_change_success") << std::endl;
    } else {
        std::cout << getText("password_change_failed") << std::endl;
    }
    
    std::cout << getText("press_enter_to_continue") << "..." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 显式实例化常用的模板函数，解决链接错误
template std::string CourseSystem::getFormattedText(const std::string& key, int) const;
template std::string CourseSystem::getFormattedText(const std::string& key, const std::string&) const;
template std::string CourseSystem::getFormattedText(const std::string& key, const char*) const;
