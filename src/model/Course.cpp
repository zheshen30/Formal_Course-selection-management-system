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
#include "../../include/model/Course.h"
#include <utility>

Course::Course(std::string id, std::string name, CourseType type,
               double credit, int hours, std::string semester,
               std::string teacherId, int maxCapacity)
    : id_(std::move(id)),
      name_(std::move(name)),
      type_(type),
      credit_(credit),
      hours_(hours),
      semester_(std::move(semester)),
      teacherId_(std::move(teacherId)),
      maxCapacity_(maxCapacity) {
}

Course::Course(Course&& other) noexcept
    : id_(std::move(other.id_)),
      name_(std::move(other.name_)),
      type_(other.type_),
      credit_(other.credit_),
      hours_(other.hours_),
      semester_(std::move(other.semester_)),
      teacherId_(std::move(other.teacherId_)),
      maxCapacity_(other.maxCapacity_),
      enrolledStudents_(std::move(other.enrolledStudents_)) {
    
    other.credit_ = 0.0;
    other.hours_ = 0;
    other.maxCapacity_ = 0;
    other.type_ = CourseType::ELECTIVE;
}

Course& Course::operator=(Course&& other) noexcept {
    if (this != &other) {
        id_ = std::move(other.id_);
        name_ = std::move(other.name_);
        type_ = other.type_;
        credit_ = other.credit_;
        hours_ = other.hours_;
        semester_ = std::move(other.semester_);
        teacherId_ = std::move(other.teacherId_);
        maxCapacity_ = other.maxCapacity_;
        enrolledStudents_ = std::move(other.enrolledStudents_);
        
        other.credit_ = 0.0;
        other.hours_ = 0;
        other.maxCapacity_ = 0;
        other.type_ = CourseType::ELECTIVE;
    }
    return *this;
}

bool Course::addStudent(const std::string& studentId) {
    if (enrolledStudents_.size() >= static_cast<size_t>(maxCapacity_)) {
        return false; // 课程已满
    }
    
    auto result = enrolledStudents_.insert(studentId);
    return result.second; // 如果插入成功，返回true
}

bool Course::removeStudent(const std::string& studentId) {
    return enrolledStudents_.erase(studentId) > 0;
}

bool Course::hasStudent(const std::string& studentId) const {
    return enrolledStudents_.find(studentId) != enrolledStudents_.end();
}

std::string Course::getTypeString() const {
    switch (type_) {
        case CourseType::REQUIRED:
            return "必修";
        case CourseType::ELECTIVE:
            return "选修";
        case CourseType::RESTRICTED:
            return "限选";
        default:
            return "未知";
    }
} 