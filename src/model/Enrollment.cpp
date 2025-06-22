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

Enrollment::Enrollment(std::string studentId, std::string courseId, EnrollmentStatus status)
    : studentId_(std::move(studentId)),
      courseId_(std::move(courseId)),
      status_(status),
      enrollmentTime_(getCurrentTimeString()) {
}

Enrollment::Enrollment(Enrollment&& other) noexcept
    : studentId_(std::move(other.studentId_)),
      courseId_(std::move(other.courseId_)),
      status_(other.status_),
      enrollmentTime_(std::move(other.enrollmentTime_)) {
}

Enrollment& Enrollment::operator=(Enrollment&& other) noexcept {
    if (this != &other) {
        studentId_ = std::move(other.studentId_);
        courseId_ = std::move(other.courseId_);
        status_ = other.status_;
        enrollmentTime_ = std::move(other.enrollmentTime_);
    }
    return *this;
}

std::string Enrollment::getStatusString() const {
    switch (status_) {
        case EnrollmentStatus::ENROLLED:
            return "已选";
        case EnrollmentStatus::DROPPED:
            return "已退";
        case EnrollmentStatus::WAITLISTED:
            return "等待名单";
        default:
            return "未知";
    }
}

std::string Enrollment::getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}