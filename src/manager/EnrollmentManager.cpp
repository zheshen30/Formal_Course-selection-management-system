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
#include "../../include/manager/EnrollmentManager.h"
#include "../../include/manager/UserManager.h"
#include "../../include/manager/CourseManager.h"
#include "../../include/util/DataManager.h"
#include "../../include/system/LockGuard.h"
#include "../../include/system/SystemException.h"
#include "../../include/util/Logger.h"

#include "../../nlohmann/json.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>

using json = nlohmann::json;

EnrollmentManager& EnrollmentManager::getInstance() {
    static EnrollmentManager instance;  // Meyer's单例模式
    return instance;
}
 
 //选课
bool EnrollmentManager::enrollCourse(const std::string& studentId, const std::string& courseId) {
    // 检查参数
    if (studentId.empty() || courseId.empty()) {
        Logger::getInstance().error("选课失败：学生ID或课程ID为空");
        return false;
    }
    
    try 
    {
        // 验证学生存在
        UserManager& userManager = UserManager::getInstance();
        Student* student = userManager.getStudent(studentId);
        if (!student) {
            Logger::getInstance().warning("选课失败：学生ID " + studentId + " 不存在");
            return false;
        }
        
        // 验证课程存在
        CourseManager& courseManager = CourseManager::getInstance();
        Course* course = courseManager.getCourse(courseId);
        if (!course) {
            Logger::getInstance().warning("选课失败：课程ID " + courseId + " 不存在");
            return false;
        }
        
        // 检查是否已选此课程
        if (isEnrolled(studentId, courseId)) {
            Logger::getInstance().warning("选课失败：学生 " + studentId + " 已选课程 " + courseId);
            throw SystemException(ErrorType::ALREADY_ENROLLED, "学生已选择此课程");
        }
        
        // 检查课程是否已满
        if (course->isFull()) {
            Logger::getInstance().warning("选课失败：课程 " + courseId + " 已满");
            throw SystemException(ErrorType::COURSE_FULL, "课程已满");
        }
        
        // 创建选课记录并添加到json文件
        auto enrollment = std::make_unique<Enrollment>(studentId, courseId);
        bool enrollmentAdded = addEnrollment(std::move(enrollment));
        
        if (!enrollmentAdded) {
            Logger::getInstance().error("选课失败：无法添加选课记录");
            return false;
        }
        
        // 更新课程的学生列表
        bool studentAdded = course->addStudent(studentId);
        if (!studentAdded) {
            Logger::getInstance().error("选课失败：无法将学生添加到课程");
            return false;
        }
        
        // 保存选课数据和课程数据，确保数据同步
        saveData(true); // 传入true表示已获取锁
        courseManager.saveData(true); // 传入true表示已获取锁
        
        // 记录选课信息到日志
        Logger::getInstance().info("选课成功：学生 " + studentId + " 选择课程 " + courseId);
        return true;

    } catch (const SystemException& e) {
        // 已处理的系统异常，重新抛出
        throw;
    }
    catch (const std::exception& e) {
        Logger::getInstance().error("选课失败：发生异常 - " + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "选课操作失败：" + std::string(e.what()));
    }
}

bool EnrollmentManager::dropCourse(const std::string& studentId, const std::string& courseId) {
    // 检查参数
    if (studentId.empty() || courseId.empty()) {
        Logger::getInstance().error("退课失败：学生ID或课程ID为空");
        return false;
    }
    
    try {
        // 验证选课记录存在
        Enrollment* enrollment = getEnrollment(studentId, courseId);
        if (!enrollment) {
            Logger::getInstance().warning("退课失败：未找到学生 " + studentId + " 的课程 " + courseId + " 的选课记录");
            throw SystemException(ErrorType::NOT_ENROLLED, "未找到该选课记录");
        }
        
        // 验证课程存在
        CourseManager& courseManager = CourseManager::getInstance();
        Course* course = courseManager.getCourse(courseId);
        if (!course) {
            Logger::getInstance().warning("退课失败：课程ID " + courseId + " 不存在");
            return false;
        }
        
        // 从课程的学生列表中移除学生
        bool removed = course->removeStudent(studentId);
        if (!removed) {
            Logger::getInstance().warning("退课警告：无法从课程 " + courseId + " 中移除学生 " + studentId);
            return false;
        }
        
        // 移除选课记录
        bool recordRemoved = removeEnrollment(studentId, courseId);
        if (!recordRemoved) {
            Logger::getInstance().error("退课失败：无法删除选课记录");
            return false;
        }
        
        // 保存选课数据和课程数据，确保数据同步
        saveData(true); // 传入true表示已获取锁
        courseManager.saveData(true); // 传入true表示已获取锁
        
        // 记录退课信息到日志
        Logger::getInstance().info("退课成功：学生 " + studentId + " 退出课程 " + courseId);
        return true;
    } catch (const SystemException& e) {
        // 已处理的系统异常，重新抛出
        throw;
    } catch (const std::exception& e) {
        Logger::getInstance().error("退课失败：发生异常 - " + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "退课操作失败：" + std::string(e.what()));
    }
}

Enrollment* EnrollmentManager::getEnrollment(const std::string& studentId, const std::string& courseId) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
    }
    
    std::string key = generateKey(studentId, courseId);
    auto it = enrollments_.find(key);
    
    if (it == enrollments_.end()) {
        return nullptr;
    }
    
    return it->second.get();
}

