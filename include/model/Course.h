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


enum class CourseType {
    REQUIRED,      // 必修课
    ELECTIVE       // 选修课
};

class Course {
public:
    Course() = default;

    Course(std::string id, std::string name, CourseType type,
           double credit, int hours, std::string semester,
           std::string teacherId, int maxCapacity);

    Course(Course&& other) noexcept;

    Course& operator=(Course&& other) noexcept;

    Course(const Course&) = delete;

    Course& operator=(const Course&) = delete;

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

    bool addStudent(const std::string& studentId);

    bool removeStudent(const std::string& studentId);

    bool hasStudent(const std::string& studentId) const;

    const std::unordered_set<std::string>& getEnrolledStudents() const { return enrolledStudents_; }

    int getAvailableSeats() const { return maxCapacity_ - static_cast<int>(enrolledStudents_.size()); }

    std::string getTypeString() const;

private:
    std::string id_;                           // 课程ID
    std::string name_;                         // 课程名称
    CourseType type_ = CourseType::ELECTIVE;   // 课程性质
    double credit_ = 0.0;                      // 学分
    int hours_ = 0;                            // 总学时
    std::string semester_;                     // 开课学期
    std::string teacherId_;                    // 授课教师ID
    int maxCapacity_ = 0;                      // 最大容量
    std::unordered_set<std::string> enrolledStudents_; // 已选学生ID集合
}; 