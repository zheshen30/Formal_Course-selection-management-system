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
#include "../../include/model/Enrollment.h"

// Enrollment类的测试fixture
class EnrollmentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
    }

    void TearDown() override {
        // 清理测试环境
    }
};

// 测试Enrollment类的构造函数和基本功能
TEST_F(EnrollmentTest, ConstructorAndBasicFunctions) {
    Enrollment enrollment("student001", "CS101");
    
    EXPECT_EQ("student001", enrollment.getStudentId());
    EXPECT_EQ("CS101", enrollment.getCourseId());
    EXPECT_FALSE(enrollment.getEnrollmentTime().empty());
}

// 测试移动构造函数和移动赋值运算符
TEST_F(EnrollmentTest, MoveOperations) {
    Enrollment enrollment1("student001", "CS101");
    std::string time1 = enrollment1.getEnrollmentTime();
    
    // 测试移动构造函数
    Enrollment enrollment2(std::move(enrollment1));
    EXPECT_EQ("student001", enrollment2.getStudentId());
    EXPECT_EQ("CS101", enrollment2.getCourseId());
    EXPECT_EQ(time1, enrollment2.getEnrollmentTime());
    
    // 创建新选课记录用于测试移动赋值运算符
    Enrollment enrollment3("student002", "CS102");
    
    Enrollment enrollment4("student003", "CS103");
    
    // 测试移动赋值运算符
    enrollment4 = std::move(enrollment3);
    EXPECT_EQ("student002", enrollment4.getStudentId());
    EXPECT_EQ("CS102", enrollment4.getCourseId());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 