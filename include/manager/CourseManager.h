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

#include "../model/Course.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <functional>

class CourseManager {
public:
    static CourseManager& getInstance();

    bool addCourse(std::unique_ptr<Course> course);

    bool removeCourse(const std::string& courseId);

    Course* getCourse(const std::string& courseId);

    bool updateCourseInfo(const Course& course);

    std::vector<std::string> getAllCourseIds() const;

    // 获取某教师的所有课程ID
    std::vector<std::string> getTeacherCourseIds(const std::string& teacherId) const;

    std::vector<std::string> getStudentEnrolledCourseIds(const std::string& studentId) const;

    std::vector<std::string> findCourses(const std::function<bool(const Course&)>& predicate) const;

    bool hasCourse(const std::string& courseId) const;

    bool loadData();

    bool saveData(bool alreadyLocked = false);

private:
    CourseManager() = default;
    
    CourseManager(const CourseManager&) = delete;

    CourseManager& operator=(const CourseManager&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<Course>> courses_; // 课程映射表
    mutable std::mutex mutex_; // 互斥锁
}; 