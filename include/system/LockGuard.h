#pragma once

#include <mutex>
#include <chrono>

/**
 * @brief 锁守卫类，基于RAII管理锁的获取和释放
 */
class LockGuard {
public:
    /**
     * @brief 构造函数，获取锁
     * @param mutex 互斥锁引用
     * @param timeout 获取锁的超时时间（毫秒），0表示无超时
     * @throw SystemException 超时时抛出LOCK_TIMEOUT异常
     */
    explicit LockGuard(std::mutex& mutex, unsigned long timeout = 0);

    /**
     * @brief 析构函数，释放锁
     */
    ~LockGuard();

    /**
     * @brief 禁用拷贝构造函数
     */
    LockGuard(const LockGuard&) = delete;

    /**
     * @brief 禁用赋值运算符
     */
    LockGuard& operator=(const LockGuard&) = delete;

    /**
     * @brief 检查是否已获取锁
     * @return 是否已获取锁
     */
    bool isLocked() const { return locked_; }

private:
    std::mutex& mutex_;  ///< 互斥锁引用
    bool locked_ = false; ///< 是否已获取锁
}; 