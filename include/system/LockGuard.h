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