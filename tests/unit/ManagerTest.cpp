#include <gtest/gtest.h>
#include <memory>
#include "../../include/manager/UserManager.h"
#include "../../include/manager/CourseManager.h"
#include "../../include/manager/EnrollmentManager.h"
#include "../../include/util/DataManager.h"

// Manager类的测试fixture
class ManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        DataManager::getInstance().setDataDirectory("./test_data");
    }

    void TearDown() override {
        // 清理测试环境
        // 可以删除测试数据文件
    }
};

// 测试UserManager基本功能
TEST_F(ManagerTest, UserManagerBasicFunctions) {
    UserManager& userManager = UserManager::getInstance();
    
    // 创建测试用户
    std::unique_ptr<Student> student = std::make_unique<Student>(
        "test_student", "测试学生", "password",
        "男", 20, "计算机科学", "计算机1班", "test@example.com"
    );
    
    // 添加用户
    EXPECT_TRUE(userManager.addUser(std::move(student)));
    
    // 获取用户
    User* user = userManager.getUser("test_student");
    ASSERT_NE(nullptr, user);
    EXPECT_EQ("test_student", user->getId());
    EXPECT_EQ("测试学生", user->getName());
    EXPECT_EQ(UserType::STUDENT, user->getType());
    
    // 验证密码
    EXPECT_TRUE(userManager.verifyPassword("test_student", "password"));
    EXPECT_FALSE(userManager.verifyPassword("test_student", "wrong_password"));
    
    // 获取不存在的用户
    EXPECT_EQ(nullptr, userManager.getUser("non_existent_user"));
    
    // 移除用户
    EXPECT_TRUE(userManager.removeUser("test_student"));
    EXPECT_EQ(nullptr, userManager.getUser("test_student"));
}

// 测试CourseManager基本功能
TEST_F(ManagerTest, CourseManagerBasicFunctions) {
    CourseManager& courseManager = CourseManager::getInstance();
    
    // 创建测试课程
    std::unique_ptr<Course> course = std::make_unique<Course>(
        "TEST101", "测试课程", CourseType::REQUIRED,
        3.0, 48, "2023秋季", "teacher001", 50
    );
    
    // 添加课程
    EXPECT_TRUE(courseManager.addCourse(std::move(course)));
    
    // 获取课程
    Course* retrievedCourse = courseManager.getCourse("TEST101");
    ASSERT_NE(nullptr, retrievedCourse);
    EXPECT_EQ("TEST101", retrievedCourse->getId());
    EXPECT_EQ("测试课程", retrievedCourse->getName());
    EXPECT_EQ(CourseType::REQUIRED, retrievedCourse->getType());
    
    // 获取不存在的课程
    EXPECT_EQ(nullptr, courseManager.getCourse("non_existent_course"));
    
    // 移除课程
    EXPECT_TRUE(courseManager.removeCourse("TEST101"));
    EXPECT_EQ(nullptr, courseManager.getCourse("TEST101"));
}

// 测试EnrollmentManager基本功能
TEST_F(ManagerTest, EnrollmentManagerBasicFunctions) {
    // 设置测试环境
    UserManager& userManager = UserManager::getInstance();
    CourseManager& courseManager = CourseManager::getInstance();
    EnrollmentManager& enrollmentManager = EnrollmentManager::getInstance();
    
    // 创建测试用户和课程
    std::unique_ptr<Student> student = std::make_unique<Student>(
        "test_student", "测试学生", "password",
        "男", 20, "计算机科学", "计算机1班", "test@example.com"
    );
    userManager.addUser(std::move(student));
    
    std::unique_ptr<Course> course = std::make_unique<Course>(
        "TEST101", "测试课程", CourseType::REQUIRED,
        3.0, 48, "2023秋季", "teacher001", 50
    );
    courseManager.addCourse(std::move(course));
    
    // 测试选课
    EXPECT_TRUE(enrollmentManager.enrollCourse("test_student", "TEST101"));
    
    // 验证选课状态
    EXPECT_TRUE(enrollmentManager.isEnrolled("test_student", "TEST101"));
    
    // 获取选课记录
    Enrollment* enrollment = enrollmentManager.getEnrollment("test_student", "TEST101");
    ASSERT_NE(nullptr, enrollment);
    EXPECT_EQ("test_student", enrollment->getStudentId());
    EXPECT_EQ("TEST101", enrollment->getCourseId());
    EXPECT_EQ(EnrollmentStatus::ENROLLED, enrollment->getStatus());
    
    // 获取学生选课列表
    std::vector<Enrollment*> studentEnrollments = enrollmentManager.getStudentEnrollments("test_student");
    EXPECT_EQ(1, studentEnrollments.size());
    
    // 获取课程选课列表
    std::vector<Enrollment*> courseEnrollments = enrollmentManager.getCourseEnrollments("TEST101");
    EXPECT_EQ(1, courseEnrollments.size());
    
    // 测试退课
    EXPECT_TRUE(enrollmentManager.dropCourse("test_student", "TEST101"));
    EXPECT_FALSE(enrollmentManager.isEnrolled("test_student", "TEST101"));
    
    // 清理测试数据
    userManager.removeUser("test_student");
    courseManager.removeCourse("TEST101");
}

// 测试Manager类的查询功能
TEST_F(ManagerTest, ManagerQueryFunctions) {
    UserManager& userManager = UserManager::getInstance();
    CourseManager& courseManager = CourseManager::getInstance();
    
    // 创建多个测试用户
    std::unique_ptr<Student> student1 = std::make_unique<Student>(
        "student001", "学生1", "password",
        "男", 20, "计算机科学", "计算机1班", "student1@example.com"
    );
    userManager.addUser(std::move(student1));
    
    std::unique_ptr<Student> student2 = std::make_unique<Student>(
        "student002", "学生2", "password",
        "女", 21, "计算机科学", "计算机2班", "student2@example.com"
    );
    userManager.addUser(std::move(student2));
    
    // 创建多个测试课程
    std::unique_ptr<Course> course1 = std::make_unique<Course>(
        "CS101", "计算机导论", CourseType::REQUIRED,
        3.0, 48, "2023秋季", "teacher001", 50
    );
    courseManager.addCourse(std::move(course1));
    
    std::unique_ptr<Course> course2 = std::make_unique<Course>(
        "CS102", "数据结构", CourseType::REQUIRED,
        4.0, 64, "2023秋季", "teacher002", 40
    );
    courseManager.addCourse(std::move(course2));
    
    // 测试UserManager查询功能
    std::vector<User*> computerStudents = userManager.findUsers(
        [](const User& user) {
            if (user.getType() == UserType::STUDENT) {
                const Student& student = static_cast<const Student&>(user);
                return student.getDepartment() == "计算机科学";
            }
            return false;
        }
    );
    EXPECT_EQ(2, computerStudents.size());
    
    // 测试CourseManager查询功能
    std::vector<Course*> requiredCourses = courseManager.findCourses(
        [](const Course& course) {
            return course.getType() == CourseType::REQUIRED;
        }
    );
    EXPECT_EQ(2, requiredCourses.size());
    
    // 清理测试数据
    userManager.removeUser("student001");
    userManager.removeUser("student002");
    courseManager.removeCourse("CS101");
    courseManager.removeCourse("CS102");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 