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

/**
 * @brief 用户管理类，负责用户的CRUD操作和认证
 */
class UserManager {
public:
    /**
     * @brief 获取单例实例
     * @return UserManager单例引用
     */
    static UserManager& getInstance();

    /**
     * @brief 添加学生
     * @param student 学生对象
     * @return 是否添加成功
     */
    bool addStudent(std::unique_ptr<Student> student);

    /**
     * @brief 添加教师
     * @param teacher 教师对象
     * @return 是否添加成功
     */
    bool addTeacher(std::unique_ptr<Teacher> teacher);

    /**
     * @brief 添加管理员
     * @param admin 管理员对象
     * @return 是否添加成功
     */
    bool addAdmin(std::unique_ptr<Admin> admin);

    /**
     * @brief 移除用户
     * @param userId 用户ID
     * @return 是否移除成功
     */
    bool removeUser(const std::string& userId);

    /**
     * @brief 获取用户
     * @param userId 用户ID
     * @return 用户指针，不存在则返回nullptr
     */
    User* getUser(const std::string& userId);

    /**
     * @brief 获取学生
     * @param studentId 学生ID
     * @return 学生指针，不存在则返回nullptr
     */
    Student* getStudent(const std::string& studentId);

    /**
     * @brief 获取教师
     * @param teacherId 教师ID
     * @return 教师指针，不存在则返回nullptr
     */
    Teacher* getTeacher(const std::string& teacherId);

    /**
     * @brief 获取管理员
     * @param adminId 管理员ID
     * @return 管理员指针，不存在则返回nullptr
     */
    Admin* getAdmin(const std::string& adminId);

    /**
     * @brief 用户认证
     * @param userId 用户ID
     * @param password 密码
     * @return 认证成功的用户指针，失败则返回nullptr
     */
    User* authenticate(const std::string& userId, const std::string& password);

    /**
     * @brief 获取所有学生
     * @return 学生ID列表
     */
    std::vector<std::string> getAllStudentIds() const;

    /**
     * @brief 获取所有教师
     * @return 教师ID列表
     */
    std::vector<std::string> getAllTeacherIds() const;

    /**
     * @brief 获取所有管理员
     * @return 管理员ID列表
     */
    std::vector<std::string> getAllAdminIds() const;

    /**
     * @brief 加载用户数据
     * @return 是否加载成功
     */
    bool loadData();

    /**
     * @brief 保存用户数据
     * @return 是否保存成功
     */
    bool saveData();

    /**
     * @brief 更新用户信息
     * @param user 用户对象
     * @return 是否更新成功
     */
    bool updateUserInfo(const User& user);

    /**
     * @brief 检查用户是否存在
     * @param userId 用户ID
     * @return 是否存在
     */
    bool hasUser(const std::string& userId) const;

    /**
     * @brief 查找用户
     * @param predicate 过滤谓词
     * @return 符合条件的用户ID列表
     */
    std::vector<std::string> findUsers(const std::function<bool(const User&)>& predicate) const;

private:
    /**
     * @brief 私有构造函数，确保单例
     */
    UserManager() = default;
    
    /**
     * @brief 删除拷贝构造函数
     */
    UserManager(const UserManager&) = delete;
    
    /**
     * @brief 删除赋值运算符
     */
    UserManager& operator=(const UserManager&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<User>> users_; ///< 用户映射表
    mutable std::mutex mutex_; ///< 互斥锁
    
    /**
     * @brief 添加用户
     * @param user 用户对象
     * @return 是否添加成功
     */
    bool addUser(std::unique_ptr<User> user);
}; 