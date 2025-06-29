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
    static CourseSystem instance; // Meyer's单例模式
    return instance;
}

CourseSystem::CourseSystem()
    : initialized_(false),
      running_(false),
      currentUser_(nullptr) {
}

bool CourseSystem::initialize(const std::string& dataDir) {
    try {
        
        // 初始化国际化管理器
        I18nManager& i18n = I18nManager::getInstance();
        if (!i18n.initialize(dataDir)) {
            Logger::getInstance().critical("初始化国际化系统失败");
            return false;
        }
  
        // 初始化数据管理器
        DataManager& dataManager = DataManager::getInstance();
        dataManager.setDataDirectory(dataDir);
        
        try {
            // 加载用户数据
            UserManager& userManager = UserManager::getInstance();
            bool userDataLoaded = userManager.loadData();
            
            if (!userDataLoaded) {
                Logger::getInstance().warning("用户数据加载失败");
            }
            
            // 加载课程数据
            CourseManager& courseManager = CourseManager::getInstance();
            bool courseDateLoaded = courseManager.loadData();
            
            if (!courseDateLoaded) {
                Logger::getInstance().warning("课程数据加载失败");
            }
            
            // 加载选课数据
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            bool enrollmentDataLoaded = enrollmentManager.loadData();
            
            if (!enrollmentDataLoaded) {
                Logger::getInstance().warning("选课数据加载失败");
            }
            
            initialized_ = true;
            
            Logger::getInstance().info("系统初始化成功");
            return true;
        } catch (const std::exception& e) {
            Logger::getInstance().error("初始化CourseSystem时发生异常: " + std::string(e.what()));
            return false;
        }
    } catch (const std::exception& e) {
        Logger::getInstance().critical("初始化CourseSystem时发生严重异常: " + std::string(e.what()));
        return false;
    } catch (...) {
        Logger::getInstance().critical("初始化CourseSystem时发生未知异常");
        return false;
    }
}

int CourseSystem::run() {
    if (!initialized_) {
        Logger::getInstance().critical("系统未初始化");
        return -1;
    }
    
    running_ = true;
    
    // 显示欢迎界面
    showWelcome();
    
    try {
        // 选择语言
        std::cout << "================================================" << std::endl;
        std::cout << "         请选择语言 / Please select language     " << std::endl;
        std::cout << "================================================" << std::endl;
        std::cout << "1. 中文 / Chinese" << std::endl;
        std::cout << "2. English / 英文" << std::endl;
        std::cout << "3. " << "退出 / Exit" << std::endl;

        int choice = 0;
        std::string input;
        int attempts = 0;
        const int MAX_ATTEMPTS = 3; // 最大尝试次数
        
        do {
            std::cout << "> ";
            std::getline(std::cin, input);
            attempts++;
            
            if (input.empty()) {
                std::cout << "输入为空，请输入数字 1-3 / Empty input, please enter a number 1-3" << std::endl;
                continue;
            }
            
            if (input == "1" || input == "2" || input == "3") {
                choice = std::stoi(input);
                break;
            } else {
                std::cout << "输入无效，请输入数字 1-3 / Invalid input, please enter a number 1-3" << std::endl;
            }    
            if (attempts >= MAX_ATTEMPTS) {
                std::cout << "多次输入无效，默认选择退出 / Multiple invalid inputs, defaulting to exit" << std::endl;
                choice = 3; // 默认选择退出
                break;
            }
        } while (true);
        
        bool result = false;
        // 设置语言
        switch (choice) {
            case 1: // Language::CHINESE
                result = I18nManager::getInstance().setLanguage(Language::CHINESE);
                break;
            case 2: // Language::ENGLISH
                result = I18nManager::getInstance().setLanguage(Language::ENGLISH);
                break;
            case 3:
                return -1;
        }
        if (!result) {
            Logger::getInstance().critical("语言设置失败");
            return -1;
        }
        
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
        Logger::getInstance().error("运行时异常: " + std::string(e.what()));
        return -1;
    }
    catch (...) {
        Logger::getInstance().error("未知异常");
        return -1;
    }
}

