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
#include "../../include/system/LockGuard.h"
#include "../../include/system/SystemException.h"
#include <iostream>
#include <thread>

LockGuard::LockGuard(std::mutex& mutex, unsigned long timeout)
    : mutex_(mutex), locked_(false) {
    
    if (timeout == 0) {
        // 无超时，直接尝试获取锁
        try {
            mutex_.lock();
            locked_ = true;
        } catch (const std::exception& e) {
            std::cerr << "锁定失败: " << e.what() << std::endl;
            throw SystemException(ErrorType::LOCK_FAILURE, "获取锁失败: " + std::string(e.what()));
        }
    } else {
        // 有超时，实现一个简单的超时逻辑
        auto start = std::chrono::steady_clock::now();
        auto end = start + std::chrono::milliseconds(timeout);
        
        // 先尝试一次立即加锁
        locked_ = mutex_.try_lock();
        
        // 如果失败，则进行多次尝试，直到超时
        while (!locked_ && std::chrono::steady_clock::now() < end) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            locked_ = mutex_.try_lock();
        }
        
        if (!locked_) {
            std::cerr << "获取锁超时" << std::endl;
            throw SystemException(ErrorType::LOCK_TIMEOUT, "获取锁超时");
        }
    }
}

LockGuard::~LockGuard() {
    if (locked_) {
        mutex_.unlock();
        locked_ = false;
    }
}
