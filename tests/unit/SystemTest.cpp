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
#include <gtest/gtest.h>
#include "../../include/system/CourseSystem.h"
#include "../../include/system/LockGuard.h"
#include "../../include/system/SystemException.h"
#include "../../include/manager/UserManager.h"
#include "../../include/manager/CourseManager.h"
#include "../../include/manager/EnrollmentManager.h"
#include "../../include/model/User.h"
#include <filesystem>
#include <memory>

// CourseSystem类的测试fixture
class SystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录
        try {
            std::filesystem::create_directories("../test_data");
            std::filesystem::create_directories("../test_log");
        } catch (const std::exception& e) {
            std::cerr << "创建测试目录异常: " << e.what() << std::endl;
        }
        
        // 设置测试环境
        system = &CourseSystem::getInstance();
        system->initialize("../test_data", "../test_log");
        
        // 添加测试用户数据
        UserManager& userManager = UserManager::getInstance();
        
        // 添加管理员用户
        if (!userManager.hasUser("admin001")) {
            // 创建管理员用户
            std::unique_ptr<Admin> admin = std::make_unique<Admin>("admin001", "系统管理员", "admin");
            userManager.addAdmin(std::move(admin));
        }
        
        // 添加教师用户
        if (!userManager.hasUser("teacher001")) {
            // 创建教师用户
            std::unique_ptr<Teacher> teacher = std::make_unique<Teacher>(
                "teacher001", "测试教师", "password",
                "计算机系", "教授", "teacher@example.com"
            );
            userManager.addTeacher(std::move(teacher));
        }
        
        // 添加学生用户
        if (!userManager.hasUser("student001")) {
            // 创建学生用户
            std::unique_ptr<Student> student = std::make_unique<Student>(
                "student001", "测试学生", "password",
                "男", 20, "计算机系", "计算机1班", "student@example.com"
            );
            userManager.addStudent(std::move(student));
        }
        
        // 保存用户数据
        userManager.saveData();
    }

    void TearDown() override {
        // 清理测试环境
        system->shutdown();
    }

    CourseSystem* system;
};

// 测试CourseSystem初始化和关闭
TEST_F(SystemTest, InitializeAndShutdown) {
    CourseSystem& testSystem = CourseSystem::getInstance();
    
    // 测试初始化 - 使用规定的测试数据目录
    EXPECT_TRUE(testSystem.initialize("../test_data", "../test_log"));
    
    // 测试关闭
    testSystem.shutdown();
}

// 测试用户登录和权限检查
TEST_F(SystemTest, LoginAndPermissionCheck) {
    // 测试管理员登录
    EXPECT_TRUE(system->login("admin001", "admin"));
    ASSERT_NE(nullptr, system->getCurrentUser()) << "登录后用户不应为空";
    if (system->getCurrentUser() != nullptr) {
        EXPECT_EQ("admin001", system->getCurrentUser()->getId());
        EXPECT_EQ(UserType::ADMIN, system->getCurrentUser()->getType());
        
        // 测试权限检查 - 管理员有所有权限
        EXPECT_TRUE(system->checkPermission(UserType::ADMIN));
        EXPECT_TRUE(system->checkPermission(UserType::TEACHER));
        EXPECT_TRUE(system->checkPermission(UserType::STUDENT));
    }
    
    // 测试注销
    system->logout();
    EXPECT_EQ(nullptr, system->getCurrentUser());
    
    // 测试教师登录
    EXPECT_TRUE(system->login("teacher001", "password"));
    ASSERT_NE(nullptr, system->getCurrentUser()) << "登录后用户不应为空";
    if (system->getCurrentUser() != nullptr) {
        EXPECT_EQ("teacher001", system->getCurrentUser()->getId());
        EXPECT_EQ(UserType::TEACHER, system->getCurrentUser()->getType());
        
        // 测试权限检查 - 教师只有教师权限
        EXPECT_FALSE(system->checkPermission(UserType::ADMIN));
        EXPECT_TRUE(system->checkPermission(UserType::TEACHER));
        EXPECT_FALSE(system->checkPermission(UserType::STUDENT));
    }
    
    // 测试注销
    system->logout();
    
    // 测试学生登录
    EXPECT_TRUE(system->login("student001", "password"));
    ASSERT_NE(nullptr, system->getCurrentUser()) << "登录后用户不应为空";
    if (system->getCurrentUser() != nullptr) {
        EXPECT_EQ("student001", system->getCurrentUser()->getId());
        EXPECT_EQ(UserType::STUDENT, system->getCurrentUser()->getType());
        
        // 测试权限检查 - 学生只有学生权限
        EXPECT_FALSE(system->checkPermission(UserType::ADMIN));
        EXPECT_FALSE(system->checkPermission(UserType::TEACHER));
        EXPECT_TRUE(system->checkPermission(UserType::STUDENT));
    }
    
    // 测试注销
    system->logout();
    
    // 测试错误密码
    EXPECT_FALSE(system->login("admin001", "wrongpassword"));
    EXPECT_EQ(nullptr, system->getCurrentUser());
    
    // 测试不存在的用户
    EXPECT_FALSE(system->login("nonexistent", "anypassword"));
    EXPECT_EQ(nullptr, system->getCurrentUser());
}

// 测试语言切换
TEST_F(SystemTest, LanguageSelection) {
    // 测试默认语言
    EXPECT_EQ(Language::CHINESE, system->getCurrentLanguage());
    
    // 测试切换到英文
    EXPECT_TRUE(system->selectLanguage(Language::ENGLISH));
    EXPECT_EQ(Language::ENGLISH, system->getCurrentLanguage());
    
    // 测试获取翻译文本
    std::string loginText = system->getText("login");
    EXPECT_FALSE(loginText.empty());
    
    // 切换回中文
    EXPECT_TRUE(system->selectLanguage(Language::CHINESE));
    EXPECT_EQ(Language::CHINESE, system->getCurrentLanguage());
}

