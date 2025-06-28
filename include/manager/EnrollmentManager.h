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

#include "../model/Enrollment.h"
#include "../model/Course.h"
#include "../model/User.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <functional>

class EnrollmentManager {
public:
   
    static EnrollmentManager& getInstance();

    bool enrollCourse(const std::string& studentId, const std::string& courseId);

    bool dropCourse(const std::string& studentId, const std::string& courseId);

    Enrollment* getEnrollment(const std::string& studentId, const std::string& courseId);

    std::vector<Enrollment*> getStudentEnrollments(const std::string& studentId) const; 

    std::vector<Enrollment*> getCourseEnrollments(const std::string& courseId) const; 

    bool isEnrolled(const std::string& studentId, const std::string& courseId) const;

    std::vector<Enrollment*> findEnrollments(const std::function<bool(const Enrollment&)>& predicate) const;

    bool loadData();

    bool saveData(bool alreadyLocked = false);

    bool removeEnrollment(const std::string& studentId, const std::string& courseId);

private:

    EnrollmentManager() = default;
    
    EnrollmentManager(const EnrollmentManager&) = delete;
    
    EnrollmentManager& operator=(const EnrollmentManager&) = delete;
    
    bool addEnrollment(std::unique_ptr<Enrollment> enrollment);
    
    static std::string generateKey(const std::string& studentId, const std::string& courseId);
    
    std::unordered_map<std::string, std::unique_ptr<Enrollment>> enrollments_; // 选课记录映射表
    mutable std::mutex mutex_; // 互斥锁
};