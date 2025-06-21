cmake_minimum_required(VERSION 3.10)

# 查找GoogleTest
find_package(GTest REQUIRED)

# 添加User测试可执行文件
add_executable(test_user tests/unit/UserTest.cpp ../src/model/User.cpp)

# 包含头文件目录
target_include_directories(test_user PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_user PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
    OpenSSL::SSL
    OpenSSL::Crypto
)

# 添加Course测试可执行文件
add_executable(test_course tests/unit/CourseTest.cpp ../src/model/Course.cpp)

# 包含头文件目录
target_include_directories(test_course PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_course PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
)

# 添加Enrollment测试可执行文件
add_executable(test_enrollment tests/unit/EnrollmentTest.cpp ../src/model/Enrollment.cpp)

# 包含头文件目录
target_include_directories(test_enrollment PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_enrollment PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
)

# 添加Util测试可执行文件
add_executable(test_util tests/unit/UtilTest.cpp 
    ../src/util/DataManager.cpp
    ../src/util/Logger.cpp
    ../src/util/I18nManager.cpp
    ../src/util/InputValidator.cpp
)

# 包含头文件目录
target_include_directories(test_util PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_util PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# 添加Manager测试可执行文件
add_executable(test_manager tests/unit/ManagerTest.cpp 
    ../src/model/User.cpp
    ../src/model/Course.cpp
    ../src/model/Enrollment.cpp
    ../src/manager/UserManager.cpp
    ../src/manager/CourseManager.cpp
    ../src/manager/EnrollmentManager.cpp
    ../src/util/DataManager.cpp
)

# 包含头文件目录
target_include_directories(test_manager PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_manager PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

# 添加System测试可执行文件
add_executable(test_system tests/unit/SystemTest.cpp 
    ../src/system/CourseSystem.cpp
    ../src/system/LockGuard.cpp
    ../src/system/SystemException.cpp
    ../src/model/User.cpp
    ../src/manager/UserManager.cpp
    ../src/util/Logger.cpp
    ../src/util/I18nManager.cpp
    ../src/util/DataManager.cpp
)

# 包含头文件目录
target_include_directories(test_system PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_system PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# 添加并发测试可执行文件
add_executable(test_concurrency tests/unit/ConcurrencyTest.cpp 
    ../src/model/User.cpp
    ../src/model/Course.cpp
    ../src/model/Enrollment.cpp
    ../src/manager/UserManager.cpp
    ../src/manager/CourseManager.cpp
    ../src/manager/EnrollmentManager.cpp
    ../src/system/LockGuard.cpp
    ../src/util/DataManager.cpp
)

# 包含头文件目录
target_include_directories(test_concurrency PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_concurrency PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# 添加集成测试可执行文件
add_executable(test_integration tests/integration/SystemIntegrationTest.cpp
    ../src/model/User.cpp
    ../src/model/Course.cpp
    ../src/model/Enrollment.cpp
    ../src/manager/UserManager.cpp
    ../src/manager/CourseManager.cpp
    ../src/manager/EnrollmentManager.cpp
    ../src/system/CourseSystem.cpp
    ../src/system/LockGuard.cpp
    ../src/system/SystemException.cpp
    ../src/util/Logger.cpp
    ../src/util/I18nManager.cpp
    ../src/util/DataManager.cpp
    ../src/util/InputValidator.cpp
)

# 包含头文件目录
target_include_directories(test_integration PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_integration PRIVATE
    ${GTEST_LIBRARIES}
    GTest::Main
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# 添加测试
add_test(NAME UserTest COMMAND test_user)
add_test(NAME CourseTest COMMAND test_course)
add_test(NAME EnrollmentTest COMMAND test_enrollment)
add_test(NAME UtilTest COMMAND test_util)
add_test(NAME ManagerTest COMMAND test_manager)
add_test(NAME SystemTest COMMAND test_system)
add_test(NAME ConcurrencyTest COMMAND test_concurrency)
add_test(NAME IntegrationTest COMMAND test_integration) 