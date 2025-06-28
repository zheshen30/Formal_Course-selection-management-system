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

class LockGuard {
public:
    explicit LockGuard(std::mutex& mutex, unsigned long timeout = 0);

    ~LockGuard();

    LockGuard(const LockGuard&) = delete;

    LockGuard& operator=(const LockGuard&) = delete;

    bool isLocked() const { return locked_; }

private:
    std::mutex& mutex_;     // 互斥锁引用
    bool locked_ = false;   // 是否已获取锁
}; 