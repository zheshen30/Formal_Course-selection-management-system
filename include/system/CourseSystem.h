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

class CourseSystem {
public:
    static CourseSystem& getInstance();
    
    bool initialize(const std::string& dataDir);
    
    int run();
    
    void shutdown();

    bool login(const std::string& userId, const std::string& password);
    
    void logout();
    
    Language getCurrentLanguage() const;

    std::string getText(const std::string& key) const;

    template<typename... Args>
    std::string getFormattedText(const std::string& key, Args... args) const;

    bool changePassword(const std::string& userId, const std::string& oldPassword,
                        const std::string& newPassword, const std::string& confirmPassword);

    void log(LogLevel level, const std::string& message) const;

private:
    
    CourseSystem();
    
    CourseSystem(const CourseSystem&) = delete;
    
    CourseSystem& operator=(const CourseSystem&) = delete;
    
    void showWelcome() const;
        
    void showMainMenu();
    
    void showLanguageMenu();
    
    void showAdminMenu();
    
    void showTeacherMenu();
    
    void showStudentMenu();
    
    void handleAdminFunctions(int choice);
    
    void handleTeacherFunctions(int choice);
    
    void handleStudentFunctions(int choice);

    void handlePasswordChange();
    
    void handleUserInfoModification();

    bool initialized_ = false;      // 是否已初始化
    bool running_ = false;          // 是否正在运行
    User* currentUser_ = nullptr;   // 当前登录用户
}; 