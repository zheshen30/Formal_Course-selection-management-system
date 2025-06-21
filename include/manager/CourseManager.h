#pragma once

#include "../model/Course.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <functional>

/**
 * @brief 课程管理类，负责课程的CRUD操作和查询
 */
class CourseManager {
public:
    /**
     * @brief 获取单例实例
     * @return CourseManager单例引用
     */
    static CourseManager& getInstance();

    /**
     * @brief 添加课程
     * @param course 课程对象
     * @return 是否添加成功
     */
    bool addCourse(std::unique_ptr<Course> course);

    /**
     * @brief 移除课程
     * @param courseId 课程ID
     * @return 是否移除成功
     */
    bool removeCourse(const std::string& courseId);

    /**
     * @brief 获取课程
     * @param courseId 课程ID
     * @return 课程指针，不存在则返回nullptr
     */
    Course* getCourse(const std::string& courseId);

    /**
     * @brief 更新课程信息
     * @param course 课程对象
     * @return 是否更新成功
     */
    bool updateCourseInfo(const Course& course);

    /**
     * @brief 获取所有课程
     * @return 所有课程ID列表
     */
    std::vector<std::string> getAllCourseIds() const;

    /**
     * @brief 获取教师的所有课程
     * @param teacherId 教师ID
     * @return 课程ID列表
     */
    std::vector<std::string> getTeacherCourseIds(const std::string& teacherId) const;

    /**
     * @brief 获取包含特定学生的所有课程
     * @param studentId 学生ID
     * @return 课程ID列表
     */
    std::vector<std::string> getStudentEnrolledCourseIds(const std::string& studentId) const;

    /**
     * @brief 查找课程
     * @param predicate 过滤谓词
     * @return 符合条件的课程ID列表
     */
    std::vector<std::string> findCourses(const std::function<bool(const Course&)>& predicate) const;

    /**
     * @brief 检查课程是否存在
     * @param courseId 课程ID
     * @return 是否存在
     */
    bool hasCourse(const std::string& courseId) const;

    /**
     * @brief 加载课程数据
     * @return 是否加载成功
     */
    bool loadData();

    /**
     * @brief 保存课程数据
     * @return 是否保存成功
     */
    bool saveData();

private:
    /**
     * @brief 私有构造函数，确保单例
     */
    CourseManager() = default;
    
    /**
     * @brief 删除拷贝构造函数
     */
    CourseManager(const CourseManager&) = delete;
    
    /**
     * @brief 删除赋值运算符
     */
    CourseManager& operator=(const CourseManager&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<Course>> courses_; ///< 课程映射表
    mutable std::mutex mutex_; ///< 互斥锁
}; 