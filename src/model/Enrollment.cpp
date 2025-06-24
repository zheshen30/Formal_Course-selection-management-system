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
#include "../../include/model/Enrollment.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

Enrollment::Enrollment(std::string studentId, std::string courseId)
    : studentId_(std::move(studentId)),
      courseId_(std::move(courseId)),
      enrollmentTime_(getCurrentTimeString()) {
}

Enrollment::Enrollment(Enrollment&& other) noexcept
    : studentId_(std::move(other.studentId_)),
      courseId_(std::move(other.courseId_)),
      enrollmentTime_(std::move(other.enrollmentTime_)) {
}

Enrollment& Enrollment::operator=(Enrollment&& other) noexcept {
    if (this != &other) {
        studentId_ = std::move(other.studentId_);
        courseId_ = std::move(other.courseId_);
        enrollmentTime_ = std::move(other.enrollmentTime_);
    }
    return *this;
}

std::string Enrollment::getCurrentTimeString() {
    // 定义北京时间相对UTC的偏移量（秒）
    const int BEIJING_OFFSET = 8 * 3600; // 8小时 = 8 * 3600秒
    
    // 获取当前UTC时间戳
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    
    // 加上北京时间偏移
    time_t beijing_time_t = now_time_t + BEIJING_OFFSET;
    
    // 转换为tm结构
    std::tm beijing_tm = *std::gmtime(&beijing_time_t);
    
    // 格式化时间字符串
    std::stringstream ss;
    ss << std::put_time(&beijing_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}