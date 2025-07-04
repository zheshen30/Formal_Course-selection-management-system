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
#include "../../include/manager/UserManager.h"
#include "../../include/util/DataManager.h"
#include "../../include/system/LockGuard.h"
#include "../../include/system/SystemException.h"
#include "../../include/util/Logger.h"

#include "../../nlohmann/json.hpp"
#include <algorithm>
#include <vector>
#include <stdexcept>

using json = nlohmann::json;

UserManager& UserManager::getInstance() {
    static UserManager instance;
    return instance;
}

bool UserManager::addStudent(std::unique_ptr<Student> student) {
    if (!student) {
        Logger::getInstance().error("尝试添加空学生对象");
        return false;
    }
    //release返回unique_ptr的指针，并释放unique_ptr
    return addUser(std::unique_ptr<User>(student.release()));
}

bool UserManager::addTeacher(std::unique_ptr<Teacher> teacher) {
    if (!teacher) {
        Logger::getInstance().error("尝试添加空教师对象");
        return false;
    }
    
    return addUser(std::unique_ptr<User>(teacher.release()));
}

bool UserManager::addAdmin(std::unique_ptr<Admin> admin) {
    if (!admin) {
        Logger::getInstance().error("尝试添加空管理员对象");
        return false;
    }
    
    return addUser(std::unique_ptr<User>(admin.release()));
}

bool UserManager::addUser(std::unique_ptr<User> user) {
    if (!user) {
        return false;
    }
    
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    const std::string& userId = user->getId();
    if (users_.find(userId) != users_.end()) {
        Logger::getInstance().warning("添加用户失败：用户ID " + userId + " 已存在");
        return false;
    }
    
    users_[userId] = std::move(user);
    
    // 立即保存数据，传入true表示已获取锁
    bool saveResult = saveData(true);
    if (!saveResult) {
        Logger::getInstance().error("添加用户后保存数据失败");
        return false;
    }

     Logger::getInstance().info("成功添加用户: " + userId);
    return true;
}

bool UserManager::removeUser(const std::string& userId) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    auto it = users_.find(userId);
    if (it == users_.end()) {
        Logger::getInstance().warning("移除用户失败：用户ID " + userId + " 不存在");
        return false;
    }
    
    users_.erase(it);
        
    // 立即保存数据，传入true表示已获取锁
    bool saveResult = saveData(true);
    if (!saveResult) {
        Logger::getInstance().warning("移除用户后保存数据失败");
        return false;
    }

    Logger::getInstance().info("成功移除用户: " + userId);
    return true;
}

User* UserManager::getUser(const std::string& userId) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    auto it = users_.find(userId);
    if (it == users_.end()) {
        return nullptr;
    }
    
    //get原始指针
    return it->second.get();
}

Student* UserManager::getStudent(const std::string& studentId) {
    User* user = getUser(studentId);
    if (!user || user->getType() != UserType::STUDENT) {
        return nullptr;
    }
    
    return dynamic_cast<Student*>(user);
}

Teacher* UserManager::getTeacher(const std::string& teacherId) {
    User* user = getUser(teacherId);
    if (!user || user->getType() != UserType::TEACHER) {
        return nullptr;
    }
    
    return dynamic_cast<Teacher*>(user);
}

Admin* UserManager::getAdmin(const std::string& adminId) {
    User* user = getUser(adminId);
    if (!user || user->getType() != UserType::ADMIN) {
        return nullptr;
    }
    
    return dynamic_cast<Admin*>(user);
}

User* UserManager::authenticate(const std::string& userId, const std::string& password) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    auto it = users_.find(userId);
    if (it == users_.end()) {
        Logger::getInstance().warning("认证失败：用户ID " + userId + " 不存在");
        return nullptr;
    }
    
    User* user = it->second.get();
    if (!user->verifyPassword(password)) {
        Logger::getInstance().warning("认证失败：用户 " + userId + " 密码错误");
        return nullptr;
    }
    
    Logger::getInstance().info("用户 " + userId + " 认证成功");
    return user;
}

std::vector<std::string> UserManager::getAllStudentIds() const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    std::vector<std::string> studentIds;
    for (const auto& pair : users_) {
        if (pair.second->getType() == UserType::STUDENT) {
            studentIds.push_back(pair.first);
        }
    }
    
    return studentIds;
}

