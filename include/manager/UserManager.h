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
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <functional>

class UserManager {
public:
    
    static UserManager& getInstance();

    
    bool addStudent(std::unique_ptr<Student> student);

    
    bool addTeacher(std::unique_ptr<Teacher> teacher);

    
    bool addAdmin(std::unique_ptr<Admin> admin);

    
    bool removeUser(const std::string& userId);

    
    User* getUser(const std::string& userId);

    
    Student* getStudent(const std::string& studentId);


    Teacher* getTeacher(const std::string& teacherId);

    
    Admin* getAdmin(const std::string& adminId);

    
    User* authenticate(const std::string& userId, const std::string& password);

    
    std::vector<std::string> getAllStudentIds() const;

    
    std::vector<std::string> getAllTeacherIds() const;

    
    std::vector<std::string> getAllAdminIds() const;

    
    bool loadData();

    
    bool saveData(bool alreadyLocked);

    
    bool updateUserInfo(const User& user);


    bool hasUser(const std::string& userId) const;

    
    bool changeUserPassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword);

    
    std::vector<std::string> findUsers(const std::function<bool(const User&)>& predicate) const;

private:
    
    UserManager() = default;
    
    
    UserManager(const UserManager&) = delete;
    
    
    UserManager& operator=(const UserManager&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<User>> users_; // 用户映射表
    mutable std::mutex mutex_; // 互斥锁
    
    // 添加用户
    bool addUser(std::unique_ptr<User> user);
}; 