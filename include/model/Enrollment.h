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

#include <string>
#include <chrono>
#include <utility>

class Enrollment {
public:
    Enrollment() = default;
    
    Enrollment(std::string studentId, std::string courseId);
    
    Enrollment(Enrollment&& other) noexcept;
    
    Enrollment& operator=(Enrollment&& other) noexcept;
    
    Enrollment(const Enrollment&) = delete;
    
    Enrollment& operator=(const Enrollment&) = delete;
    
    ~Enrollment() = default;
    
    // Getters
    const std::string& getStudentId() const { return studentId_; }
    const std::string& getCourseId() const { return courseId_; }
    const std::string& getEnrollmentTime() const { return enrollmentTime_; }
    
    void setEnrollmentTime(const std::string& time) { enrollmentTime_ = time; }

private:
    std::string studentId_;           // 学生ID
    std::string courseId_;            // 课程ID
    std::string enrollmentTime_;      // 选课时间
    
    static std::string getCurrentTimeString();
}; 