std::vector<std::string> UserManager::getAllTeacherIds() const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    std::vector<std::string> teacherIds;
    for (const auto& pair : users_) {
        if (pair.second->getType() == UserType::TEACHER) {
            teacherIds.push_back(pair.first);
        }
    }
    
    return teacherIds;
}

std::vector<std::string> UserManager::getAllAdminIds() const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    std::vector<std::string> adminIds;
    for (const auto& pair : users_) {
        if (pair.second->getType() == UserType::ADMIN) {
            adminIds.push_back(pair.first);
        }
    }
    
    return adminIds;
}

bool UserManager::loadData() {
    try {
        LockGuard lock(mutex_, 5000); // 设置5秒超时
        if (!lock.isLocked()) {
            throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
        }
        
        DataManager& dataManager = DataManager::getInstance(); // 获取单例
        std::string jsonStr = dataManager.loadJsonFromFile("users.json");
        
        if (jsonStr.empty()) {
            Logger::getInstance().warning("用户数据文件为空或不存在");
            return false;
        }
        
        json usersJson = json::parse(jsonStr);
        users_.clear();
        
        for (const auto& userJson : usersJson) {
            std::string id = userJson["id"];
            std::string name = userJson["name"];
            std::string password = userJson["password"];
            std::string salt = userJson["salt"];
            std::string typeStr = userJson["type"];
            
            std::unique_ptr<User> user;
            
            if (typeStr == "STUDENT") {
                auto student = std::make_unique<Student>();
                student->setGender(userJson["gender"]);
                student->setAge(userJson["age"]);
                student->setDepartment(userJson["department"]);
                student->setClassInfo(userJson["classInfo"]);
                student->setContact(userJson["contact"]);
                user = std::move(student);
            } else if (typeStr == "TEACHER") {
                auto teacher = std::make_unique<Teacher>();
                teacher->setDepartment(userJson["department"]);
                teacher->setTitle(userJson["title"]);
                teacher->setContact(userJson["contact"]);
                user = std::move(teacher);
            } else if (typeStr == "ADMIN") {
                user = std::make_unique<Admin>();
            } else {
                Logger::getInstance().warning("未知的用户类型：" + typeStr);
                continue;
            }
            
            // 设置通用属性
            user->id_ = id;
            user->name_ = name;
            user->password_ = password;
            user->salt_ = salt;
            
            users_[id] = std::move(user);
        }
        
        Logger::getInstance().info("成功加载用户数据，共 " + std::to_string(users_.size()) + " 个用户");
        return true;
    } catch (const json::exception& e) {
        Logger::getInstance().error("解析用户数据JSON失败：" + std::string(e.what()));
        throw SystemException(ErrorType::DATA_INVALID, "解析用户数据失败：" + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::getInstance().error("加载用户数据失败：" + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "加载用户数据失败：" + std::string(e.what()));
    }
}

bool UserManager::saveData(bool alreadyLocked) {
    try {
        json usersJson = json::array();
        std::string jsonStr;
        {
            // 指向锁的智能指针，实现条件性锁定和作用域控制
            std::unique_ptr<LockGuard> lockPtr;
            if (!alreadyLocked) {
                // 在堆上创建锁
                lockPtr = std::make_unique<LockGuard>(mutex_, 5000);
                if (!lockPtr->isLocked()) {
                    throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
                }
            }

            for (const auto& pair : users_) {
                const User* user = pair.second.get();
                json userJson;
                
                // 通用属性
                userJson["id"] = user->getId();
                userJson["name"] = user->getName();
                userJson["password"] = user->password_;
                userJson["salt"] = user->salt_;
                
                switch (user->getType()) {
                    case UserType::STUDENT: {
                        const Student* student = static_cast<const Student*>(user);
                        userJson["type"] = "STUDENT";
                        userJson["gender"] = student->getGender();
                        userJson["age"] = student->getAge();
                        userJson["department"] = student->getDepartment();
                        userJson["classInfo"] = student->getClassInfo();
                        userJson["contact"] = student->getContact();
                        break;
                    }
                    case UserType::TEACHER: {
                        const Teacher* teacher = static_cast<const Teacher*>(user);
                        userJson["type"] = "TEACHER";
                        userJson["department"] = teacher->getDepartment();
                        userJson["title"] = teacher->getTitle();
                        userJson["contact"] = teacher->getContact();
                        break;
                    }
                    case UserType::ADMIN:
                        userJson["type"] = "ADMIN";
                        break;
                    default:
                        Logger::getInstance().warning("未知的用户类型：" + std::to_string(static_cast<int>(user->getType())));
                        continue;
                }
                
                usersJson.push_back(userJson);
            }
            
            jsonStr = usersJson.dump(4); // 格式化JSON，缩进4个空格
        } // 如果创建了锁，锁会在这里释放
        
        // 第二阶段：在锁释放后保存数据到文件
        DataManager& dataManager = DataManager::getInstance();
        bool result = dataManager.saveJsonToFile("users.json", jsonStr);
        
        if (result) {
            Logger::getInstance().info("成功保存用户数据，共 " + std::to_string(usersJson.size()) + " 个用户");
        } else {
            Logger::getInstance().error("保存用户数据失败");
        }
        
        return result;
    } catch (const json::exception& e) {
        Logger::getInstance().error("生成用户数据JSON失败：" + std::string(e.what()));
        throw SystemException(ErrorType::DATA_INVALID, "生成用户数据失败：" + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::getInstance().error("保存用户数据失败：" + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "保存用户数据失败：" + std::string(e.what()));
    }
}

bool UserManager::updateUserInfo(const User& user) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }

    auto it = users_.find(user.getId());
    if (it == users_.end()) {
        Logger::getInstance().warning("更新用户信息失败：用户ID " + user.getId() + " 不存在");
        return false;
    }
    
    // 根据用户类型，执行不同的更新操作
    switch (user.getType()) {
        case UserType::STUDENT: {
            Student* existingStudent = dynamic_cast<Student*>(it->second.get());
            const Student& student = dynamic_cast<const Student&>(user);
            
            existingStudent->setName(student.getName());
            existingStudent->setGender(student.getGender());
            existingStudent->setAge(student.getAge());
            existingStudent->setDepartment(student.getDepartment());
            existingStudent->setClassInfo(student.getClassInfo());
            existingStudent->setContact(student.getContact());
            break;
        }
        case UserType::TEACHER: {
            Teacher* existingTeacher = dynamic_cast<Teacher*>(it->second.get());
            const Teacher& teacher = dynamic_cast<const Teacher&>(user);
            
            existingTeacher->setName(teacher.getName());
            existingTeacher->setDepartment(teacher.getDepartment());
            existingTeacher->setTitle(teacher.getTitle());
            existingTeacher->setContact(teacher.getContact());
            break;
        }
        case UserType::ADMIN: {
            Admin* existingAdmin = dynamic_cast<Admin*>(it->second.get());
            const Admin& admin = dynamic_cast<const Admin&>(user);
            
            existingAdmin->setName(admin.getName());
            break;
        }
        default:
            Logger::getInstance().warning("未知的用户类型：" + std::to_string(static_cast<int>(user.getType())));
            return false;
    }
    
    // 立即保存数据，传入true表示已获取锁
    bool saveResult = saveData(true);
    if (!saveResult) {
        Logger::getInstance().warning("更新用户信息后保存数据失败");
    }
    Logger::getInstance().info("成功更新用户信息: " + user.getId());
    
    return true;
}

