#include "../../include/manager/CourseManager.h"
#include "../../include/util/DataManager.h"
#include "../../include/system/LockGuard.h"
#include "../../include/system/SystemException.h"
#include "../../include/util/Logger.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <vector>
#include <stdexcept>

using json = nlohmann::json;

CourseManager& CourseManager::getInstance() {
    static CourseManager instance;
    return instance;
}

bool CourseManager::addCourse(std::unique_ptr<Course> course) {
    if (!course) {
        Logger::getInstance().error("尝试添加空课程对象");
        return false;
    }
    
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    const std::string& courseId = course->getId();
    if (courses_.find(courseId) != courses_.end()) {
        Logger::getInstance().warning("添加课程失败：课程ID " + courseId + " 已存在");
        return false;
    }
    
    courses_[courseId] = std::move(course);
    Logger::getInstance().info("成功添加课程: " + courseId);
    return true;
}

bool CourseManager::removeCourse(const std::string& courseId) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    auto it = courses_.find(courseId);
    if (it == courses_.end()) {
        Logger::getInstance().warning("移除课程失败：课程ID " + courseId + " 不存在");
        return false;
    }
    
    courses_.erase(it);
    Logger::getInstance().info("成功移除课程: " + courseId);
    return true;
}

Course* CourseManager::getCourse(const std::string& courseId) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    auto it = courses_.find(courseId);
    if (it == courses_.end()) {
        return nullptr;
    }
    
    return it->second.get();
}

bool CourseManager::updateCourseInfo(const Course& course) {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    auto it = courses_.find(course.getId());
    if (it == courses_.end()) {
        Logger::getInstance().warning("更新课程信息失败：课程ID " + course.getId() + " 不存在");
        return false;
    }
    
    Course* existingCourse = it->second.get();
    
    existingCourse->setName(course.getName());
    existingCourse->setType(course.getType());
    existingCourse->setCredit(course.getCredit());
    existingCourse->setHours(course.getHours());
    existingCourse->setSemester(course.getSemester());
    existingCourse->setTeacherId(course.getTeacherId());
    existingCourse->setMaxCapacity(course.getMaxCapacity());
    
    Logger::getInstance().info("成功更新课程信息: " + course.getId());
    return true;
}

std::vector<std::string> CourseManager::getAllCourseIds() const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    std::vector<std::string> courseIds;
    courseIds.reserve(courses_.size());
    
    for (const auto& pair : courses_) {
        courseIds.push_back(pair.first);
    }
    
    return courseIds;
}

std::vector<std::string> CourseManager::getTeacherCourseIds(const std::string& teacherId) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    std::vector<std::string> courseIds;
    
    for (const auto& pair : courses_) {
        if (pair.second->getTeacherId() == teacherId) {
            courseIds.push_back(pair.first);
        }
    }
    
    return courseIds;
}

std::vector<std::string> CourseManager::getStudentEnrolledCourseIds(const std::string& studentId) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    std::vector<std::string> courseIds;
    
    for (const auto& pair : courses_) {
        if (pair.second->hasStudent(studentId)) {
            courseIds.push_back(pair.first);
        }
    }
    
    return courseIds;
}

std::vector<std::string> CourseManager::findCourses(const std::function<bool(const Course&)>& predicate) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    std::vector<std::string> result;
    
    for (const auto& pair : courses_) {
        if (predicate(*(pair.second))) {
            result.push_back(pair.first);
        }
    }
    
    return result;
}

bool CourseManager::hasCourse(const std::string& courseId) const {
    LockGuard lock(mutex_, 5000); // 设置5秒超时
    if (!lock.isLocked()) {
        throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
    }
    
    return courses_.find(courseId) != courses_.end();
}

