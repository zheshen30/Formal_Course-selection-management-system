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
#include <vector>
#include <memory>
#include <utility>
#include <unordered_set>

/**
 * @brief 课程性质枚举
 */
enum class CourseType {
    REQUIRED,      ///< 必修课
    ELECTIVE,      ///< 选修课
    RESTRICTED     ///< 限选课
};

/**
 * @brief 课程类，提供课程信息和容量管理功能
 */
class Course {
public:
    /**
     * @brief 默认构造函数
     */
    Course() = default;

    /**
     * @brief 构造函数
     * @param id 课程ID
     * @param name 课程名称
     * @param type 课程性质
     * @param credit 学分
     * @param hours 总学时
     * @param semester 开课学期
     * @param teacherId 授课教师ID
     * @param maxCapacity 最大容量
     */
    Course(std::string id, std::string name, CourseType type,
           double credit, int hours, std::string semester,
           std::string teacherId, int maxCapacity);

    /**
     * @brief 移动构造函数
     * @param other 源对象
     */
    Course(Course&& other) noexcept;

    /**
     * @brief 移动赋值运算符
     * @param other 源对象
     * @return 当前对象引用
     */
    Course& operator=(Course&& other) noexcept;

    /**
     * @brief 禁用拷贝构造函数
     */
    Course(const Course&) = delete;

    /**
     * @brief 禁用拷贝赋值运算符
     */
    Course& operator=(const Course&) = delete;

    /**
     * @brief 析构函数
     */
    ~Course() = default;

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    CourseType getType() const { return type_; }
    double getCredit() const { return credit_; }
    int getHours() const { return hours_; }
    const std::string& getSemester() const { return semester_; }
    const std::string& getTeacherId() const { return teacherId_; }
    int getMaxCapacity() const { return maxCapacity_; }
    int getCurrentEnrollment() const { return static_cast<int>(enrolledStudents_.size()); }
    bool isFull() const { return enrolledStudents_.size() >= static_cast<size_t>(maxCapacity_); }

    // Setters
    void setName(std::string name) { name_ = std::move(name); }
    void setType(CourseType type) { type_ = type; }
    void setCredit(double credit) { credit_ = credit; }
    void setHours(int hours) { hours_ = hours; }
    void setSemester(std::string semester) { semester_ = std::move(semester); }
    void setTeacherId(std::string teacherId) { teacherId_ = std::move(teacherId); }
    void setMaxCapacity(int maxCapacity) { maxCapacity_ = maxCapacity; }

    /**
     * @brief 添加学生到课程
     * @param studentId 学生ID
     * @return 是否添加成功
     */
    bool addStudent(const std::string& studentId);

    /**
     * @brief 从课程中移除学生
     * @param studentId 学生ID
     * @return 是否移除成功
     */
    bool removeStudent(const std::string& studentId);

    /**
     * @brief 检查学生是否已选此课程
     * @param studentId 学生ID
     * @return 是否已选课
     */
    bool hasStudent(const std::string& studentId) const;

    /**
     * @brief 获取已选学生ID列表
     * @return 学生ID集合
     */
    const std::unordered_set<std::string>& getEnrolledStudents() const { return enrolledStudents_; }

    /**
     * @brief 获取课程空位数
     * @return 空位数
     */
    int getAvailableSeats() const { return maxCapacity_ - static_cast<int>(enrolledStudents_.size()); }

    /**
     * @brief 获取课程类型的字符串表示
     * @return 课程类型字符串
     */
    std::string getTypeString() const;

private:
    std::string id_;                           ///< 课程ID
    std::string name_;                         ///< 课程名称
    CourseType type_ = CourseType::ELECTIVE;   ///< 课程性质
    double credit_ = 0.0;                      ///< 学分
    int hours_ = 0;                            ///< 总学时
    std::string semester_;                     ///< 开课学期
    std::string teacherId_;                    ///< 授课教师ID
    int maxCapacity_ = 0;                      ///< 最大容量
    std::unordered_set<std::string> enrolledStudents_; ///< 已选学生ID集合
}; 