void CourseSystem::shutdown() {
    if (running_) {
        // 保存所有数据
        try {
            UserManager::getInstance().saveData(false);
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
    
    User* user = UserManager::getInstance().authenticate(userId, password);
    
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

std::string CourseSystem::getText(const std::string& key) const {
    return I18nManager::getInstance().getText(key);
}

template<typename... Args>
std::string CourseSystem::getFormattedText(const std::string& key, Args... args) const {
    return I18nManager::getInstance().getFormattedText(key, args...);
}

// 显式实例化常用的模板函
template std::string CourseSystem::getFormattedText(const std::string& key, int) const;

void CourseSystem::showWelcome() const {
    std::cout << "================================================" << std::endl;
    std::cout << "               大学选课系统                      " << std::endl;
    std::cout << "      University Course Selection System         " << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

void CourseSystem::showMainMenu() {
    try {
        // 确保输入流处于良好状态
        if (std::cin.fail() || !std::cin.good()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        std::cout << "================================================" << std::endl;
        std::cout << "            " << getText("main_menu_title") << "            " << std::endl;
        std::cout << "================================================" << std::endl;
    
                                            
        // 显示菜单选项
        std::cout << "1. " << getText("login") << std::endl;
        std::cout << "2. " << getText("switch_language") << std::endl;
        std::cout << "3. " << getText("exit") << std::endl;
    
        // 获取用户输入
        int choice = 0;
        std::string input;
        int attempts = 0;
        const int MAX_ATTEMPTS = 3; // 最大尝试次数
        
        do {
            std::cout << "> ";
            std::getline(std::cin, input);
            
            attempts++;
            
            if (input.empty()) {
                std::cout << getText("input_cannot_be_empty") << std::endl;
                continue;
            }
            
           if (input == "1" || input == "2" || input == "3") {
                choice = std::stoi(input);
                break;
            } else {
                std::cout << getText("invalid_input") << std::endl;
            }    
            if (attempts >= MAX_ATTEMPTS) {
                std::cout << getText("too_many_attempts") << std::endl;
                choice = 3; // 默认选择退出
                break;
            }
        } while (true);
        
        if (choice == 1) {
            // 登录
            std::string userId, password;
            
            std::cout << getText("enter_user_id") << ": ";
            std::getline(std::cin, userId);
            
            std::cout << getText("enter_password") << ": ";
            std::getline(std::cin, password);
            
            try {
                if (login(userId, password)) {
                    std::cout << getText("login_success") << std::endl;
                } else {
                    std::cout << getText("login_failed") << std::endl;
                    // 安全性考虑，防止暴力破解
                    std::this_thread::sleep_for(std::chrono::seconds(1)); 
                }
            } catch (const std::exception& e) {
                Logger::getInstance().error(std::string("登陆时遇到系统错误") + ": " + e.what());
                std::cout << getText("login_system_error") << std::endl;
                // 安全性考虑，防止暴力破解
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else if (choice == 2) {
            // 切换语言
            showLanguageMenu();
        } else {
            // 退出
            std::cout << getText("exiting_system") << std::endl;
            shutdown();
        }
    } catch (const std::exception& e) {
        Logger::getInstance().error(std::string("主菜单中发生异常") + ": " + e.what());
        std::cout << getText("system_error_retry") << std::endl;
    } catch (...) {
        Logger::getInstance().error("主菜单中发生未知异常");
        std::cout << getText("unknown_system_error") << std::endl;
    }
}

void CourseSystem::showLanguageMenu() {
    std::cout << "================================================" << std::endl;
    std::cout << "            " << getText("language_menu_title") << "            " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // 显示当前语言，languageToString 是 I18nManager 的静态成员函数
    std::cout << getText("current_language") << ": " << 
    I18nManager::languageToString(I18nManager::getInstance().getCurrentLanguage()) << std::endl;
    
    // 显示可用的语言选项
    std::cout << "1. 中文 (Chinese)" << std::endl;
    std::cout << "2. English (英语)" << std::endl;
    std::cout << "3. " << "回到主菜单 (return_to_main_menu)" << std::endl;
    
    // 获取用户输入
    int choice = 0;
    std::string input;
    int attempts = 0;
    const int MAX_ATTEMPTS = 3;
    
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        attempts++;
        
        if (input.empty()) {
            std::cout << getText("input_cannot_be_empty") << std::endl;
            continue;
        }
        
        if (input == "1" || input == "2" || input == "3") {
            choice = std::stoi(input);
            break;
        } else {
            std::cout << getText("invalid_input") << std::endl;
        }
        if (attempts >= MAX_ATTEMPTS) {
            std::cout << getText("too_many_attempts") << std::endl;
            choice = 3;
            break;
        }
        
    } while (true);
    
    bool result = false;
    
    if (choice == 1) {
        // 切换到中文
        result = I18nManager::getInstance().setLanguage(Language::CHINESE);
        if (result) {
            std::cout << getText("language_switched_chinese") << std::endl;
        } else {
            std::cout << getText("language_switch_failed") << std::endl;
        }
    } else if (choice == 2) {
        // 切换到英文
        result = I18nManager::getInstance().setLanguage(Language::ENGLISH);
        if (result) {
            std::cout << getText("language_switched_english") << std::endl;
        } else {
            std::cout << getText("language_switch_failed") << std::endl;
        }
    }
    
    // 看清提示信息，防止操作过快
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void CourseSystem::showAdminMenu() {
    
    std::cout << "========= " << getText("admin_menu") << " =========" << std::endl;
    std::cout << "1. " << getText("user_management") << std::endl;
    std::cout << "2. " << getText("course_management") << std::endl;
    std::cout << "3. " << getText("query_enrollment_records") << std::endl;
    std::cout << "4. " << getText("modify_password") << std::endl;
    std::cout << "5. " << getText("modify_account_info") << std::endl;
    std::cout << "6. " << getText("logout") << std::endl;
    std::cout << "7. " << getText("exit") << std::endl;
    std::cout << "==============================" << std::endl;
    
    int choice = 0;
    std::string input;
    std::cout << "> ";
    std::getline(std::cin, input);
    
    if (!InputValidator::validateChoice(input, 1, 7, choice)) {
        std::cout << getText("invalid_input") << std::endl;
        return;
    }
    
    if (choice == 6) {
        logout();
        return;
    } else if (choice == 7) {
        shutdown();
        return;
    }
    
    try {
        switch (choice) {
            case 1:
            case 2:
            case 3:           
                handleAdminFunctions(choice);
                break;
            case 4:
                handlePasswordChange();
                break;
            case 5:
                handleUserInfoModification(); // 处理修改用户信息
                break;
            default:
                std::cout << getText("invalid_choice") << std::endl;
                break;
        }
    } catch (const SystemException& e) {
        // 处理系统异常
        Logger::getInstance().error("处理管理员菜单选择时发生异常: " + e.getFormattedMessage());
        std::cout << getText("system_error") << ": " << e.getFormattedMessage() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } catch (const std::exception& e) {
        // 处理标准异常
        Logger::getInstance().error("处理管理员菜单选择时发生标准异常: " + std::string(e.what()));
        std::cout << getText("system_error") << ": " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } catch (...) {
        // 处理未知异常
        Logger::getInstance().error("处理管理员菜单选择时发生未知异常");
        std::cout << getText("system_error") << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void CourseSystem::showTeacherMenu() {
    std::cout << getText("teacher_menu_title") << std::endl;
    std::cout << "1. " << getText("view_courses") << std::endl;
    std::cout << "2. " << getText("view_students") << std::endl;
    std::cout << "3. " << getText("change_password") << std::endl;
    std::cout << "4. " << getText("modify_user_info") << std::endl;
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
    
    try {
        if (choice >= 1 && choice <= 2) {
            handleTeacherFunctions(choice);
        } else if (choice == 3) {
            handlePasswordChange(); 
        } else if (choice == 4) {
            handleUserInfoModification(); 
        } else if (choice == 5) {
            logout();
        } else {
            shutdown();
        }
    } catch(const SystemException& e) {
        Logger::getInstance().error("处理教师菜单选择时发生异常: " + e.getFormattedMessage());
        std::cout << getText("system_error") << ": " << e.getFormattedMessage() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } catch(const std::exception& e) {
        Logger::getInstance().error("处理教师菜单选择时发生标准异常: " + std::string(e.what()));
        std::cout << getText("system_error") << ": " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } catch(...) {
        Logger::getInstance().error("处理教师菜单选择时发生未知异常");
        std::cout << getText("system_error") << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void CourseSystem::showStudentMenu() {
    std::cout << getText("student_menu_title") << std::endl;
    std::cout << "1. " << getText("query_courses") << std::endl;
    std::cout << "2. " << getText("select_course") << std::endl;
    std::cout << "3. " << getText("drop_course") << std::endl;
    std::cout << "4. " << getText("view_selected_courses") << std::endl;
    std::cout << "5. " << getText("change_password") << std::endl;
    std::cout << "6. " << getText("modify_user_info") << std::endl;
    std::cout << "7. " << getText("logout") << std::endl;
    std::cout << "8. " << getText("exit") << std::endl;
    
    int choice = 0;
    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!InputValidator::validateChoice(input, 1, 8, choice)) {
            std::cout << getText("invalid_input") << std::endl;
        }
    } while (choice < 1 || choice > 8);
    try{
        if (choice >= 1 && choice <= 4) {
            handleStudentFunctions(choice);
        } else if (choice == 5) {
            handlePasswordChange(); 
        } else if (choice == 6) {
            handleUserInfoModification(); 
        } else if (choice == 7) {
            logout();
        } else {
            shutdown();
        }
    }catch(const SystemException& e){
        Logger::getInstance().error("处理学生菜单选择时发生异常: " + e.getFormattedMessage());
        std::cout << getText("system_error") << ": " << e.getFormattedMessage() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }catch(const std::exception& e){
        Logger::getInstance().error("处理学生菜单选择时发生标准异常: " + std::string(e.what()));
        std::cout << getText("system_error") << ": " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }catch(...){
        Logger::getInstance().error("处理学生菜单选择时发生未知异常");
        std::cout << getText("system_error") << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void CourseSystem::handleAdminFunctions(int choice) {
    // 定义最大尝试次数常量，在整个函数中共用
    const int MAX_ATTEMPTS = 3;
    
    switch (choice) {
        case 1: { // 用户管理
            std::cout << getText("user_management_function") << std::endl;
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
                        
                        int userType;
                        std::string userTypeInput;
                        int attempts = 0;
                        std::cout << "> ";
                        std::getline(std::cin, userTypeInput);
                        
                        if (!InputValidator::validateChoice(userTypeInput, 1, 3, userType)) {
                            std::cout << getText("invalid_user_type") << std::endl;
                            break;
                        }
                        
                        // 添加通用用户信息
                        std::string userId, name, password, gender;
                        
                        std::cout << getText("enter_user_id_prompt") << "：";
                        std::getline(std::cin, userId);
                        
                        // 添加用户ID非空验证
                        attempts = 0;
                        while (InputValidator::isEmptyInput(userId) && attempts < MAX_ATTEMPTS) {
                            attempts++;
                            std::cout << getText("input_cannot_be_empty") << std::endl;
                            std::cout << getText("enter_user_id_prompt") << "：";
                            std::getline(std::cin, userId);
                            
                            if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(userId)) {
                                std::cout << getText("too_many_attempts") << std::endl;
                                break;
                            }
                        }
                        
                        // 检查用户ID是否已存在
                        if (userManager.getUser(userId) != nullptr) {
                            std::cout << getText("user_id_exists") << std::endl;
                            break;
                        }
                        
                        std::cout << getText("enter_username") << "：";
                        std::getline(std::cin, name);
                        
                        // 添加用户名非空验证
                        attempts = 0;
                        while (InputValidator::isEmptyInput(name) && attempts < MAX_ATTEMPTS) {
                            attempts++;
                            std::cout << getText("username_cannot_be_empty") << std::endl;
                            std::cout << getText("enter_username") << "：";
                            std::getline(std::cin, name);
                                
                            if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(name)) {
                                std::cout << getText("too_many_attempts") << std::endl;
                                break;
                            }
                        }
                        
                        std::cout << getText("enter_user_password") << "：";
                        std::getline(std::cin, password);
                        
                        // 添加密码非空验证
                        attempts = 0;
                        while (InputValidator::isEmptyInput(password) && attempts < MAX_ATTEMPTS) {
                            attempts++;
                                std::cout << getText("password_cannot_be_empty") << std::endl;
                                std::cout << getText("enter_user_password") << "：";
                                std::getline(std::cin, password);
                                
                            if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(password)) {
                                std::cout << getText("too_many_attempts") << std::endl;
                                break;
                            }
                        }
                        
                        // 性别选择菜单
                        std::cout << getText("enter_user_gender") << "：" << std::endl;
                        std::cout << "1. " << getText("gender_male") << std::endl;
                        std::cout << "2. " << getText("gender_female") << std::endl;
                        
                        int genderChoice;
                        std::string genderInput;
                        int genderAttempts = 0;

                        while (genderAttempts < MAX_ATTEMPTS) {
                            std::cout << "> ";
                            std::getline(std::cin, genderInput);
                            genderAttempts++;
                            
                            if (InputValidator::validateChoice(genderInput, 1, 2, genderChoice)) {
                                gender = (genderChoice == 1) ? "male" : "female";
                                break;
                            } else {
                                std::cout << getText("invalid_choice") << std::endl;
                                if (genderAttempts >= MAX_ATTEMPTS) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                            }
                        }
                        
                        // 根据用户类型创建不同的用户对象
                        if (userType == 1) {  // 学生
                            std::string age, department, classInfo, email;
                            
                            std::cout << getText("enter_student_age") << "：";
                            
                            int ageValue = 0;
                            int ageAttempts = 0;
 
                            while (ageAttempts < MAX_ATTEMPTS) {
                                std::getline(std::cin, age);
                                ageAttempts++;
                                
                                if (InputValidator::validateInteger(age, 15, 80, ageValue)) {
                                    break;
                                } else {
                                    std::cout << getText("invalid_age") << std::endl;
                                    if (ageAttempts >= MAX_ATTEMPTS) {
                                        std::cout << getText("too_many_attempts") << std::endl;
                                        break;
                                    }
                                    std::cout << getText("enter_student_age") << "：";
                                }
                            }
                           
                            std::cout << getText("enter_department") << "：";
                            std::getline(std::cin, department);
                            
                            attempts = 0;
                            while (InputValidator::isEmptyInput(department) && attempts < MAX_ATTEMPTS) {
                                attempts++;
                                std::cout << getText("department_cannot_be_empty") << std::endl;
                                std::cout << getText("enter_department") << "：";
                                std::getline(std::cin, department);
                                    
                                if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(department)) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                            }
                           
                            std::cout << getText("enter_class_info") << "：";
                            std::getline(std::cin, classInfo);
                            
                            attempts = 0;
                            while (InputValidator::isEmptyInput(classInfo) && attempts < MAX_ATTEMPTS) {
                                attempts++;
                                std::cout << getText("class_info_cannot_be_empty") << std::endl;
                                std::cout << getText("enter_class_info") << "：";
                                std::getline(std::cin, classInfo);
                                    
                                if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(classInfo)) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                            }
                           
                            std::cout << getText("enter_email") << "：";
                            std::getline(std::cin, email);
                            
                            attempts = 0;
                            while (InputValidator::isEmptyInput(email) && attempts < MAX_ATTEMPTS) {
                                attempts++;
                                    std::cout << getText("input_cannot_be_empty") << std::endl;
                                    std::cout << getText("enter_email") << "：";
                                    std::getline(std::cin, email);
                                    
                                if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(email)) {
                                        std::cout << getText("too_many_attempts") << std::endl;
                                        break;
                                    }
                            }
                            
                            
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
                            
                            attempts = 0;
                            while (InputValidator::isEmptyInput(title) && attempts < MAX_ATTEMPTS) {
                                attempts++;
                                std::cout << getText("input_cannot_be_empty") << std::endl;
                                std::cout << getText("enter_teacher_title") << "：";
                                std::getline(std::cin, title);
                                    
                                if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(title)) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                            }
                            
                            std::cout << getText("enter_teacher_department") << "：";
                            std::getline(std::cin, department);
                            
                            // 添加院系非空验证
                            attempts = 0;
                            while (InputValidator::isEmptyInput(department) && attempts < MAX_ATTEMPTS) {
                                attempts++;
                                std::cout << getText("department_cannot_be_empty") << std::endl;
                                std::cout << getText("enter_teacher_department") << "：";
                                std::getline(std::cin, department);
                                    
                                if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(department)) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                            }
                            
                            std::cout << getText("enter_email") << "：";
                            std::getline(std::cin, email);
                           
                            attempts = 0;
                            while (InputValidator::isEmptyInput(email) && attempts < MAX_ATTEMPTS) {
                                attempts++;
                                std::cout << getText("input_cannot_be_empty") << std::endl;
                                std::cout << getText("enter_email") << "：";
                                std::getline(std::cin, email);
                                    
                                if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(email)) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                            }
                            
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
                        
                        // 先显示所有用户列表
                        std::vector<std::string> studentIds = userManager.getAllStudentIds();
                        std::vector<std::string> teacherIds = userManager.getAllTeacherIds();
                        std::vector<std::string> adminIds = userManager.getAllAdminIds();
                        
                        std::cout << getText("user_list") << "：" << std::endl;
                        std::cout << "--------------------------------" << std::endl;
                        
                        // 显示学生列表
                        if (!studentIds.empty()) {
                            std::cout << getText("student_list") << "：" << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getText("user_id") << "\t" << getText("user_name") << "\t" 
                                      << getText("age") << "\t" << getText("gender") << "\t"
                                      << getText("department") << "\t" << getText("class") << "\t"
                                      << getText("enter_email") << std::endl;
                            
                            for (const std::string& studentId : studentIds) {
                                Student* student = userManager.getStudent(studentId);
                                if (student) {
                                    std::cout << student->getId() << "\t"
                                              << student->getName() << "\t"
                                              << student->getAge() << "\t"
                                              << student->getGender() << "\t"
                                              << student->getDepartment() << "\t"
                                              << student->getClassInfo() << "\t"
                                              << student->getContact() << std::endl;
                                }
                            }
                            std::cout << "--------------------------------" << std::endl;
                        }
                        
                        // 显示教师列表
                        if (!teacherIds.empty()) {
                            std::cout << getText("teacher_list") << "：" << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getText("user_id") << "\t" << getText("user_name") << "\t" 
                                      << getText("title") << "\t" << getText("department") << "\t"
                                      << getText("enter_email") << std::endl;
                            
                            for (const std::string& teacherId : teacherIds) {
                                Teacher* teacher = userManager.getTeacher(teacherId);
                                if (teacher) {
                                    std::cout << teacher->getId() << "\t"
                                              << teacher->getName() << "\t"
                                              << teacher->getTitle() << "\t"
                                              << teacher->getDepartment() << "\t"
                                              << teacher->getContact() << std::endl;
                                }
                            }
                            std::cout << "--------------------------------" << std::endl;
                        }
                        
                        // 显示管理员列表
                        if (!adminIds.empty()) {
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
                        }
                        
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
                        
                        // 检查当前用户是否为管理员且尝试删除自己
                        if (currentUser_ && currentUser_->getType() == UserType::ADMIN && currentUser_->getId() == userId) {
                            std::cout << getText("cannot_delete_self") << std::endl;
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
                                Student* student = dynamic_cast<Student*>(user);
                                std::cout << getText("age") << ": " << student->getAge() << std::endl;
                                std::cout << getText("gender") << ": " << student->getGender() << std::endl;
                                std::cout << getText("department") << ": " << student->getDepartment() << std::endl;
                                std::cout << getText("class") << ": " << student->getClassInfo() << std::endl;
                                std::cout << getText("email_address") << ": " << student->getContact() << std::endl;
                            } else if (user->getType() == UserType::TEACHER) {
                                Teacher* teacher = dynamic_cast<Teacher*>(user);
                                std::cout << getText("title") << ": " << teacher->getTitle() << std::endl;
                                std::cout << getText("department") << ": " << teacher->getDepartment() << std::endl;
                                std::cout << getText("email_address") << ": " << teacher->getContact() << std::endl;
                            } else if (user->getType() == UserType::ADMIN) {
                                // Admin类没有额外属性需要显示
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
                                      << getText("age") << "\t" << getText("gender") << "\t"
                                      << getText("department") << "\t" << getText("class") << "\t"
                                      << getText("email_address") << std::endl;
                            
                            for (const std::string& studentId : studentIds) {
                                Student* student = userManager.getStudent(studentId);
                                if (student) {
                                    std::cout << student->getId() << "\t"
                                              << student->getName() << "\t"
                                              << student->getAge() << "\t"
                                              << student->getGender() << "\t"
                                              << student->getDepartment() << "\t"
                                              << student->getClassInfo() << "\t"
                                              << student->getContact() << std::endl;
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
                                      << getText("title") << "\t" << getText("department") << "\t"
                                      << getText("email_address") << std::endl;
                            
                            for (const std::string& teacherId : teacherIds) {
                                Teacher* teacher = userManager.getTeacher(teacherId);
                                if (teacher) {
                                    std::cout << teacher->getId() << "\t"
                                              << teacher->getName() << "\t"
                                              << teacher->getTitle() << "\t"
                                              << teacher->getDepartment() << "\t"
                                              << teacher->getContact() << std::endl;
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
                
                if (subChoice == 5) {
                    subMenuRunning = false;
                    continue;
                }
                
                // 根据用户选择执行相应操作
                switch (subChoice) {
                    case 1: { // 添加课程
                        // 获取课程管理器
                        CourseManager& courseManager = CourseManager::getInstance();
                        
                        // 添加课程信息
                        std::string courseId, name, typeStr, creditStr, hoursStr, semester, teacherId, maxCapacityStr;
                        
                        std::cout << getText("enter_course_id") << "：";
                        std::getline(std::cin, courseId);
                        
                        // 添加课程ID非空验证
                        int attempts = 0;
                        while (InputValidator::isEmptyInput(courseId) && attempts < MAX_ATTEMPTS) {
                            attempts++;
                            std::cout << getText("input_cannot_be_empty") << std::endl;
                            std::cout << getText("enter_course_id") << "：";
                            std::getline(std::cin, courseId);
                                
                            if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(courseId)) {
                                std::cout << getText("too_many_attempts") << std::endl;
                                break;
                            }
                        }
                        
                        // 检查课程ID是否已存在
                        if (courseManager.hasCourse(courseId)) {
                            std::cout << getText("course_id_exists") << std::endl;
                            continue;
                        }
                        
                        std::cout << getText("enter_course_name") << "：";
                        std::getline(std::cin, name);
                        
                        // 添加课程名称非空验证
                        attempts = 0;
                        while (InputValidator::isEmptyInput(name) && attempts < MAX_ATTEMPTS) {
                            attempts++;
                            std::cout << getText("course_name_cannot_be_empty") << std::endl;
                            std::cout << getText("enter_course_name") << "：";
                            std::getline(std::cin, name);
                                
                            if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(name)) {
                                std::cout << getText("too_many_attempts") << std::endl;
                                break;
                            }
                        }
                        
                        // 选择课程类型
                        std::cout << getText("select_course_type") << "：" << std::endl;
                        std::cout << "1. " << getText("required_course") << std::endl;
                        std::cout << "2. " << getText("elective_course") << std::endl;
                        
                        int typeChoice = 0;
                        std::string typeInput;
                        int typeAttempts = 0;
                
                        while (typeAttempts < MAX_ATTEMPTS) {
                            std::cout << "> ";
                            std::getline(std::cin, typeInput);
                            typeAttempts++;
                            
                            if (InputValidator::validateChoice(typeInput, 1, 2, typeChoice)) {
                                break;
                            } else {
                                std::cout << getText("invalid_course_type") << std::endl;
                                if (typeAttempts >= MAX_ATTEMPTS) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                            }
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
                        
                        double credit = 0.0;
                        int creditAttempts = 0;
                        
                        while (creditAttempts < MAX_ATTEMPTS) {
                            std::getline(std::cin, creditStr);
                            creditAttempts++;
                            
                            if (InputValidator::validateDouble(creditStr, 0.0, 10.0, credit)) {
                                break;
                            } else {
                                std::cout << getText("invalid_credit") << std::endl;
                                if (creditAttempts >= MAX_ATTEMPTS) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                                std::cout << getText("enter_credit") << "：";
                            }
                        }
                        
                        std::cout << getText("enter_hours") << "：";
                        std::getline(std::cin, hoursStr);
                        
                        int hours = 0;
                        int hoursAttempts = 0;
                        while (hoursAttempts < MAX_ATTEMPTS) {
                            hoursAttempts++;
                            if (InputValidator::validateInteger(hoursStr, 0, 200, hours)) {
                                break;
                            } else {
                                std::cout << getText("invalid_hours") << std::endl;
                                if (hoursAttempts >= MAX_ATTEMPTS) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                                std::cout << getText("enter_hours") << "：";
                                std::getline(std::cin, hoursStr);
                            }
                        }
                        
                        std::cout << getText("enter_semester") << "：";
                        std::getline(std::cin, semester);
                        
                        // 添加学期非空验证
                        attempts = 0;
                        while (InputValidator::isEmptyInput(semester) && attempts < MAX_ATTEMPTS) {
                            attempts++;
                            std::cout << getText("semester_cannot_be_empty") << std::endl;
                            std::cout << getText("enter_semester") << "：";
                            std::getline(std::cin, semester);
                            
                            if (attempts >= MAX_ATTEMPTS && InputValidator::isEmptyInput(semester)) {
                                std::cout << getText("too_many_attempts") << std::endl;
                                break;
                            }
                        }
                        
                        
                        // 获取并显示所有教师列表
                        UserManager& userManager = UserManager::getInstance();
                        std::vector<std::string> teacherIds = userManager.getAllTeacherIds();
                        
                        if (teacherIds.empty()) {
                            std::cout << getText("no_teachers") << std::endl;
                            continue;
                        }
                        
                        std::cout << getText("available_teachers") << "：" << std::endl;
                        std::cout << "--------------------------------" << std::endl;
                        std::cout << getText("teacher_id") << "\t" 
                                  << getText("teacher_name") << std::endl;
                        
                        for (const std::string& id : teacherIds) {
                            Teacher* teacher = userManager.getTeacher(id);
                            if (teacher) {
                                std::cout << teacher->getId() << "\t"
                                          << teacher->getName() << std::endl;
                            }
                        }
                        std::cout << "--------------------------------" << std::endl;
                        
                        std::cout << getText("enter_teacher_id") << "：";
                        
                        int teacherAttempts = 0;
                        
                        while (teacherAttempts < MAX_ATTEMPTS) {
                            std::getline(std::cin, teacherId);
                            teacherAttempts++;
                            
                            if (userManager.getTeacher(teacherId)) {
                                break;
                            } else {
                                std::cout << getText("teacher_id_not_exists") << std::endl;
                                if (teacherAttempts >= MAX_ATTEMPTS) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                                std::cout << getText("enter_teacher_id") << "：";
                            }
                        }
                        
                        std::cout << getText("enter_max_capacity") << "：";
                        
                        int maxCapacity = 0;
                        int maxCapacityAttempts = 0;
                        
                        while (maxCapacityAttempts < MAX_ATTEMPTS) {
                            std::getline(std::cin, maxCapacityStr);
                            maxCapacityAttempts++;
                            
                            if (InputValidator::validateInteger(maxCapacityStr, 1, 1000, maxCapacity)) {
                                break;
                            } else {
                                std::cout << getText("invalid_max_capacity") << std::endl;
                                if (maxCapacityAttempts >= MAX_ATTEMPTS) {
                                    std::cout << getText("too_many_attempts") << std::endl;
                                    break;
                                }
                                std::cout << getText("enter_max_capacity") << "：";
                            }
                        }
                        
                        
                        // 创建课程对象
                        std::unique_ptr<Course> course = std::make_unique<Course>(
                            courseId, name, type, credit, hours, 
                            semester, teacherId, maxCapacity
                        );
                            
                            // 添加课程
                        if (courseManager.addCourse(std::move(course))) {
                            std::cout << getText("add_course_success") << std::endl;
                        } else {
                            std::cout << getText("add_course_failed") << std::endl;
                        }
                        break;
                    }
                    case 2: { // 删除课程
                        // 获取课程管理器
                        CourseManager& courseManager = CourseManager::getInstance();
                        EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
                        
                        // 显示所有课程列表
                        std::vector<std::string> allCourseIds = courseManager.getAllCourseIds();
                        
                        if (allCourseIds.empty()) {
                            std::cout << getText("no_courses") << std::endl;
                            continue;
                        }
                        
                        std::cout << getText("all_courses") << "：" << std::endl;
                        std::cout << "--------------------------------" << std::endl;
                        std::cout << getText("course_id") << "\t" 
                                << getText("course_name") << "\t" 
                                << getText("course_type") << std::endl;
                        
                        for (const std::string& id : allCourseIds) {
                            Course* course = courseManager.getCourse(id);
                            if (course) {
                                std::cout << course->getId() << "\t"
                                        << course->getName() << "\t"
                                        << course->getTypeString() << std::endl;
                            }
                        }
                        std::cout << "--------------------------------" << std::endl;
                        
                        // 获取课程ID
                        std::string courseId;
                        std::cout << getText("enter_delete_course_id") << "：";
                        std::getline(std::cin, courseId);
                        
                        // 检查课程是否存在
                        Course* course = courseManager.getCourse(courseId);
                        if (!course) {
                            std::cout << getText("course_id_not_exists") << std::endl;
                            continue;
                        }
                        
                        // 显示课程信息
                        std::cout << getText("course_to_delete") << "：" << std::endl;
                        std::cout << getText("course_id") << ": " << course->getId() << std::endl;
                        std::cout << getText("course_name") << ": " << course->getName() << std::endl;
                        std::cout << getText("course_type") << ": " << course->getTypeString() << std::endl;
                        std::cout << getText("credit") << ": " << course->getCredit() << std::endl;
                        std::cout << getText("current_enrollment") << ": " << course->getCurrentEnrollment() << "/" << course->getMaxCapacity() << std::endl;
                        
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
                        
                        // 显示所有课程列表
                        std::vector<std::string> allCourseIds = courseManager.getAllCourseIds();
                        
                        if (allCourseIds.empty()) {
                            std::cout << getText("no_courses") << std::endl;
                            break;
                        }
                        
                        std::cout << "所有课程：" << std::endl;
                        std::cout << "--------------------------------" << std::endl;
                        std::cout << getText("course_id") << "\t" 
                                  << getText("course_name") << "\t" 
                                  << getText("course_type") << std::endl;
                        
                        for (const std::string& id : allCourseIds) {
                            Course* course = courseManager.getCourse(id);
                            if (course) {
                                std::cout << course->getId() << "\t"
                                          << course->getName() << "\t"
                                          << course->getTypeString() << std::endl;
                            }
                        }
                        std::cout << "--------------------------------" << std::endl;
                        
                        // 获取课程ID
                        std::string courseId;
                        std::cout << getText("enter_modify_course_id") << "：";
                        std::getline(std::cin, courseId);
                        
                        // 检查课程是否存在
                        Course* course = courseManager.getCourse(courseId);
                        if (!course) {
                            std::cout << getText("course_id_not_exists") << std::endl;
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
                        std::cout << getText("current_enrollment") << ": " << course->getCurrentEnrollment() << "/" << course->getMaxCapacity() << std::endl;
                        
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
                        
                        int modifyChoice;
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
                                if (newName.empty()) {
                                    std::cout << getText("course_name_cannot_be_empty") << std::endl;
                                    break;
                                }
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
                                if (newSemester.empty()) {
                                    std::cout << getText("invalid_input") << std::endl;
                                    break;
                                }
                                course->setSemester(newSemester);
                                std::cout << getText("course_semester_modify_success") << std::endl;
                                break;
                            }
                            case 6: { // 修改教师ID
                                // 获取并显示所有教师列表
                                UserManager& userManager = UserManager::getInstance();
                                std::vector<std::string> teacherIds = userManager.getAllTeacherIds();
                                
                                if (teacherIds.empty()) {
                                    std::cout << getText("no_teachers") << std::endl;
                                    break;
                                }
                                
                                std::cout << getText("available_teachers") << "：" << std::endl;
                                std::cout << "--------------------------------" << std::endl;
                                std::cout << getText("teacher_id") << "\t" 
                                          << getText("teacher_name") << std::endl;
                                
                                for (const std::string& id : teacherIds) {
                                    Teacher* teacher = userManager.getTeacher(id);
                                    if (teacher) {
                                        std::cout << teacher->getId() << "\t"
                                                  << teacher->getName() << std::endl;
                                    }
                                }
                                std::cout << "--------------------------------" << std::endl;
                                
                                std::string newTeacherId;
                                std::cout << getText("enter_new_teacher_id") << "：";
                                std::getline(std::cin, newTeacherId);
                                
                                // 验证教师ID是否存在
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
                                //lambda表达式,返回值为bool类型，findcourses的形参为谓词，find为标准库中string的成员函数
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
                                
                                int typeChoice;
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
                                std::cout << getText("course_id") << "\t" 
                                  << getText("course_name") << "\t" 
                                  << getText("course_type") << "\t"
                                  << getText("credit") << "\t"
                                  << getText("hours") << "\t"
                                  << getText("teacher_id") << "\t"
                                  << getText("current_enrollment") << "/" << getText("max_capacity") << std::endl;
                                
                                for (const std::string& courseId : courseIds) {
                                    Course* course = courseManager.getCourse(courseId);
                                    if (course) {
                                        std::cout << course->getId() << "\t"
                                                << course->getName() << "\t"
                                                << course->getTypeString() << "\t"
                                                << course->getCredit() << "\t"
                                                << course->getHours() << "\t"
                                                << course->getTeacherId() << "\t"
                                                << course->getCurrentEnrollment() << "/"
                                                << course->getMaxCapacity() << std::endl;
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
                    default:
                        std::cout << getText("invalid_choice") << std::endl;
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
                
                int subChoice;
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
                        
                        // 先验证学生ID是否存在
                        UserManager& userManager = UserManager::getInstance();
                        Student* student = userManager.getStudent(studentId);
                        
                        if (!student) {
                            std::cout << getText("user_id_not_exists") << std::endl;
                        } else {
                            // 获取学生的所有选课记录
                            std::vector<Enrollment*> enrollments = enrollmentManager.getStudentEnrollments(studentId);
                            
                            if (enrollments.empty()) {
                                std::cout << getText("no_selected_courses") << std::endl;
                            } else {
                                std::cout << getFormattedText("student_selected_courses", studentId) << std::endl;
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
                                std::cout << getFormattedText("selected_courses_count", static_cast<int>(enrollments.size())) << std::endl;
                            }
                        }
                        
                        std::cout << getText("press_enter_to_continue") << std::endl;
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        break;
                    }
                    case 2: { // 按课程查询
                        std::string courseId;
                        std::cout << getText("enter_course_id") << "：";
                        std::getline(std::cin, courseId);
                        
                        // 获取该课程的所有选课记录
                        CourseManager& courseManager = CourseManager::getInstance(); // 添加课程管理器
                        
                        // 先检查课程是否存在
                        if (!courseManager.hasCourse(courseId)) {
                            std::cout << getText("course_not_exists") << std::endl;
                            break;
                        }
                        
                        std::vector<Enrollment*> enrollments = enrollmentManager.getCourseEnrollments(courseId);
                        
                        if (enrollments.empty()) {
                            std::cout << getText("no_course_students") << std::endl;
                        } else {
                            std::cout << getFormattedText("course_students", courseId) << std::endl;
                            std::cout << "--------------------------------" << std::endl;
                            std::cout << getText("user_id") << "\t" 
                                          << getText("user_name") << "\t" 
                                          << getText("class") << "\t" 
                                          << getText("department") << std::endl;
                            
                            UserManager& userManager = UserManager::getInstance();
                            for (Enrollment* enrollment : enrollments) {
                                std::string studentId = enrollment->getStudentId();
                                Student* student = dynamic_cast<Student*>(userManager.getStudent(studentId));
                                
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

void CourseSystem::handleStudentFunctions(int choice) {
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
                    case 5: // 返回上级菜单
                        subMenuRunning = false;
                        continue;
                }
                
                // 显示查询结果
                if (courseIds.empty()) {
                    std::cout << getText("no_courses") << std::endl;
                } else {
                    std::cout << getText("query_result") << "：" << std::endl;
                    std::cout << "--------------------------------" << std::endl;
                    std::cout << getText("course_id") << "\t" 
                          << getText("course_name") << "\t" 
                          << getText("credit") << "\t" 
                          << getText("hours") << "\t"
                          << getText("teacher_id") << "\t"
                          << getText("current_enrollment") << "/" << getText("max_capacity") << std::endl;
                    
                    for (const std::string& courseId : courseIds) {
                        Course* course = courseManager.getCourse(courseId);
                        if (course) {
                            std::cout << course->getId() << "\t"
                                      << course->getName() << "\t"
                                      << course->getCredit() << "\t"
                                      << course->getHours() << "\t"
                                      << course->getTeacherId() << "\t"
                                      << course->getCurrentEnrollment() << "/" 
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
            
            // 获取课程管理器和选课管理器
            CourseManager& courseManager = CourseManager::getInstance();
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            
            // 先显示所有可选课程
            std::vector<std::string> allCourseIds = courseManager.getAllCourseIds();
            
            if (allCourseIds.empty()) {
                std::cout << getText("no_courses") << std::endl;
            } else {
                std::cout << getText("available_courses") << "：" << std::endl;
                std::cout << "--------------------------------" << std::endl;
                std::cout << getText("course_id") << "\t" 
                          << getText("course_name") << "\t" 
                          << getText("credit") << "\t" 
                          << getText("teacher_id") << "\t"
                          << getText("current_enrollment") << "/"
                          << getText("max_capacity") << std::endl;
                
                // 获取学生当前已选课程列表，用于显示时标记
                std::vector<Enrollment*> studentEnrollments = enrollmentManager.getStudentEnrollments(studentId);
                std::vector<std::string> enrolledCourseIds;
                for (const auto& enrollment : studentEnrollments) {
                    enrolledCourseIds.push_back(enrollment->getCourseId());
                }
                
                for (const std::string& courseId : allCourseIds) {
                    Course* course = courseManager.getCourse(courseId);
                    if (course) {
                        bool alreadyEnrolled = std::find(enrolledCourseIds.begin(), enrolledCourseIds.end(), courseId) != enrolledCourseIds.end();
                        
                        std::cout << course->getId() << "\t"
                                  << course->getName() << "\t"
                                  << course->getCredit() << "\t"
                                  << course->getTeacherId() << "\t"
                                  << course->getCurrentEnrollment() << "/"
                                  << course->getMaxCapacity();
                                  
                        // 标记已选课程
                        if (alreadyEnrolled) {
                            std::cout << " (" << getText("already_selected") << ")";
                        }
                        std::cout << std::endl;
                    }
                }
                std::cout << "--------------------------------" << std::endl;
                
                // 获取课程ID
                std::cout << getText("select_by_course_id") << ": ";
                std::string courseId;
                std::getline(std::cin, courseId);
                
                // 验证课程ID存在
                if (!courseManager.hasCourse(courseId)) {
                    std::cout << getText("course_not_found") << std::endl;
                } else {
                    // 尝试选课
                    try {
                        if (enrollmentManager.enrollCourse(studentId, courseId)) {
                            std::cout << getText("operation_success") << std::endl;
                            
                            // 更新课程数据以确保人数正确显示
                            courseManager.loadData();
                            
                            // 显示选课成功后的课程信息
                            Course* course = courseManager.getCourse(courseId);
                            if (course) {
                                std::cout << getText("course") << " " << course->getName() << " " 
                                          << getText("current_enrollment") << ": " 
                                          << course->getCurrentEnrollment() << "/" 
                                          << course->getMaxCapacity() << std::endl;
                            }
                        } else {
                            std::cout << getText("operation_failed") << std::endl;
                        }
                    } catch (const SystemException& e) {
                        std::cout << getText("operation_failed") << ": " << e.what() << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << getText("system_error") << ": " << e.what() << std::endl;
                    }
                }
            }
            
            std::cout << getText("press_enter_to_continue") << "..." << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
            
        case 3: { // 退选课程
            std::cout << getText("drop_course_function") << std::endl;
            
            std::string studentId = currentUser_->getId();
            
            // 获取选课管理器和课程管理器
            EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
            CourseManager& courseManager = CourseManager::getInstance();
            
            // 获取学生的所有选课记录
            std::vector<Enrollment*> enrollments = enrollmentManager.getStudentEnrollments(studentId);
            
            if (enrollments.empty()) {
                std::cout << getText("no_selected_courses") << std::endl;
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
                
                // 获取要退选的课程ID
                std::cout << getText("enter_drop_course_id") << ": ";
                std::string courseId;
                std::getline(std::cin, courseId);
                
                // 检查是否已选该课程，同时获取课程对象确认存在性
                bool hasEnrolled = false;
                Course* courseToDrop = nullptr;
                CourseManager& courseManager = CourseManager::getInstance();
                
                for (const auto& enrollment : enrollments) {
                    if (enrollment->getCourseId() == courseId) {
                        hasEnrolled = true;
                        courseToDrop = courseManager.getCourse(courseId);
                        break;
                    }
                }
                
                if (!hasEnrolled || !courseToDrop) {
                    std::cout << getText("not_enrolled_course") << std::endl;
                } else {
                    // 尝试退课
                    try {
                        if (enrollmentManager.dropCourse(studentId, courseId)) {
                            std::cout << getText("operation_success") << std::endl;
                            
                            // 更新课程数据以确保人数正确显示
                            courseManager.loadData();
                        } else {
                            std::cout << getText("operation_failed") << std::endl;
                        }
                    } catch (const SystemException& e) {
                        std::cout << getText("operation_failed") << ": " << e.what() << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << getText("system_error") << ": " << e.what() << std::endl;
                    }
                }
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
                std::cout << getText("no_selected_courses") << std::endl;
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
            Logger::getInstance().error("修改密码失败：用户未登录");
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

void CourseSystem::handlePasswordChange() {
    if (!currentUser_) {
        std::cout << getText("operation_failed") << ": " << getText("password_change_failed") << std::endl;
        Logger::getInstance().error("修改密码失败：用户未登录");
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

void CourseSystem::handleTeacherFunctions(int choice) {
    switch (choice) {
        case 1: { // 查看课程
            std::cout << getText("view_courses_function") << std::endl;
            
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
                std::cout << getText("no_teaching_courses") << std::endl;
            } else {
                std::cout << getText("your_courses") << "：" << std::endl;
                std::cout << "--------------------------------" << std::endl;
                std::cout << getText("course_id") << "\t" 
                          << getText("course_name") << "\t" 
                          << getText("credit") << "\t" 
                          << getText("hours") << "\t"
                          << getText("semester") << "\t"
                          << getText("current_enrollment") << "/" << getText("max_capacity") << std::endl;
                
                for (const std::string& courseId : teacherCourseIds) {
                    Course* course = courseManager.getCourse(courseId);
                    if (course) {
                        std::cout << course->getId() << "\t"
                                  << course->getName() << "\t"
                                  << course->getCredit() << "\t"
                                  << course->getHours() << "\t"
                                  << course->getSemester() << "\t"
                                  << course->getCurrentEnrollment() << "/" << course->getMaxCapacity() << std::endl;
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
                std::cout << getText("no_teaching_courses") << std::endl;
            } else {
                // 先列出所有课程
                std::cout << getText("your_courses") << "：" << std::endl;
                for (std::size_t i = 0; i < teacherCourseIds.size(); ++i) {
                    Course* course = courseManager.getCourse(teacherCourseIds[i]);
                    if (course) {
                        std::cout << (i+1) << ". " << course->getId() << " - " << course->getName() << std::endl;
                    }
                }
                
                // 选择要查看的课程
                int courseIndex = 0;
                std::string input;
                int attempts = 0;
                const int MAX_ATTEMPTS = 3;
                
                while (attempts < MAX_ATTEMPTS) {
                    std::cout << getFormattedText("select_course_to_view", static_cast<int>(teacherCourseIds.size())) << "：";
                    std::getline(std::cin, input);
                    attempts++;     
                    if (InputValidator::validateChoice(input, 1, teacherCourseIds.size(), courseIndex)) {
                        break;
                    } else {
                        std::cout << getText("invalid_input") << std::endl;
                        if (attempts >= MAX_ATTEMPTS) {
                            std::cout << getText("too_many_attempts") << std::endl;
                            break;
                        }
                    }
                }
                
                // 获取选定的课程
                std::string selectedCourseId = teacherCourseIds[courseIndex-1];
                Course* selectedCourse = courseManager.getCourse(selectedCourseId);
                
                if (selectedCourse) {
                    // 获取该课程的所有选课记录
                    std::vector<Enrollment*> enrollments = enrollmentManager.getCourseEnrollments(selectedCourseId);
                    
                    if (enrollments.empty()) {
                        std::cout << getText("no_course_students") << std::endl;
                    } else {
                        std::cout << getFormattedText("course_students", selectedCourse->getName()) << std::endl;
                        std::cout << "--------------------------------" << std::endl;
                        std::cout << getText("user_id") << "\t" 
                                  << getText("user_name") << "\t" 
                                  << getText("class") << "\t" 
                                  << getText("department") << "\t"
                                  << getText("enrollment_time") << std::endl;
                        for (Enrollment* enrollment : enrollments) {
                            std::string studentId = enrollment->getStudentId();
                            Student* student = static_cast<Student*>(userManager.getStudent(studentId));
                            
                            if (student) {
                                std::cout << student->getId() << "\t"
                                          << student->getName() << "\t"
                                          << student->getClassInfo() << "\t"
                                          << student->getDepartment() << "\t"
                                          << enrollment->getEnrollmentTime() << std::endl;
                            }
                        }
                        std::cout << "--------------------------------" << std::endl;
                        std::cout << getFormattedText("enrolled_student_count", static_cast<int>(enrollments.size())) << std::endl;
                    }
                }
            }
            
            std::cout << getText("press_enter_to_continue") << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
            
        default:
            std::cout << getText("invalid_choice") << std::endl;
            break;
    }
}

void CourseSystem::handleUserInfoModification() {
    if (!currentUser_) {
        std::cout << getText("operation_failed") << ": " << getText("not_logged_in") << std::endl;
        Logger::getInstance().error("修改账户信息失败：用户未登录");
        return;
    }
    
    // 获取当前用户ID
    std::string userId = currentUser_->getId();
    UserType userType = currentUser_->getType();
    
    std::cout << "========= " << getText("modify_user_info") << " =========" << std::endl;
    std::cout << getText("current_user_info") << ":" << std::endl;
    std::cout << getText("user_id") << ": " << userId << std::endl;
    std::cout << getText("user_name") << ": " << currentUser_->getName() << std::endl;
    
    // 根据用户类型显示不同的信息
    if (userType == UserType::STUDENT) {
        Student* student = dynamic_cast<Student*>(currentUser_);
        std::cout << getText("gender") << ": " << student->getGender() << std::endl;
        std::cout << getText("age") << ": " << student->getAge() << std::endl;
        std::cout << getText("department") << ": " << student->getDepartment() << std::endl;
        std::cout << getText("class") << ": " << student->getClassInfo() << std::endl;
        std::cout << getText("contact") << ": " << student->getContact() << std::endl;
    } else if (userType == UserType::TEACHER) {
        Teacher* teacher = dynamic_cast<Teacher*>(currentUser_);
        std::cout << getText("department") << ": " << teacher->getDepartment() << std::endl;
        std::cout << getText("title") << ": " << teacher->getTitle() << std::endl;
        std::cout << getText("contact") << ": " << teacher->getContact() << std::endl;
    } else if (userType == UserType::ADMIN) {
        // 管理员没有额外信息
    }
    
    std::cout << "-------------------------------" << std::endl;
    
    // 显示可修改的信息选项
    std::cout << getText("select_modify_content") << ":" << std::endl;
    std::cout << "1. " << getText("user_name") << std::endl;
    
    int maxOption = 1;
    
    if (userType == UserType::STUDENT) {
        std::cout << "2. " << getText("gender") << std::endl;
        std::cout << "3. " << getText("age") << std::endl;
        std::cout << "4. " << getText("department") << std::endl;
        std::cout << "5. " << getText("class") << std::endl;
        std::cout << "6. " << getText("contact") << std::endl;
        maxOption = 6;
    } else if (userType == UserType::TEACHER) {
        std::cout << "2. " << getText("department") << std::endl;
        std::cout << "3. " << getText("title") << std::endl;
        std::cout << "4. " << getText("contact") << std::endl;
        maxOption = 4;
    }
    
    std::cout << (maxOption + 1) << ". " << getText("return") << std::endl;
    
    // 获取用户选择
    int choice = 0;
    std::string input;
    int attempts = 0;
    const int MAX_ATTEMPTS = 3;
    
    while (attempts < MAX_ATTEMPTS) {
        std::cout << "> ";
        std::getline(std::cin, input);
        attempts++; 
        
        if (InputValidator::validateChoice(input, 1, maxOption + 1, choice)) {
            break;
        } else {
            std::cout << getText("invalid_input") << std::endl;
            if (attempts >= MAX_ATTEMPTS) {
                std::cout << getText("too_many_attempts") << std::endl;
                break;
            }
        }
    }
    
    // 返回上级菜单
    if (choice == maxOption + 1) {
        return;
    }
    
    // 修改信息
    std::string newValue;
    int newAge;
    bool updateSuccess = false;
    
    switch (userType) {
        case UserType::STUDENT: {
            Student* student = dynamic_cast<Student*>(currentUser_);
            
            switch (choice) {
                case 1: // 修改名字
                    std::cout << getText("enter_username") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        student->setName(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
                case 2: // 修改性别
                    std::cout << getText("enter_user_gender") << " (1-" << getText("male") << " 2-" << getText("female") << "): ";
                    std::getline(std::cin, newValue);
                    if (newValue == "1") {
                        student->setGender(getText("male"));
                    } else if (newValue == "2") {
                        student->setGender(getText("female"));
                    } else {
                        std::cout << getText("invalid_gender") << std::endl;
                        return;
                    }
                    break;
                case 3: // 修改年龄
                    std::cout << getText("enter_student_age") << ": ";
                    std::getline(std::cin, newValue);
                    if (InputValidator::validateInteger(newValue, 15, 80, newAge)) {
                        student->setAge(newAge);
                    } else {
                        std::cout << getText("invalid_age") << std::endl;
                        return;
                    }
                    break;
                case 4: // 修改系别
                    std::cout << getText("enter_department") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        student->setDepartment(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
                case 5: // 修改班级信息
                    std::cout << getText("enter_class_info") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        student->setClassInfo(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
                case 6: // 修改联系方式
                    std::cout << getText("enter_email") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        student->setContact(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
            }
            
            // 更新用户信息
            updateSuccess = UserManager::getInstance().updateUserInfo(*student);
            break;
        }
        case UserType::TEACHER: {
            Teacher* teacher = dynamic_cast<Teacher*>(currentUser_);
            
            switch (choice) {
                case 1: // 修改名字
                    std::cout << getText("enter_username") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        teacher->setName(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
                case 2: // 修改系别
                    std::cout << getText("enter_teacher_department") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        teacher->setDepartment(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
                case 3: // 修改职称
                    std::cout << getText("enter_teacher_title") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        teacher->setTitle(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
                case 4: // 修改联系方式
                    std::cout << getText("enter_email") << ": ";
                    std::getline(std::cin, newValue);
                    if (!newValue.empty()) {
                        teacher->setContact(newValue);
                    }
                    else{
                        std::cout << getText("invalid_input") << std::endl;
                        return;
                    }
                    break;
            }
            
            // 更新用户信息
            updateSuccess = UserManager::getInstance().updateUserInfo(*teacher);
            break;
        }
        case UserType::ADMIN: {
            Admin* admin = dynamic_cast<Admin*>(currentUser_);
            
            if (choice == 1) { // 修改名字
                std::cout << getText("enter_username") << ": ";
                std::getline(std::cin, newValue);
                if (!newValue.empty()) {
                    admin->setName(newValue);
                }
                else{
                    std::cout << getText("invalid_input") << std::endl;
                    return;
                }
                
                // 更新用户信息
                updateSuccess = UserManager::getInstance().updateUserInfo(*admin);
            }
            break;
        }
        default:
            break;
    }
    
    if (updateSuccess) {
        std::cout << getText("operation_success") << std::endl;
    } else {
        std::cout << getText("operation_failed") << std::endl;
    }
    
    std::cout << getText("press_enter_to_continue") << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
