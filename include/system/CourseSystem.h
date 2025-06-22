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
#pragma once

#include "../model/User.h"
#include "../model/Course.h"
#include "../model/Enrollment.h"
#include "../manager/UserManager.h"
#include "../manager/CourseManager.h"
#include "../manager/EnrollmentManager.h"
#include "../util/Logger.h"
#include "../util/I18nManager.h"

#include <string>
#include <memory>
#include <vector>
#include <functional>

/**
 * @brief 课程系统主类，系统核心控制器
 */
class CourseSystem {
public:
    /**
     * @brief 获取单例实例
     * @return CourseSystem单例引用
     */
    static CourseSystem& getInstance();
    
    /**
     * @brief 初始化系统
     * @param dataDir 数据目录
     * @param logDir 日志目录
     * @return 是否初始化成功
     */
    bool initialize(const std::string& dataDir, const std::string& logDir);
    
    /**
     * @brief 运行系统
     * @return 退出码
     */
    int run();
    
    /**
     * @brief 关闭系统
     */
    void shutdown();

    /**
     * @brief 用户登录
     * @param userId 用户ID
     * @param password 密码
     * @return 是否登录成功
     */
    bool login(const std::string& userId, const std::string& password);
    
    /**
     * @brief 用户注销
     */
    void logout();
    
    /**
     * @brief 获取当前用户
     * @return 当前登录用户，未登录时返回nullptr
     */
    User* getCurrentUser() const;
    
    /**
     * @brief 检查用户是否有权限执行操作
     * @param requiredUserType 所需用户类型
     * @return 是否有权限
     */
    bool checkPermission(UserType requiredUserType) const;

    /**
     * @brief 选择语言
     * @param language 语言枚举
     * @return 是否设置成功
     */
    bool selectLanguage(Language language);

    /**
     * @brief 获取当前语言
     * @return 当前语言枚举
     */
    Language getCurrentLanguage() const;

    /**
     * @brief 获取翻译文本
     * @param key 文本键
     * @return 翻译后的文本
     */
    std::string getText(const std::string& key) const;

    /**
     * @brief 获取带参数的翻译文本
     * @param key 文本键
     * @param args 参数值
     * @return 翻译后的文本
     */
    template<typename... Args>
    std::string getFormattedText(const std::string& key, Args... args) const;

    /**
     * @brief 修改用户密码
     * @param userId 用户ID
     * @param oldPassword 旧密码
     * @param newPassword 新密码
     * @param confirmPassword 确认密码
     * @return 是否修改成功
     */
    bool changePassword(const std::string& userId, const std::string& oldPassword,
                        const std::string& newPassword, const std::string& confirmPassword);

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(LogLevel level, const std::string& message) const;

private:
    /**
     * @brief 私有构造函数，确保单例
     */
    CourseSystem();
    
    /**
     * @brief 删除拷贝构造函数
     */
    CourseSystem(const CourseSystem&) = delete;
    
    /**
     * @brief 删除赋值运算符
     */
    CourseSystem& operator=(const CourseSystem&) = delete;
    
    /**
     * @brief 显示欢迎界面
     */
    void showWelcome() const;
    
    /**
     * @brief 显示主菜单
     */
    void showMainMenu();
    
    /**
     * @brief 显示管理员菜单
     */
    void showAdminMenu();
    
    /**
     * @brief 显示教师菜单
     */
    void showTeacherMenu();
    
    /**
     * @brief 显示学生菜单
     */
    void showStudentMenu();
    
    /**
     * @brief 处理管理员功能
     * @param choice 选择项
     */
    void handleAdminFunctions(int choice);
    
    /**
     * @brief 处理教师功能
     * @param choice 选择项
     */
    void handleTeacherFunctions(int choice);
    
    /**
     * @brief 处理学生功能
     * @param choice 选择项
     */
    void handleStudentFunctions(int choice);

    bool initialized_ = false;      ///< 是否已初始化
    bool running_ = false;          ///< 是否正在运行
    User* currentUser_ = nullptr;   ///< 当前登录用户
}; 