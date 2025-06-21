#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include "../../include/system/LockGuard.h"
#include "../../include/manager/UserManager.h"
#include "../../include/manager/CourseManager.h"
#include "../../include/manager/EnrollmentManager.h"
#include "../../include/util/DataManager.h"

// 并发测试fixture
class ConcurrencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        DataManager::getInstance().setDataDirectory("./test_concurrency_data");
        
        // 创建测试数据
        setupTestData();
    }

    void TearDown() override {
        // 清理测试环境
        cleanupTestData();
    }
    
    void setupTestData() {
        // 创建测试用户
        userManager.addUser(std::make_unique<Student>(
            "test_student", "测试学生", "password",
            "男", 20, "计算机科学", "计算机1班", "student@test.com"
        ));
        
        // 创建测试课程
        courseManager.addCourse(std::make_unique<Course>(
            "TEST101", "测试课程", CourseType::REQUIRED,
            3.0, 48, "2023秋季", "teacher001", 50
        ));
    }
    
    void cleanupTestData() {
        // 移除测试数据
        userManager.removeUser("test_student");
        courseManager.removeCourse("TEST101");
    }

    UserManager& userManager = UserManager::getInstance();
    CourseManager& courseManager = CourseManager::getInstance();
    EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
};

// 测试互斥锁
TEST_F(ConcurrencyTest, MutexTest) {
    std::mutex mtx;
    std::atomic<int> counter(0);
    
    // 创建多个线程同时增加计数器
    std::vector<std::thread> threads;
    const int numThreads = 10;
    const int numIncrements = 1000;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&mtx, &counter, numIncrements]() {
            for (int j = 0; j < numIncrements; ++j) {
                LockGuard lock(mtx);
                counter++;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // 验证计数器值
    EXPECT_EQ(numThreads * numIncrements, counter);
}

// 测试读写锁
TEST_F(ConcurrencyTest, SharedMutexTest) {
    std::shared_mutex rwMutex;
    std::atomic<int> readCount(0);
    std::atomic<int> writeCount(0);
    std::atomic<bool> writing(false);
    std::atomic<int> concurrentReaders(0);
    std::atomic<int> maxConcurrentReaders(0);
    
    // 创建多个读线程和写线程
    std::vector<std::thread> threads;
    const int numReadThreads = 8;
    const int numWriteThreads = 2;
    const int numOperations = 100;
    
    // 读线程
    for (int i = 0; i < numReadThreads; ++i) {
        threads.emplace_back([&rwMutex, &readCount, &writing, &concurrentReaders, &maxConcurrentReaders, numOperations]() {
            for (int j = 0; j < numOperations; ++j) {
                // 共享锁（读锁）
                std::shared_lock<std::shared_mutex> lock(rwMutex);
                readCount++;
                
                // 确认没有写操作正在进行
                EXPECT_FALSE(writing);
                
                // 记录并发读取器数量
                int readers = ++concurrentReaders;
                maxConcurrentReaders = std::max(maxConcurrentReaders.load(), readers);
                
                // 模拟读取操作
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                
                --concurrentReaders;
            }
        });
    }
    
    // 写线程
    for (int i = 0; i < numWriteThreads; ++i) {
        threads.emplace_back([&rwMutex, &writeCount, &writing, &concurrentReaders, numOperations]() {
            for (int j = 0; j < numOperations; ++j) {
                // 排他锁（写锁）
                std::unique_lock<std::shared_mutex> lock(rwMutex);
                writeCount++;
                
                // 确认没有其他读或写操作正在进行
                EXPECT_EQ(0, concurrentReaders);
                EXPECT_FALSE(writing.exchange(true));
                
                // 模拟写入操作
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                
                writing = false;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // 验证操作计数
    EXPECT_EQ(numReadThreads * numOperations, readCount);
    EXPECT_EQ(numWriteThreads * numOperations, writeCount);
    
    // 验证有多个并发读取器
    EXPECT_GT(maxConcurrentReaders, 1);
}

// 测试条件变量
TEST_F(ConcurrencyTest, ConditionVariableTest) {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    std::atomic<int> count(0);
    
    // 创建多个等待线程
    std::vector<std::thread> threads;
    const int numThreads = 5;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&mtx, &cv, &ready, &count]() {
            std::unique_lock<std::mutex> lock(mtx);
            // 等待条件变量通知
            cv.wait(lock, [&ready] { return ready; });
            count++;
        });
    }
    
    // 确保所有线程都已启动并等待
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 通知所有等待线程
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_all();
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // 验证所有线程都被唤醒
    EXPECT_EQ(numThreads, count);
}

// 测试多线程选课
TEST_F(ConcurrencyTest, ConcurrentEnrollmentTest) {
    const int numStudents = 20;
    const int maxCapacity = 10;
    
    // 创建一个有限容量的课程
    courseManager.removeCourse("TEST101");
    courseManager.addCourse(std::make_unique<Course>(
        "TEST101", "测试课程", CourseType::REQUIRED,
        3.0, 48, "2023秋季", "teacher001", maxCapacity
    ));
    
    // 创建多个学生
    for (int i = 0; i < numStudents; ++i) {
        std::string studentId = "student" + std::to_string(i);
        userManager.addUser(std::make_unique<Student>(
            studentId, "学生" + std::to_string(i), "password",
            "男", 20, "计算机科学", "计算机1班", studentId + "@test.com"
        ));
    }
    
    // 多线程同时选课
    std::vector<std::thread> threads;
    std::atomic<int> successCount(0);
    
    for (int i = 0; i < numStudents; ++i) {
        std::string studentId = "student" + std::to_string(i);
        threads.emplace_back([this, studentId, &successCount]() {
            if (enrollmentManager.enrollCourse(studentId, "TEST101")) {
                successCount++;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // 验证选课结果
    EXPECT_EQ(maxCapacity, successCount);
    
    // 获取课程并验证选课人数
    Course* course = courseManager.getCourse("TEST101");
    ASSERT_NE(nullptr, course);
    EXPECT_EQ(maxCapacity, course->getCurrentEnrollment());
    EXPECT_TRUE(course->isFull());
    
    // 清理测试数据
    for (int i = 0; i < numStudents; ++i) {
        std::string studentId = "student" + std::to_string(i);
        enrollmentManager.dropCourse(studentId, "TEST101");
        userManager.removeUser(studentId);
    }
}

// 测试多线程数据访问
TEST_F(ConcurrencyTest, ConcurrentDataAccessTest) {
    std::vector<std::thread> threads;
    const int numThreads = 10;
    const int numOperations = 100;
    
    // 多线程同时读取和修改用户数据
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, numOperations]() {
            for (int j = 0; j < numOperations; ++j) {
                if (i % 2 == 0) {
                    // 偶数线程：读取操作
                    User* user = userManager.getUser("test_student");
                    if (user) {
                        EXPECT_EQ("test_student", user->getId());
                    }
                } else {
                    // 奇数线程：修改操作
                    std::string newName = "学生" + std::to_string(j);
                    Student* student = static_cast<Student*>(userManager.getUser("test_student"));
                    if (student) {
                        student->setName(newName);
                    }
                }
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // 验证用户依然存在
    User* user = userManager.getUser("test_student");
    ASSERT_NE(nullptr, user);
}

// 测试死锁检测和超时机制
TEST_F(ConcurrencyTest, DeadlockDetectionTest) {
    std::mutex mtx1, mtx2;
    std::atomic<bool> thread1Started(false);
    std::atomic<bool> thread2Started(false);
    std::atomic<bool> thread1Locked1(false);
    std::atomic<bool> thread2Locked2(false);
    
    // 创建两个线程，尝试以相反的顺序获取两个锁
    std::thread t1([&mtx1, &mtx2, &thread1Started, &thread1Locked1]() {
        thread1Started = true;
        
        // 获取第一个锁
        mtx1.lock();
        thread1Locked1 = true;
        
        // 等待一段时间，确保另一个线程有机会获取第二个锁
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 尝试获取第二个锁，但设置超时
        bool locked = false;
        try {
            // 在实际系统中，这里应该使用系统的超时机制
            // 这里我们模拟一个超时检查
            if (mtx2.try_lock()) {
                locked = true;
                mtx2.unlock();
            }
        } catch (...) {
            // 捕获任何异常
        }
        
        // 释放第一个锁
        mtx1.unlock();
        
        // 我们期望无法获取第二个锁（因为线程2持有它）
        EXPECT_FALSE(locked);
    });
    
    std::thread t2([&mtx1, &mtx2, &thread2Started, &thread2Locked2]() {
        thread2Started = true;
        
        // 获取第二个锁
        mtx2.lock();
        thread2Locked2 = true;
        
        // 等待一段时间，确保另一个线程有机会获取第一个锁
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 尝试获取第一个锁，但设置超时
        bool locked = false;
        try {
            // 在实际系统中，这里应该使用系统的超时机制
            // 这里我们模拟一个超时检查
            if (mtx1.try_lock()) {
                locked = true;
                mtx1.unlock();
            }
        } catch (...) {
            // 捕获任何异常
        }
        
        // 释放第二个锁
        mtx2.unlock();
        
        // 我们期望无法获取第一个锁（因为线程1持有它）
        EXPECT_FALSE(locked);
    });
    
    // 等待线程完成
    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
    
    // 验证两个线程都已启动并获取了各自的第一个锁
    EXPECT_TRUE(thread1Started);
    EXPECT_TRUE(thread2Started);
    EXPECT_TRUE(thread1Locked1);
    EXPECT_TRUE(thread2Locked2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 