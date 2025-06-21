#include "../../include/model/User.h"
#include "../../include/system/SystemException.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <utility>

// User 类实现
User::User(std::string id, std::string name, std::string password)
    : id_(std::move(id)), name_(std::move(name)) {
    salt_ = generateSalt();
    password_ = generatePasswordHash(password, salt_);
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
    return password_ == generatePasswordHash(password, salt_);
}

void User::setPassword(std::string password) {
    salt_ = generateSalt();
    password_ = generatePasswordHash(password, salt_);
}

std::string User::generatePasswordHash(const std::string& password, const std::string& salt) {
    // 将密码和盐值拼接
    std::string combined = password + salt;
    
    // 计算 SHA-256 哈希
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined.c_str(), combined.length());
    SHA256_Final(hash, &sha256);
    
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