bool CourseManager::loadData() {
    try {
        LockGuard lock(mutex_, 5000); // 设置5秒超时
        if (!lock.isLocked()) {
            throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
        }
        
        DataManager& dataManager = DataManager::getInstance();
        std::string jsonStr = dataManager.loadJsonFromFile("courses.json");
        
        if (jsonStr.empty()) {
            Logger::getInstance().warning("课程数据文件为空或不存在");
            return false;
        }
        
        json coursesJson = json::parse(jsonStr);
        courses_.clear();
        
        for (const auto& courseJson : coursesJson) {
            std::string id = courseJson["id"];
            std::string name = courseJson["name"];
            std::string typeStr = courseJson["type"];
            double credit = courseJson["credit"];
            int hours = courseJson["hours"];
            std::string semester = courseJson["semester"];
            std::string teacherId = courseJson["teacherId"];
            int maxCapacity = courseJson["maxCapacity"];
            
            CourseType type;
            if (typeStr == "REQUIRED") {
                type = CourseType::REQUIRED;
            } else if (typeStr == "ELECTIVE") {
                type = CourseType::ELECTIVE;
            } else if (typeStr == "RESTRICTED") {
                type = CourseType::RESTRICTED;
            } else {
                Logger::getInstance().warning("未知的课程类型：" + typeStr);
                type = CourseType::ELECTIVE; // 默认为选修课
            }
            
            auto course = std::make_unique<Course>(
                id, name, type, credit, hours, semester, teacherId, maxCapacity);
            
            // 加载已选学生
            if (courseJson.contains("enrolledStudents") && courseJson["enrolledStudents"].is_array()) {
                for (const auto& studentId : courseJson["enrolledStudents"]) {
                    course->addStudent(studentId);
                }
            }
            
            courses_[id] = std::move(course);
        }
        
        Logger::getInstance().info("成功加载课程数据，共 " + std::to_string(courses_.size()) + " 个课程");
        return true;
    } catch (const json::exception& e) {
        Logger::getInstance().error("解析课程数据JSON失败：" + std::string(e.what()));
        throw SystemException(ErrorType::DATA_INVALID, "解析课程数据失败：" + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::getInstance().error("加载课程数据失败：" + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "加载课程数据失败：" + std::string(e.what()));
    }
}

bool CourseManager::saveData() {
    try {
        LockGuard lock(mutex_, 5000); // 设置5秒超时
        if (!lock.isLocked()) {
            throw SystemException(ErrorType::LOCK_TIMEOUT, "获取课程管理器锁超时");
        }
        
        json coursesJson = json::array();
        
        for (const auto& pair : courses_) {
            const Course* course = pair.second.get();
            json courseJson;
            
            courseJson["id"] = course->getId();
            courseJson["name"] = course->getName();
            
            switch (course->getType()) {
                case CourseType::REQUIRED:
                    courseJson["type"] = "REQUIRED";
                    break;
                case CourseType::ELECTIVE:
                    courseJson["type"] = "ELECTIVE";
                    break;
                case CourseType::RESTRICTED:
                    courseJson["type"] = "RESTRICTED";
                    break;
                default:
                    courseJson["type"] = "ELECTIVE"; // 默认为选修课
                    break;
            }
            
            courseJson["credit"] = course->getCredit();
            courseJson["hours"] = course->getHours();
            courseJson["semester"] = course->getSemester();
            courseJson["teacherId"] = course->getTeacherId();
            courseJson["maxCapacity"] = course->getMaxCapacity();
            
            // 保存已选学生
            json enrolledStudents = json::array();
            for (const auto& studentId : course->getEnrolledStudents()) {
                enrolledStudents.push_back(studentId);
            }
            courseJson["enrolledStudents"] = enrolledStudents;
            
            coursesJson.push_back(courseJson);
        }
        
        std::string jsonStr = coursesJson.dump(4); // 格式化JSON，缩进4个空格
        
        DataManager& dataManager = DataManager::getInstance();
        bool result = dataManager.saveJsonToFile("courses.json", jsonStr);
        
        if (result) {
            Logger::getInstance().info("成功保存课程数据，共 " + std::to_string(courses_.size()) + " 个课程");
        } else {
            Logger::getInstance().error("保存课程数据失败");
        }
        
        return result;
    } catch (const json::exception& e) {
        Logger::getInstance().error("生成课程数据JSON失败：" + std::string(e.what()));
        throw SystemException(ErrorType::DATA_INVALID, "生成课程数据失败：" + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::getInstance().error("保存课程数据失败：" + std::string(e.what()));
        throw SystemException(ErrorType::OPERATION_FAILED, "保存课程数据失败：" + std::string(e.what()));
    }
}
