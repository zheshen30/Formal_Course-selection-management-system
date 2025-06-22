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
#include <gtest/gtest.h>
#include "../../include/model/Course.h"

// Course类的测试fixture
class CourseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
    }

    void TearDown() override {
        // 清理测试环境
    }
};

// 测试Course类的构造函数和基本功能
TEST_F(CourseTest, ConstructorAndBasicFunctions) {
    Course course("CS101", "计算机导论", CourseType::REQUIRED, 
                 3.0, 48, "2023秋季", "teacher001", 50);
    
    EXPECT_EQ("CS101", course.getId());
    EXPECT_EQ("计算机导论", course.getName());
    EXPECT_EQ(CourseType::REQUIRED, course.getType());
    EXPECT_EQ(3.0, course.getCredit());
    EXPECT_EQ(48, course.getHours());
    EXPECT_EQ("2023秋季", course.getSemester());
    EXPECT_EQ("teacher001", course.getTeacherId());
    EXPECT_EQ(50, course.getMaxCapacity());
    EXPECT_EQ(0, course.getCurrentEnrollment());
    EXPECT_EQ(50, course.getAvailableSeats());
    EXPECT_FALSE(course.isFull());
}

// 测试修改课程属性
TEST_F(CourseTest, ModifyCourseProperties) {
    Course course("CS101", "计算机导论", CourseType::REQUIRED, 
                 3.0, 48, "2023秋季", "teacher001", 50);
    
    // 修改属性
    course.setName("高级计算机导论");
    course.setType(CourseType::ELECTIVE);
    course.setCredit(4.0);
    course.setHours(64);
    course.setSemester("2024春季");
    course.setTeacherId("teacher002");
    course.setMaxCapacity(60);
    
    // 验证修改后的属性
    EXPECT_EQ("高级计算机导论", course.getName());
    EXPECT_EQ(CourseType::ELECTIVE, course.getType());
    EXPECT_EQ(4.0, course.getCredit());
    EXPECT_EQ(64, course.getHours());
    EXPECT_EQ("2024春季", course.getSemester());
    EXPECT_EQ("teacher002", course.getTeacherId());
    EXPECT_EQ(60, course.getMaxCapacity());
}

// 测试学生选课和退课
TEST_F(CourseTest, StudentEnrollmentAndDrop) {
    Course course("CS101", "计算机导论", CourseType::REQUIRED, 
                 3.0, 48, "2023秋季", "teacher001", 2);
    
    // 添加学生
    EXPECT_TRUE(course.addStudent("student001"));
    EXPECT_EQ(1, course.getCurrentEnrollment());
    EXPECT_EQ(1, course.getAvailableSeats());
    EXPECT_FALSE(course.isFull());
    EXPECT_TRUE(course.hasStudent("student001"));
    
    // 添加另一个学生
    EXPECT_TRUE(course.addStudent("student002"));
    EXPECT_EQ(2, course.getCurrentEnrollment());
    EXPECT_EQ(0, course.getAvailableSeats());
    EXPECT_TRUE(course.isFull());
    EXPECT_TRUE(course.hasStudent("student002"));
    
    // 尝试添加第三个学生（应该失败，因为课程已满）
    EXPECT_FALSE(course.addStudent("student003"));
    EXPECT_EQ(2, course.getCurrentEnrollment());
    EXPECT_FALSE(course.hasStudent("student003"));
    
    // 移除学生
    EXPECT_TRUE(course.removeStudent("student001"));
    EXPECT_EQ(1, course.getCurrentEnrollment());
    EXPECT_EQ(1, course.getAvailableSeats());
    EXPECT_FALSE(course.isFull());
    EXPECT_FALSE(course.hasStudent("student001"));
    
    // 尝试移除不存在的学生
    EXPECT_FALSE(course.removeStudent("student999"));
    EXPECT_EQ(1, course.getCurrentEnrollment());
}

// 测试课程类型字符串
TEST_F(CourseTest, CourseTypeString) {
    Course requiredCourse("CS101", "必修课", CourseType::REQUIRED, 3.0, 48, "2023秋季", "teacher001", 50);
    Course electiveCourse("CS102", "选修课", CourseType::ELECTIVE, 2.0, 32, "2023秋季", "teacher001", 50);
    Course restrictedCourse("CS103", "限选课", CourseType::RESTRICTED, 2.5, 40, "2023秋季", "teacher001", 50);
    
    EXPECT_EQ("必修", requiredCourse.getTypeString());
    EXPECT_EQ("选修", electiveCourse.getTypeString());
    EXPECT_EQ("限选", restrictedCourse.getTypeString());
}

// 测试移动构造函数和移动赋值运算符
TEST_F(CourseTest, MoveOperations) {
    Course course1("CS101", "计算机导论", CourseType::REQUIRED, 
                  3.0, 48, "2023秋季", "teacher001", 50);
    
    // 添加学生以测试是否正确移动
    course1.addStudent("student001");
    course1.addStudent("student002");
    
    // 测试移动构造函数
    Course course2(std::move(course1));
    EXPECT_EQ("CS101", course2.getId());
    EXPECT_EQ("计算机导论", course2.getName());
    EXPECT_EQ(3.0, course2.getCredit());
    EXPECT_EQ(2, course2.getCurrentEnrollment());
    EXPECT_TRUE(course2.hasStudent("student001"));
    EXPECT_TRUE(course2.hasStudent("student002"));
    
    // 创建新课程用于测试移动赋值运算符
    Course course3("CS102", "数据结构", CourseType::REQUIRED, 
                  4.0, 64, "2023秋季", "teacher002", 40);
    
    Course course4("CS103", "算法设计", CourseType::ELECTIVE, 
                  3.5, 56, "2023秋季", "teacher003", 30);
    
    // 测试移动赋值运算符
    course4 = std::move(course3);
    EXPECT_EQ("CS102", course4.getId());
    EXPECT_EQ("数据结构", course4.getName());
    EXPECT_EQ(4.0, course4.getCredit());
    EXPECT_EQ(64, course4.getHours());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 