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

/**
 * @brief 选课状态枚举
 */
enum class EnrollmentStatus {
    ENROLLED,        ///< 已选
    DROPPED,         ///< 已退
    WAITLISTED       ///< 等待名单
};

/**
 * @brief 选课记录类，管理学生选课关系和状态
 */
class Enrollment {
public:
    /**
     * @brief 默认构造函数
     */
    Enrollment() = default;
    
    /**
     * @brief 构造函数
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @param status 选课状态
     */
    Enrollment(std::string studentId, std::string courseId, 
               EnrollmentStatus status = EnrollmentStatus::ENROLLED);
    
    /**
     * @brief 移动构造函数
     * @param other 源对象
     */
    Enrollment(Enrollment&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     * @param other 源对象
     * @return 当前对象引用
     */
    Enrollment& operator=(Enrollment&& other) noexcept;
    
    /**
     * @brief 禁用拷贝构造函数
     */
    Enrollment(const Enrollment&) = delete;
    
    /**
     * @brief 禁用拷贝赋值运算符
     */
    Enrollment& operator=(const Enrollment&) = delete;
    
    /**
     * @brief 析构函数
     */
    ~Enrollment() = default;
    
    // Getters
    const std::string& getStudentId() const { return studentId_; }
    const std::string& getCourseId() const { return courseId_; }
    EnrollmentStatus getStatus() const { return status_; }
    const std::string& getEnrollmentTime() const { return enrollmentTime_; }
    
    /**
     * @brief 设置选课状态
     * @param status 新的选课状态
     */
    void setStatus(EnrollmentStatus status) { status_ = status; }
    
    /**
     * @brief 设置选课时间
     * @param time 选课时间
     */
    void setEnrollmentTime(const std::string& time) { enrollmentTime_ = time; }
    
    /**
     * @brief 获取选课状态的字符串表示
     * @return 状态字符串
     */
    std::string getStatusString() const;

private:
    std::string studentId_;           ///< 学生ID
    std::string courseId_;            ///< 课程ID
    EnrollmentStatus status_ = EnrollmentStatus::ENROLLED; ///< 选课状态
    std::string enrollmentTime_;      ///< 选课时间
    
    /**
     * @brief 获取当前时间的字符串表示
     * @return 时间字符串
     */
    static std::string getCurrentTimeString();
}; 