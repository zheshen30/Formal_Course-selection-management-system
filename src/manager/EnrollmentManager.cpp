#include "../../include/manager/EnrollmentManager.h"
#include "../../include/manager/UserManager.h"
#include "../../include/manager/CourseManager.h"
#include "../../include/util/DataManager.h"
#include "../../include/system/LockGuard.h"
#include "../../include/system/SystemException.h"
#include "../../include/util/Logger.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <stdexcept>
#include <sstream>

using json = nlohmann::json;

EnrollmentManager& EnrollmentManager::getInstance() {
    static EnrollmentManager instance;
    return instance;
}

bool EnrollmentManager::enrollCourse(const std::string& studentId, const std::string& courseId) {
    // 检查参数
    if (studentId.empty() || courseId.empty()) {
        Logger::getInstance().error("选课失败：学生ID或课程ID为空");
        return false;
    }
    
    try {
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
        
        // 创建选课记录并添加
        auto enrollment = std::make_unique<Enrollment>(studentId, courseId);
        bool enrollmentAdded = addEnrollment(std::move(enrollment));
        
        if (!enrollmentAdded) {
            Logger::getInstance().error("选课失败：无法添加选课记录");
            return false;
        }
        
        // 更新课程的学生列表
        bool studentAdded = course->addStudent(studentId);
        if (!studentAdded) {
            // 如果学生添加失败，回滚选课记录
            enrollments_.erase(generateKey(studentId, courseId));
            Logger::getInstance().error("选课失败：无法将学生添加到课程");
            return false;
        }
        
        Logger::getInstance().info("选课成功：学生 " + studentId + " 选择课程 " + courseId);
        return true;
    } catch (const SystemException& e) {
        // 已处理的系统异常，重新抛出
        throw;
    } catch (const std::exception& e) {
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
        if (!enrollment || enrollment->getStatus() != EnrollmentStatus::ENROLLED) {
            Logger::getInstance().warning("退课失败：学生 " + studentId + " 未选课程 " + courseId);
            throw SystemException(ErrorType::NOT_ENROLLED, "学生未选择此课程");
        }
        
        // 验证课程存在
        CourseManager& courseManager = CourseManager::getInstance();
        Course* course = courseManager.getCourse(courseId);
        if (!course) {
            Logger::getInstance().warning("退课失败：课程ID " + courseId + " 不存在");
            return false;
        }
        
        // 更新选课记录状态为已退
        enrollment->setStatus(EnrollmentStatus::DROPPED);
        
        // 从课程的学生列表中移除学生
        bool removed = course->removeStudent(studentId);
        if (!removed) {
            Logger::getInstance().warning("退课警告：无法从课程 " + courseId + " 中移除学生 " + studentId);
        }
        
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
    
    if (it == enrollments_.end()) {
        return false;
    }
    
    // 检查状态是否为已选
    return it->second->getStatus() == EnrollmentStatus::ENROLLED;
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
            std::string statusStr = enrollmentJson["status"];
            std::string enrollmentTime = enrollmentJson["enrollmentTime"];
            
            EnrollmentStatus status;
            if (statusStr == "ENROLLED") {
                status = EnrollmentStatus::ENROLLED;
            } else if (statusStr == "DROPPED") {
                status = EnrollmentStatus::DROPPED;
            } else if (statusStr == "WAITLISTED") {
                status = EnrollmentStatus::WAITLISTED;
            } else {
                Logger::getInstance().warning("未知的选课状态：" + statusStr);
                status = EnrollmentStatus::ENROLLED; // 默认为已选
            }
            
            auto enrollment = std::make_unique<Enrollment>(studentId, courseId, status);
            enrollment->enrollmentTime_ = enrollmentTime; // 直接设置时间，避免使用当前时间
            
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

bool EnrollmentManager::saveData() {
    try {
        LockGuard lock(mutex_, 5000); // 设置5秒超时
        if (!lock.isLocked()) {
            throw SystemException(ErrorType::LOCK_TIMEOUT, "获取选课管理器锁超时");
        }
        
        json enrollmentsJson = json::array();
        
        for (const auto& pair : enrollments_) {
            const Enrollment* enrollment = pair.second.get();
            json enrollmentJson;
            
            enrollmentJson["studentId"] = enrollment->getStudentId();
            enrollmentJson["courseId"] = enrollment->getCourseId();
            enrollmentJson["enrollmentTime"] = enrollment->getEnrollmentTime();
            
            switch (enrollment->getStatus()) {
                case EnrollmentStatus::ENROLLED:
                    enrollmentJson["status"] = "ENROLLED";
                    break;
                case EnrollmentStatus::DROPPED:
                    enrollmentJson["status"] = "DROPPED";
                    break;
                case EnrollmentStatus::WAITLISTED:
                    enrollmentJson["status"] = "WAITLISTED";
                    break;
                default:
                    enrollmentJson["status"] = "ENROLLED"; // 默认为已选
                    break;
            }
            
            enrollmentsJson.push_back(enrollmentJson);
        }
        
        std::string jsonStr = enrollmentsJson.dump(4); // 格式化JSON，缩进4个空格
        
        DataManager& dataManager = DataManager::getInstance();
        bool result = dataManager.saveJsonToFile("enrollment.json", jsonStr);
        
        if (result) {
            Logger::getInstance().info("成功保存选课数据，共 " + std::to_string(enrollments_.size()) + " 条记录");
        } else {
            Logger::getInstance().error("保存选课数据失败");
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
