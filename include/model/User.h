#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>

/**
 * @brief 用户类型枚举
 */
enum class UserType {
    STUDENT,    ///< 学生用户
    TEACHER,    ///< 教师用户
    ADMIN       ///< 管理员用户
};

/**
 * @brief 用户基类，提供用户基本信息和认证功能
 */
class User {
public:
    /**
     * @brief 默认构造函数
     */
    User() = default;
    
    /**
     * @brief 构造函数
     * @param id 用户ID
     * @param name 用户姓名
     * @param password 用户密码
     */
    User(std::string id, std::string name, std::string password);
    
    /**
     * @brief 虚析构函数
     */
    virtual ~User() = default;
    
    /**
     * @brief 移动构造函数
     * @param other 源对象
     */
    User(User&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     * @param other 源对象
     * @return 当前对象引用
     */
    User& operator=(User&& other) noexcept;
    
    /**
     * @brief 禁用拷贝构造函数
     */
    User(const User&) = delete;
    
    /**
     * @brief 禁用拷贝赋值运算符
     */
    User& operator=(const User&) = delete;

    /**
     * @brief 验证密码
     * @param password 待验证的密码
     * @return 验证结果
     */
    bool verifyPassword(const std::string& password) const;
    
    /**
     * @brief 获取用户类型
     * @return 用户类型
     */
    virtual UserType getType() const = 0;
    
    /**
     * @brief 获取用户ID
     * @return 用户ID
     */
    const std::string& getId() const { return id_; }
    
    /**
     * @brief 获取用户姓名
     * @return 用户姓名
     */
    const std::string& getName() const { return name_; }
    
    /**
     * @brief 设置用户姓名
     * @param name 新的用户姓名
     */
    void setName(std::string name) { name_ = std::move(name); }
    
    /**
     * @brief 设置用户密码
     * @param password 新密码
     */
    void setPassword(std::string password);

protected:
    std::string id_;       ///< 用户ID
    std::string name_;     ///< 用户姓名
    std::string password_; ///< 密码(SHA-256哈希)
    std::string salt_;     ///< 密码盐值
    
    /**
     * @brief 生成密码哈希
     * @param password 原始密码
     * @param salt 盐值
     * @return 哈希密码
     */
    static std::string generatePasswordHash(const std::string& password, const std::string& salt);
    
    /**
     * @brief 生成随机盐值
     * @return 盐值字符串
     */
    static std::string generateSalt();
};

/**
 * @brief 学生用户类
 */
class Student : public User {
public:
    /**
     * @brief 默认构造函数
     */
    Student() = default;
    
    /**
     * @brief 构造函数
     * @param id 学生ID
     * @param name 学生姓名
     * @param password 密码
     * @param gender 性别
     * @param age 年龄
     * @param department 系别
     * @param classInfo 班级信息
     * @param contact 联系方式
     */
    Student(std::string id, std::string name, std::string password,
            std::string gender, int age, std::string department,
            std::string classInfo, std::string contact);
    
    /**
     * @brief 移动构造函数
     * @param other 源对象
     */
    Student(Student&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     * @param other 源对象
     * @return 当前对象引用
     */
    Student& operator=(Student&& other) noexcept;
    
    /**
     * @brief 禁用拷贝构造函数
     */
    Student(const Student&) = delete;
    
    /**
     * @brief 禁用拷贝赋值运算符
     */
    Student& operator=(const Student&) = delete;
    
    /**
     * @brief 获取用户类型
     * @return 用户类型(学生)
     */
    UserType getType() const override { return UserType::STUDENT; }
    
    // Getters and setters
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
    std::string gender_;     ///< 性别
    int age_ = 0;            ///< 年龄
    std::string department_; ///< 系别
    std::string classInfo_;  ///< 班级信息
    std::string contact_;    ///< 联系方式
};

/**
 * @brief 教师用户类
 */
class Teacher : public User {
public:
    /**
     * @brief 默认构造函数
     */
    Teacher() = default;
    
    /**
     * @brief 构造函数
     * @param id 教师ID
     * @param name 教师姓名
     * @param password 密码
     * @param department 系别
     * @param title 职称
     * @param contact 联系方式
     */
    Teacher(std::string id, std::string name, std::string password,
            std::string department, std::string title, std::string contact);
    
    /**
     * @brief 移动构造函数
     * @param other 源对象
     */
    Teacher(Teacher&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     * @param other 源对象
     * @return 当前对象引用
     */
    Teacher& operator=(Teacher&& other) noexcept;
    
    /**
     * @brief 禁用拷贝构造函数
     */
    Teacher(const Teacher&) = delete;
    
    /**
     * @brief 禁用拷贝赋值运算符
     */
    Teacher& operator=(const Teacher&) = delete;
    
    /**
     * @brief 获取用户类型
     * @return 用户类型(教师)
     */
    UserType getType() const override { return UserType::TEACHER; }
    
    // Getters and setters
    const std::string& getDepartment() const { return department_; }
    void setDepartment(std::string department) { department_ = std::move(department); }
    
    const std::string& getTitle() const { return title_; }
    void setTitle(std::string title) { title_ = std::move(title); }
    
    const std::string& getContact() const { return contact_; }
    void setContact(std::string contact) { contact_ = std::move(contact); }

private:
    std::string department_; ///< 系别
    std::string title_;      ///< 职称
    std::string contact_;    ///< 联系方式
};

/**
 * @brief 管理员用户类
 */
class Admin : public User {
public:
    /**
     * @brief 默认构造函数
     */
    Admin() = default;
    
    /**
     * @brief 构造函数
     * @param id 管理员ID
     * @param name 管理员姓名
     * @param password 密码
     */
    Admin(std::string id, std::string name, std::string password);
    
    /**
     * @brief 移动构造函数
     * @param other 源对象
     */
    Admin(Admin&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     * @param other 源对象
     * @return 当前对象引用
     */
    Admin& operator=(Admin&& other) noexcept;
    
    /**
     * @brief 禁用拷贝构造函数
     */
    Admin(const Admin&) = delete;
    
    /**
     * @brief 禁用拷贝赋值运算符
     */
    Admin& operator=(const Admin&) = delete;
    
    /**
     * @brief 获取用户类型
     * @return 用户类型(管理员)
     */
    UserType getType() const override { return UserType::ADMIN; }
}; 