bool UserManager::hasUser(const std::string& userId) const {
    LockGuard lock(mutex_, 5000);
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    return users_.find(userId) != users_.end();
}

bool UserManager::changeUserPassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword) {
    try {
        LockGuard lock(mutex_, 5000); // 设置5秒超时
        if (!lock.isLocked()) {
            throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
        }
        
        auto it = users_.find(userId);
        if (it == users_.end()) {
            Logger::getInstance().warning("修改密码失败：用户ID " + userId + " 不存在");
            return false;
        }
        
        User* user = it->second.get();
        
        if (!user->verifyPassword(oldPassword)) {
            Logger::getInstance().warning("修改密码失败：用户 " + userId + " 原密码验证失败");
            return false;
        }
        
        user->setPassword(newPassword);
        
        // 立即保存数据，传入true表示已获取锁
        bool saveResult = saveData(true);
        if (!saveResult) {
            Logger::getInstance().warning("修改密码后保存数据失败");
            return false;
        }

        Logger::getInstance().info("用户 " + userId + " 密码修改成功");
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error("修改密码出现异常: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> UserManager::findUsers(const std::function<bool(const User&)>& predicate) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取用户管理器锁超时");
    }
    
    std::vector<std::string> result;
    for (const auto& pair : users_) {
        if (predicate(*(pair.second))) {
            result.push_back(pair.first);
        }
    }
    
    return result;
}
