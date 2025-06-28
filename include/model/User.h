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

#include <string>
#include <memory>
#include <vector>
#include <utility>


enum class UserType {
    STUDENT,    // 学生用户
    TEACHER,    // 教师用户
    ADMIN       // 管理员用户
};

class User {
public:
    User() = default;
            
    User(std::string id, std::string name, std::string password);
    
    virtual ~User() = default;
    
    User(User&& other) noexcept;
    
    User& operator=(User&& other) noexcept;
    
    User(const User&) = delete;
    
    User& operator=(const User&) = delete;

    bool verifyPassword(const std::string& password) const;
    
    virtual UserType getType() const = 0;
    
    const std::string& getId() const { return id_; }
    
    const std::string& getName() const { return name_; }
    
    void setName(std::string name) { name_ = std::move(name); }

    void setPassword(std::string password);
    
    const std::string& getSalt() const { return salt_; }

protected:
    std::string id_;       // 用户ID
    std::string name_;     // 用户姓名
    std::string password_; // 密码(SHA-256哈希)
    std::string salt_;     // 密码盐值
    
    static std::string generatePasswordHash(const std::string& password, const std::string& salt);
    
    static std::string generateSalt();
    
    friend class UserManager;
};


class Student : public User {
public:
    
    Student() = default;
    
    Student(std::string id, std::string name, std::string password,
            std::string gender, int age, std::string department,
            std::string classInfo, std::string contact);
    
    Student(Student&& other) noexcept;

    Student& operator=(Student&& other) noexcept;
    
    Student(const Student&) = delete;
    
    Student& operator=(const Student&) = delete;
    
     // Getters and setters
    UserType getType() const override { return UserType::STUDENT; }
    
    const std::string& getGender() const { return gender_; }
    void setGender(std::string gender) { gender_ = std::move(gender); }
    
    int getAge() const { return age_; }
    void setAge(int age) { age_ = age; }
    
    const std::string& getDepartment() const { return department_; }
    void setDepartment(std::string department) { department_ = std::move(department); }
    
    const std::string& getClassInfo() const { return classInfo_; }
    void setClassInfo(std::string classInfo) { classInfo_ = std::move(classInfo); }
    
    const std::string& getContact() const { return contact_; }
    void setContact(std::string contact) { contact_ = std::move(contact); }

private:
    std::string gender_;     // 性别
    int age_ = 0;            // 年龄
    std::string department_; // 系别
    std::string classInfo_;  // 班级信息
    std::string contact_;    // 联系方式
};


class Teacher : public User {
public:
    Teacher() = default;
    
    Teacher(std::string id, std::string name, std::string password,
            std::string department, std::string title, std::string contact);

    Teacher(Teacher&& other) noexcept;
    
    Teacher& operator=(Teacher&& other) noexcept;

    Teacher(const Teacher&) = delete;
    
    Teacher& operator=(const Teacher&) = delete;

    UserType getType() const override { return UserType::TEACHER; }
    
    // Getters and setters
    const std::string& getDepartment() const { return department_; }
    void setDepartment(std::string department) { department_ = std::move(department); }
    
    const std::string& getTitle() const { return title_; }
    void setTitle(std::string title) { title_ = std::move(title); }
    
    const std::string& getContact() const { return contact_; }
    void setContact(std::string contact) { contact_ = std::move(contact); }

private:
    std::string department_; // 系别
    std::string title_;      // 职称
    std::string contact_;    // 联系方式
};

 
class Admin : public User {
public:
    
    Admin() = default;
    
    Admin(std::string id, std::string name, std::string password);
    
    Admin(Admin&& other) noexcept;
    
    Admin& operator=(Admin&& other) noexcept;
    
    Admin(const Admin&) = delete;
    
    Admin& operator=(const Admin&) = delete;
    
    UserType getType() const override { return UserType::ADMIN; }
}; 