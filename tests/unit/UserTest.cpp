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
#include "../../include/model/User.h"

// User类的测试fixture
class UserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
    }

    void TearDown() override {
        // 清理测试环境
    }
};

// 测试Admin类的构造函数和基本功能
TEST_F(UserTest, AdminConstructorAndBasicFunctions) {
    Admin admin("admin001", "管理员", "password123");
    
    EXPECT_EQ("admin001", admin.getId());
    EXPECT_EQ("管理员", admin.getName());
    EXPECT_TRUE(admin.verifyPassword("password123"));
    EXPECT_FALSE(admin.verifyPassword("wrongpassword"));
    EXPECT_EQ(UserType::ADMIN, admin.getType());
}

// 测试Teacher类的构造函数和基本功能
TEST_F(UserTest, TeacherConstructorAndBasicFunctions) {
    Teacher teacher("teacher001", "教师", "password123", 
                   "计算机科学", "教授", "teacher@example.com");
    
    EXPECT_EQ("teacher001", teacher.getId());
    EXPECT_EQ("教师", teacher.getName());
    EXPECT_EQ("计算机科学", teacher.getDepartment());
    EXPECT_EQ("教授", teacher.getTitle());
    EXPECT_EQ("teacher@example.com", teacher.getContact());
    EXPECT_TRUE(teacher.verifyPassword("password123"));
    EXPECT_FALSE(teacher.verifyPassword("wrongpassword"));
    EXPECT_EQ(UserType::TEACHER, teacher.getType());
}

// 测试Student类的构造函数和基本功能
TEST_F(UserTest, StudentConstructorAndBasicFunctions) {
    Student student("student001", "学生", "password123",
                  "男", 20, "计算机科学", "计算机2班", "student@example.com");
    
    EXPECT_EQ("student001", student.getId());
    EXPECT_EQ("学生", student.getName());
    EXPECT_EQ("男", student.getGender());
    EXPECT_EQ(20, student.getAge());
    EXPECT_EQ("计算机科学", student.getDepartment());
    EXPECT_EQ("计算机2班", student.getClassInfo());
    EXPECT_EQ("student@example.com", student.getContact());
    EXPECT_TRUE(student.verifyPassword("password123"));
    EXPECT_FALSE(student.verifyPassword("wrongpassword"));
    EXPECT_EQ(UserType::STUDENT, student.getType());
}

// 测试修改用户属性
TEST_F(UserTest, ModifyUserProperties) {
    Student student("student001", "原名", "password123",
                  "男", 20, "计算机科学", "计算机2班", "student@example.com");
    
    // 修改属性
    student.setName("新名");
    student.setAge(21);
    student.setGender("女");
    student.setDepartment("物理学");
    student.setClassInfo("物理1班");
    student.setContact("new@example.com");
    student.setPassword("newpassword");
    
    // 验证修改后的属性
    EXPECT_EQ("新名", student.getName());
    EXPECT_EQ(21, student.getAge());
    EXPECT_EQ("女", student.getGender());
    EXPECT_EQ("物理学", student.getDepartment());
    EXPECT_EQ("物理1班", student.getClassInfo());
    EXPECT_EQ("new@example.com", student.getContact());
    EXPECT_TRUE(student.verifyPassword("newpassword"));
    EXPECT_FALSE(student.verifyPassword("password123"));
}

// 测试移动构造函数和移动赋值运算符
TEST_F(UserTest, MoveOperations) {
    Student student1("student001", "学生1", "password123",
                   "男", 20, "计算机科学", "计算机2班", "student1@example.com");
    
    // 测试移动构造函数
    Student student2(std::move(student1));
    EXPECT_EQ("student001", student2.getId());
    EXPECT_EQ("学生1", student2.getName());
    EXPECT_EQ("男", student2.getGender());
    EXPECT_EQ(20, student2.getAge());
    
    // 创建新学生用于测试移动赋值运算符
    Student student3("student003", "学生3", "password123",
                   "女", 22, "数学", "数学1班", "student3@example.com");
    
    Student student4("student004", "学生4", "password123",
                   "男", 23, "物理", "物理1班", "student4@example.com");
    
    // 测试移动赋值运算符
    student4 = std::move(student3);
    EXPECT_EQ("student003", student4.getId());
    EXPECT_EQ("学生3", student4.getName());
    EXPECT_EQ("女", student4.getGender());
    EXPECT_EQ(22, student4.getAge());
}

// 测试密码修改功能
TEST_F(UserTest, PasswordChangeTest) {
    Student student("test001", "密码测试", "oldpassword",
                   "男", 20, "计算机科学", "计算机2班", "test@example.com");
    
    // 验证原始密码
    EXPECT_TRUE(student.verifyPassword("oldpassword"));
    EXPECT_FALSE(student.verifyPassword("newpassword"));
    
    // 修改密码
    student.setPassword("newpassword");
    
    // 验证新密码
    EXPECT_TRUE(student.verifyPassword("newpassword"));
    EXPECT_FALSE(student.verifyPassword("oldpassword"));
    
    // 验证盐值已更新
    std::string originalSalt = student.salt_;
    EXPECT_FALSE(originalSalt.empty());
    
    // 再次修改密码
    student.setPassword("anotherpassword");
    
    // 验证密码和盐值
    EXPECT_TRUE(student.verifyPassword("anotherpassword"));
    EXPECT_FALSE(student.verifyPassword("newpassword"));
    EXPECT_FALSE(student.verifyPassword("oldpassword"));
    
    // 验证盐值已再次更新（盐值应该发生变化）
    EXPECT_NE(originalSalt, student.salt_);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 