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
#include "../../include/model/User.h"
#include "../../include/system/SystemException.h"

// OpenSSL库头文件
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h> // 用于版本检查

#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <utility>
#include <functional> // 用于std::hash
#include <iostream>

// User 类实现
User::User(std::string id, std::string name, std::string password)
    : id_(std::move(id)), name_(std::move(name)) {
    // 对特定用户进行特殊处理
    if (id_ == "admin001" || id_ == "teacher001" || id_ == "student001") {
        // 特殊用户处理 - 使用不带盐值的纯密码哈希
        salt_ = "";
        // 计算纯哈希值
        password_ = generatePasswordHash(password, "");
    } else {
        // 标准处理方式
        salt_ = generateSalt();
        password_ = generatePasswordHash(password, salt_);
    }
}

User::User(User&& other) noexcept
    : id_(std::move(other.id_)),
      name_(std::move(other.name_)),
      password_(std::move(other.password_)),
      salt_(std::move(other.salt_)) {
}

User& User::operator=(User&& other) noexcept {
    if (this != &other) {
        id_ = std::move(other.id_);
        name_ = std::move(other.name_);
        password_ = std::move(other.password_);
        salt_ = std::move(other.salt_);
    }
    return *this;
}

bool User::verifyPassword(const std::string& password) const {
    // 方法1：纯哈希值比较（针对特殊账户且盐值为空）
    if (salt_.empty()) {
        if (id_ == "admin001" || id_ == "teacher001" || id_ == "student001") {
            std::cout << "特殊账户处理：使用纯哈希验证（无盐值）" << std::endl;
            // 计算纯哈希值并与存储的哈希值比较
            std::string pureHash = generatePasswordHash(password, "");
            std::cout << "  输入密码的纯哈希: " << pureHash << std::endl;
            std::cout << "  存储的哈希: " << password_ << std::endl;
            bool directMatch = (password_ == pureHash);
            std::cout << "  纯哈希匹配: " << (directMatch ? "是" : "否") << std::endl;
            return directMatch;
        }
    }
    
    // 方法2：密码和盐值拼接后哈希（标准方法）
    std::string combinedHash = generatePasswordHash(password, salt_);
    
    std::cout << "验证密码:" << std::endl;
    std::cout << "  用户ID: " << id_ << std::endl;
    std::cout << "  输入密码: " << password << std::endl;
    std::cout << "  盐值: " << salt_ << std::endl;
    std::cout << "  密码+盐值哈希: " << combinedHash << std::endl;
    std::cout << "  存储的哈希: " << password_ << std::endl;
    
    // 判断是否匹配
    bool combinedMatch = password_ == combinedHash;
    
    std::cout << "  哈希匹配: " << (combinedMatch ? "是" : "否") << std::endl;
    
    return combinedMatch;
}

void User::setPassword(std::string password) {
    // 为特殊账户设置一个标志，表示密码已被修改
    // 这样在verifyPassword中将不再使用特殊逻辑
    if (id_ == "admin001" || id_ == "teacher001" || id_ == "student001") {
        // 特殊账户修改密码时，将盐值设置为非空，这样就不会触发特殊逻辑了
        salt_ = generateSalt();
    } else {
        salt_ = generateSalt();
    }
    
    password_ = generatePasswordHash(password, salt_);
}

std::string User::generatePasswordHash(const std::string& password, const std::string& salt) {
    // 将密码和盐值拼接
    std::string combined = password + salt;
    
    // OpenSSL实现 - 使用EVP API
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
    // 使用OpenSSL 3.0推荐的EVP接口
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        throw SystemException(ErrorType::OPERATION_FAILED, "OpenSSL错误：无法创建EVP_MD_CTX");
    }
    
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw SystemException(ErrorType::OPERATION_FAILED, "OpenSSL错误：无法初始化摘要");
    }
    
    if (EVP_DigestUpdate(mdctx, combined.c_str(), combined.length()) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw SystemException(ErrorType::OPERATION_FAILED, "OpenSSL错误：无法更新摘要");
    }
    
    unsigned int hash_len = SHA256_DIGEST_LENGTH;
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw SystemException(ErrorType::OPERATION_FAILED, "OpenSSL错误：无法完成摘要");
    }
    
    EVP_MD_CTX_free(mdctx);
    #else
    // 旧版OpenSSL使用SHA256接口
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined.c_str(), combined.length());
    SHA256_Final(hash, &sha256);
    #endif
    
    // 将哈希转换为十六进制字符串
    std::stringstream ss;
    for (unsigned char i : hash) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);
    }
    
    return ss.str();
}

std::string User::generateSalt() {
    const int saltLength = 16; // 盐值长度
    const std::string chars = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, static_cast<int>(chars.size() - 1));
    
    std::string salt;
    salt.reserve(saltLength);
    
    for (int i = 0; i < saltLength; ++i) {
        salt += chars[distribution(generator)];
    }
    
    return salt;
}

// Student 类实现
Student::Student(std::string id, std::string name, std::string password,
                 std::string gender, int age, std::string department,
                 std::string classInfo, std::string contact)
    : User(std::move(id), std::move(name), std::move(password)),
      gender_(std::move(gender)),
      age_(age),
      department_(std::move(department)),
      classInfo_(std::move(classInfo)),
      contact_(std::move(contact)) {
}

Student::Student(Student&& other) noexcept
    : User(std::move(other)),
      gender_(std::move(other.gender_)),
      age_(other.age_),
      department_(std::move(other.department_)),
      classInfo_(std::move(other.classInfo_)),
      contact_(std::move(other.contact_)) {
    other.age_ = 0;
}

Student& Student::operator=(Student&& other) noexcept {
    if (this != &other) {
        User::operator=(std::move(other));
        gender_ = std::move(other.gender_);
        age_ = other.age_;
        department_ = std::move(other.department_);
        classInfo_ = std::move(other.classInfo_);
        contact_ = std::move(other.contact_);
        
        other.age_ = 0;
    }
    return *this;
}

// Teacher 类实现
Teacher::Teacher(std::string id, std::string name, std::string password,
                 std::string department, std::string title, std::string contact)
    : User(std::move(id), std::move(name), std::move(password)),
      department_(std::move(department)),
      title_(std::move(title)),
      contact_(std::move(contact)) {
}

Teacher::Teacher(Teacher&& other) noexcept
    : User(std::move(other)),
      department_(std::move(other.department_)),
      title_(std::move(other.title_)),
      contact_(std::move(other.contact_)) {
}

Teacher& Teacher::operator=(Teacher&& other) noexcept {
    if (this != &other) {
        User::operator=(std::move(other));
        department_ = std::move(other.department_);
        title_ = std::move(other.title_);
        contact_ = std::move(other.contact_);
    }
    return *this;
}

// Admin 类实现
Admin::Admin(std::string id, std::string name, std::string password)
    : User(std::move(id), std::move(name), std::move(password)) {
}

Admin::Admin(Admin&& other) noexcept
    : User(std::move(other)) {
}

Admin& Admin::operator=(Admin&& other) noexcept {
    User::operator=(std::move(other));
    return *this;
} 