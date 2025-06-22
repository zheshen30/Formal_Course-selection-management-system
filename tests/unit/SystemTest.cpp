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

// CourseSystem类的测试fixture
class SystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        system = &CourseSystem::getInstance();
        system->initialize("./test_data", "./test_log");
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
    
    // 测试初始化
    EXPECT_TRUE(testSystem.initialize("./test_data2", "./test_log2"));
    
    // 测试关闭
    testSystem.shutdown();
}

// 测试用户登录和权限检查
TEST_F(SystemTest, LoginAndPermissionCheck) {
    // 测试管理员登录
    EXPECT_TRUE(system->login("admin001", "admin"));
    EXPECT_NE(nullptr, system->getCurrentUser());
    EXPECT_EQ("admin001", system->getCurrentUser()->getId());
    EXPECT_EQ(UserType::ADMIN, system->getCurrentUser()->getType());
    
    // 测试权限检查
    EXPECT_TRUE(system->checkPermission(UserType::ADMIN));
    EXPECT_TRUE(system->checkPermission(UserType::TEACHER));
    EXPECT_TRUE(system->checkPermission(UserType::STUDENT));
    
    // 测试注销
    system->logout();
    EXPECT_EQ(nullptr, system->getCurrentUser());
    
    // 测试教师登录
    EXPECT_TRUE(system->login("teacher001", "password"));
    EXPECT_NE(nullptr, system->getCurrentUser());
    EXPECT_EQ("teacher001", system->getCurrentUser()->getId());
    EXPECT_EQ(UserType::TEACHER, system->getCurrentUser()->getType());
    
    // 测试权限检查
    EXPECT_FALSE(system->checkPermission(UserType::ADMIN));
    EXPECT_TRUE(system->checkPermission(UserType::TEACHER));
    EXPECT_TRUE(system->checkPermission(UserType::STUDENT));
    
    // 测试注销
    system->logout();
    
    // 测试学生登录
    EXPECT_TRUE(system->login("student001", "password"));
    EXPECT_NE(nullptr, system->getCurrentUser());
    EXPECT_EQ("student001", system->getCurrentUser()->getId());
    EXPECT_EQ(UserType::STUDENT, system->getCurrentUser()->getType());
    
    // 测试权限检查
    EXPECT_FALSE(system->checkPermission(UserType::ADMIN));
    EXPECT_FALSE(system->checkPermission(UserType::TEACHER));
    EXPECT_TRUE(system->checkPermission(UserType::STUDENT));
    
    // 测试注销
    system->logout();
    
    // 测试错误登录
    EXPECT_FALSE(system->login("nonexistent", "password"));
    EXPECT_FALSE(system->login("admin001", "wrongpassword"));
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 