// 测试日志记录
TEST_F(SystemTest, LoggingFunctions) {
    // 测试各级别日志记录
    system->log(LogLevel::INFO, "Info log message");
    system->log(LogLevel::WARNING, "Warning log message");
    system->log(LogLevel::ERROR, "Error log message");
    
    // 注意：这里只是测试API调用，实际日志内容需要检查日志文件
}

// 测试LockGuard类
TEST_F(SystemTest, LockGuardTest) {
    std::mutex testMutex;
    
    {
        // 测试LockGuard构造函数（加锁）
        LockGuard lock(testMutex);
        
        // 尝试在另一个线程中获取同一个锁（应该会阻塞）
        bool lockAcquired = false;
        std::thread t([&testMutex, &lockAcquired]() {
            std::unique_lock<std::mutex> lock(testMutex, std::try_to_lock);
            lockAcquired = lock.owns_lock();
        });
        t.join();
        
        // 验证另一个线程无法获取锁
        EXPECT_FALSE(lockAcquired);
    }
    
    // LockGuard析构后，锁应该被释放
    bool lockAcquired = false;
    std::thread t([&testMutex, &lockAcquired]() {
        std::unique_lock<std::mutex> lock(testMutex, std::try_to_lock);
        lockAcquired = lock.owns_lock();
    });
    t.join();
    
    // 验证另一个线程现在可以获取锁
    EXPECT_TRUE(lockAcquired);
}

// 测试SystemException类
TEST_F(SystemTest, SystemExceptionTest) {
    // 创建异常对象
    SystemException ex(ErrorType::DATA_NOT_FOUND, "测试异常", 404);
    
    // 验证异常信息
    EXPECT_EQ(ErrorType::DATA_NOT_FOUND, ex.getType());
    EXPECT_STREQ("测试异常", ex.what());
    EXPECT_EQ(404, ex.getErrorCode());
    
    // 测试异常抛出和捕获
    try {
        throw SystemException(ErrorType::PERMISSION_DENIED, "权限不足", 403);
    } catch (const SystemException& e) {
        EXPECT_EQ(ErrorType::PERMISSION_DENIED, e.getType());
        EXPECT_STREQ("权限不足", e.what());
        EXPECT_EQ(403, e.getErrorCode());
    }
}

// 测试密码修改功能
TEST_F(SystemTest, PasswordChangeTest) {
    // 创建测试用户
    UserManager& userManager = UserManager::getInstance();
    
    // 先移除可能存在的测试用户
    if (userManager.hasUser("password_test")) {
        userManager.removeUser("password_test");
    }
    
    // 创建测试用户
    std::unique_ptr<Student> student = std::make_unique<Student>(
        "password_test", "密码测试用户", "initial_password",
        "男", 20, "计算机科学", "计算机1班", "password_test@example.com"
    );
    
    EXPECT_TRUE(userManager.addStudent(std::move(student)));
    
    // 获取用户指针
    User* user = userManager.getUser("password_test");
    ASSERT_NE(nullptr, user);
    
    // 验证初始密码
    EXPECT_TRUE(user->verifyPassword("initial_password"));
    
    // 模拟用户登录
    EXPECT_TRUE(system->login("password_test", "initial_password"));
    ASSERT_NE(nullptr, system->getCurrentUser()) << "登录后用户不应为空";
    EXPECT_EQ("password_test", system->getCurrentUser()->getId());
    
    // 场景1：新密码与确认密码不一致
    EXPECT_FALSE(system->changePassword(
        "password_test", "initial_password", "new_password", "different_password"));
    
    // 验证密码未被修改
    EXPECT_TRUE(user->verifyPassword("initial_password"));
    
    // 场景2：新密码不符合要求（长度不足）
    EXPECT_FALSE(system->changePassword(
        "password_test", "initial_password", "short", "short"));
    
    // 验证密码未被修改
    EXPECT_TRUE(user->verifyPassword("initial_password"));
    
    // 场景3：旧密码错误
    EXPECT_FALSE(system->changePassword(
        "password_test", "wrong_password", "new_password123", "new_password123"));
    
    // 验证密码未被修改
    EXPECT_TRUE(user->verifyPassword("initial_password"));
    
    // 场景4：成功修改密码
    EXPECT_TRUE(system->changePassword(
        "password_test", "initial_password", "new_password123", "new_password123"));
    
    // 验证密码已被修改
    EXPECT_TRUE(user->verifyPassword("new_password123"));
    EXPECT_FALSE(user->verifyPassword("initial_password"));
    
    // 注销登录
    system->logout();
    EXPECT_EQ(nullptr, system->getCurrentUser());
    
    // 场景5：用户未登录
    EXPECT_FALSE(system->changePassword(
        "password_test", "new_password123", "another_password", "another_password"));
    
    // 场景6：尝试修改其他用户密码
    // 先以admin001登录
    EXPECT_TRUE(system->login("admin001", "admin"));
    ASSERT_NE(nullptr, system->getCurrentUser()) << "登录后用户不应为空";
    EXPECT_EQ("admin001", system->getCurrentUser()->getId());
    
    // 尝试修改password_test用户的密码，应该失败
    EXPECT_FALSE(system->changePassword(
        "password_test", "new_password123", "hacked_password", "hacked_password"));
    
    // 清理环境
    system->logout();
    
    // 清理测试用户
    userManager.removeUser("password_test");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 