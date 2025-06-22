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

LockGuard::LockGuard(std::mutex& mutex, unsigned long timeout)
    : mutex_(mutex), locked_(false) {
    
    if (timeout == 0) {
        // 无超时，直接尝试获取锁
        mutex_.lock();
        locked_ = true;
    } else {
        // 有超时，使用try_lock_for尝试获取锁
        auto timeoutDuration = std::chrono::milliseconds(timeout);
        
        // 首先尝试无限期地获取锁
        if (mutex_.try_lock()) {
            locked_ = true;
            return;
        }
        
        // 如果无法立即获取，等待指定的时间
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < timeoutDuration) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (mutex_.try_lock()) {
                locked_ = true;
                return;
            }
        }
        
        // 超时，无法获取锁
        if (!locked_) {
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
