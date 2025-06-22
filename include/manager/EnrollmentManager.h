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

/**
 * @brief 选课管理类，负责选课的CRUD操作和查询
 */
class EnrollmentManager {
public:
    /**
     * @brief 获取单例实例
     * @return EnrollmentManager单例引用
     */
    static EnrollmentManager& getInstance();

    /**
     * @brief 学生选课
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 是否选课成功
     */
    bool enrollCourse(const std::string& studentId, const std::string& courseId);

    /**
     * @brief 学生退课
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 是否退课成功
     */
    bool dropCourse(const std::string& studentId, const std::string& courseId);

    /**
     * @brief 获取选课记录
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 选课记录指针，不存在则返回nullptr
     */
    Enrollment* getEnrollment(const std::string& studentId, const std::string& courseId);

    /**
     * @brief 获取学生的所有选课
     * @param studentId 学生ID
     * @return 选课记录列表
     */
    std::vector<Enrollment*> getStudentEnrollments(const std::string& studentId) const;

    /**
     * @brief 获取课程的所有选课记录
     * @param courseId 课程ID
     * @return 选课记录列表
     */
    std::vector<Enrollment*> getCourseEnrollments(const std::string& courseId) const;

    /**
     * @brief 检查是否已选课
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 是否已选
     */
    bool isEnrolled(const std::string& studentId, const std::string& courseId) const;

    /**
     * @brief 查找选课记录
     * @param predicate 过滤谓词
     * @return 符合条件的选课记录列表
     */
    std::vector<Enrollment*> findEnrollments(
        const std::function<bool(const Enrollment&)>& predicate) const;

    /**
     * @brief 加载选课数据
     * @return 是否加载成功
     */
    bool loadData();

    /**
     * @brief 保存选课数据
     * @return 是否保存成功
     */
    bool saveData();

    /**
     * @brief 完全移除选课记录（主要用于测试）
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 是否成功移除
     */
    bool removeEnrollment(const std::string& studentId, const std::string& courseId);

private:
    /**
     * @brief 私有构造函数，确保单例
     */
    EnrollmentManager() = default;
    
    /**
     * @brief 删除拷贝构造函数
     */
    EnrollmentManager(const EnrollmentManager&) = delete;
    
    /**
     * @brief 删除赋值运算符
     */
    EnrollmentManager& operator=(const EnrollmentManager&) = delete;
    
    /**
     * @brief 添加选课记录
     * @param enrollment 选课记录对象
     * @return 是否添加成功
     */
    bool addEnrollment(std::unique_ptr<Enrollment> enrollment);
    
    /**
     * @brief 生成选课记录的唯一键
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 唯一键字符串
     */
    static std::string generateKey(const std::string& studentId, const std::string& courseId);
    
    std::unordered_map<std::string, std::unique_ptr<Enrollment>> enrollments_; ///< 选课记录映射表
    mutable std::mutex mutex_; ///< 互斥锁
};