std::vector<Enrollment*> EnrollmentManager::getStudentEnrollments(const std::string& studentId) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
    }
    
    std::vector<Enrollment*> result;
    
    for (const auto& pair : enrollments_) {
        Enrollment* enrollment = pair.second.get();
        if (enrollment->getStudentId() == studentId) {
            result.push_back(enrollment);
        }
    }
    
    return result;
}

std::vector<Enrollment*> EnrollmentManager::getCourseEnrollments(const std::string& courseId) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
    }
    
    std::vector<Enrollment*> result;
    
    for (const auto& pair : enrollments_) {
        Enrollment* enrollment = pair.second.get();
        if (enrollment->getCourseId() == courseId) {
            result.push_back(enrollment);
        }
    }
    
    return result;
}

bool EnrollmentManager::isEnrolled(const std::string& studentId, const std::string& courseId) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
    }
    
    std::string key = generateKey(studentId, courseId);
    auto it = enrollments_.find(key);
    
    return it != enrollments_.end();
}

std::vector<Enrollment*> EnrollmentManager::findEnrollments(
    const std::function<bool(const Enrollment&)>& predicate) const {
    
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
    }
    
    std::vector<Enrollment*> result;
    
    for (const auto& pair : enrollments_) {
        if (predicate(*(pair.second))) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

bool EnrollmentManager::addEnrollment(std::unique_ptr<Enrollment> enrollment) {
    if (!enrollment) {
        return false;
    }
    
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
    }
    
    std::string key = generateKey(enrollment->getStudentId(), enrollment->getCourseId());
    if (enrollments_.find(key) != enrollments_.end()) {
        Logger::getInstance().warning("添加选课记录失败：选课记录已存在");
        return false;
    }
    
    enrollments_[key] = std::move(enrollment);
    return true;
}

std::string EnrollmentManager::generateKey(const std::string& studentId, const std::string& courseId) {
    return studentId + ":" + courseId;
}

bool EnrollmentManager::loadData() {
    try {
        LockGuard lock(mutex_, 5000); // 设置5秒超时
        if (!lock.isLocked()) {
            throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
        }
        
        DataManager& dataManager = DataManager::getInstance();
        std::string jsonStr = dataManager.loadJsonFromFile("enrollment.json");
        
        if (jsonStr.empty()) {
            Logger::getInstance().warning("选课数据文件为空或不存在");
            return false;
        }
        
        json enrollmentsJson = json::parse(jsonStr);
        enrollments_.clear();
        
        for (const auto& enrollmentJson : enrollmentsJson) {
            std::string studentId = enrollmentJson["studentId"];
            std::string courseId = enrollmentJson["courseId"];
            std::string enrollmentTime = enrollmentJson["enrollmentTime"];
            
            auto enrollment = std::make_unique<Enrollment>(studentId, courseId);
            enrollment->setEnrollmentTime(enrollmentTime); // 设置时间，避免使用当前时间
            
            std::string key = generateKey(studentId, courseId);
            enrollments_[key] = std::move(enrollment);
        }
        
        Logger::getInstance().info("成功加载选课数据，共 " + std::to_string(enrollments_.size()) + " 条记录");
        return true;
    } catch (const json::exception& e) {
        Logger::getInstance().error("解析选课数据JSON失败：" + std::string(e.what()));
        throw SystemException(ErrorType::DATA_INVALID, "解析选课数据失败：" + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::getInstance().error("加载选课数据失败：" + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "加载选课数据失败：" + std::string(e.what()));
    }
}

bool EnrollmentManager::saveData(bool alreadyLocked) {
    try {
        std::unique_ptr<LockGuard> lockPtr;
        if (!alreadyLocked) {
            lockPtr = std::make_unique<LockGuard>(mutex_, 5000); // 设置5秒超时
            if (!lockPtr->isLocked()) {
                throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
            }
        }
        
        json enrollmentsJson = json::array();
        
        for (const auto& pair : enrollments_) {
            const Enrollment* enrollment = pair.second.get();
            json enrollmentJson;
            
            enrollmentJson["studentId"] = enrollment->getStudentId();
            enrollmentJson["courseId"] = enrollment->getCourseId();
            enrollmentJson["enrollmentTime"] = enrollment->getEnrollmentTime();
            
            enrollmentsJson.push_back(enrollmentJson);
        }
        
        std::string jsonStr = enrollmentsJson.dump(4); 
        
        DataManager& dataManager = DataManager::getInstance();
        bool result = dataManager.saveJsonToFile("enrollment.json", jsonStr);
        
        if (result) {
            Logger::getInstance().info("成功保存选课数据，共 " + std::to_string(enrollments_.size()) + " 条记录");
        } 

        return result;
    } catch (const json::exception& e) {
        Logger::getInstance().error("生成选课数据JSON失败：" + std::string(e.what()));
        throw SystemException(ErrorType::DATA_INVALID, "生成选课数据失败：" + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::getInstance().error("保存选课数据失败：" + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "保存选课数据失败：" + std::string(e.what()));
    }
}

bool EnrollmentManager::removeEnrollment(const std::string& studentId, const std::string& courseId) {
    if (studentId.empty() || courseId.empty()) {
        Logger::getInstance().warning("移除选课记录失败：学生ID或课程ID为空");
        return false;
    }
    
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
    }
    
    std::string key = generateKey(studentId, courseId);
    auto it = enrollments_.find(key);
    
    if (it == enrollments_.end()) {
        // 记录不存在，视为移除成功
        return true;
    }
    
    // 从哈希表中移除记录
    enrollments_.erase(it);
    Logger::getInstance().info("成功移除选课记录：学生 " + studentId + " 和课程 " + courseId);
    return true;
}
