# 
# Copyright (C) 2025 哲神
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
# 

cmake_minimum_required(VERSION 3.10)

# 添加必要的include目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../nlohmann
)

# 查找GoogleTest
find_package(GTest REQUIRED)

# 检查OpenSSL
if(OPENSSL_FOUND)
    set(OPENSSL_TARGETS "OpenSSL::SSL;OpenSSL::Crypto")
    set(OPENSSL_INCLUDES "${OPENSSL_INCLUDE_DIR}")
    set(OPENSSL_LIBS "${OPENSSL_LIBRARIES}")
    
    # 有OpenSSL
    add_compile_definitions(HAVE_OPENSSL)
else()
    # 没有OpenSSL
    add_compile_definitions(NO_OPENSSL)
endif()

# 预编译头文件设置
set(TEST_PCH_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/test_pch.h")
set(TEST_PCH_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/test_pch.cpp")

# 创建预编译头文件库
add_library(test_pch OBJECT ${TEST_PCH_SOURCE})
target_include_directories(test_pch PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../nlohmann
    ${GTEST_INCLUDE_DIRS}
    ${OPENSSL_INCLUDES}
)

# 添加GTEST_LINKED_AS_SHARED_LIBRARY定义，以避免预编译头文件警告
target_compile_definitions(test_pch PRIVATE GTEST_LINKED_AS_SHARED_LIBRARY=1)

# 使用CMake 3.16+的原生预编译头文件支持
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(test_pch PUBLIC ${TEST_PCH_HEADER})
else()
    # 较旧版本的CMake可以使用自定义命令生成预编译头文件
    if(MSVC)
        # MSVC特定设置
        target_compile_options(test_pch PRIVATE "/Yc${TEST_PCH_HEADER}")
    elseif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        # GCC/Clang特定设置
        get_filename_component(PCH_NAME ${TEST_PCH_HEADER} NAME)
        set(PCH_BINARY "${CMAKE_CURRENT_BINARY_DIR}/${PCH_NAME}.gch")
        add_custom_command(
            OUTPUT ${PCH_BINARY}
            COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -x c++-header -o ${PCH_BINARY} ${TEST_PCH_HEADER}
            DEPENDS ${TEST_PCH_HEADER}
            COMMENT "生成测试预编译头文件"
        )
        add_custom_target(generate_test_pch DEPENDS ${PCH_BINARY})
        add_dependencies(test_pch generate_test_pch)
    endif()
endif()

# 辅助宏，用于为测试添加预编译头文件支持
macro(add_test_pch target)
    # 链接和依赖
    target_link_libraries(${target} PRIVATE test_pch)
    add_dependencies(${target} test_pch)
    
    # 添加相同的编译定义，以确保与预编译头文件兼容
    target_compile_definitions(${target} PRIVATE GTEST_LINKED_AS_SHARED_LIBRARY=1)
    
    # 设置预编译头文件
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
        target_precompile_headers(${target} REUSE_FROM test_pch)
    else()
        # 较旧版本的手动设置
        if(MSVC)
            target_compile_options(${target} PRIVATE "/Yu${TEST_PCH_HEADER}" "/FI${TEST_PCH_HEADER}")
        elseif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
            target_compile_options(${target} PRIVATE "-include" "${TEST_PCH_HEADER}")
            add_dependencies(${target} generate_test_pch)
        endif()
    endif()
endmacro()

# 添加User测试可执行文件
add_executable(test_user unit/UserTest.cpp 
    ../src/model/User.cpp
    ../src/system/SystemException.cpp
)
add_test_pch(test_user)

# 包含头文件目录
target_include_directories(test_user PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
    ${OPENSSL_INCLUDES}
)

# 链接库
if(OPENSSL_FOUND)
    if(OPENSSL_TARGETS)
        target_link_libraries(test_user PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_TARGETS}
        )
    else()
        target_include_directories(test_user PRIVATE ${OPENSSL_INCLUDES})
        target_link_libraries(test_user PRIVATE
            ${GTEST_LIBRARIES}
           GTest::Main
            ${OPENSSL_LIBS}
        )
    endif()
else()
    target_link_libraries(test_user PRIVATE
        ${GTEST_LIBRARIES}
        GTest::Main
    )
endif()

# 添加Course测试可执行文件
add_executable(test_course unit/CourseTest.cpp 
    ../src/model/Course.cpp
    ../src/system/SystemException.cpp
)
add_test_pch(test_course)

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
add_executable(test_enrollment unit/EnrollmentTest.cpp 
    ../src/model/Enrollment.cpp
    ../src/system/SystemException.cpp
)
add_test_pch(test_enrollment)

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
add_executable(test_util unit/UtilTest.cpp 
    ../src/util/DataManager.cpp
    ../src/util/Logger.cpp
    ../src/util/I18nManager.cpp
    ../src/util/InputValidator.cpp
    ../src/system/SystemException.cpp
    ../src/system/LockGuard.cpp
)
add_test_pch(test_util)

# 包含头文件目录
target_include_directories(test_util PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
    ${OPENSSL_INCLUDES}
)

# 链接库
if(OPENSSL_FOUND)
    if(OPENSSL_TARGETS)
        target_link_libraries(test_util PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_TARGETS}
        )
    else()
        target_include_directories(test_util PRIVATE ${OPENSSL_INCLUDES})
        target_link_libraries(test_util PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_LIBS}
        )
    endif()
else()
    target_link_libraries(test_util PRIVATE
        ${GTEST_LIBRARIES}
        GTest::Main
    )
endif()

# 添加Manager测试可执行文件
add_executable(test_manager unit/ManagerTest.cpp 
    ../src/model/User.cpp
    ../src/model/Course.cpp
    ../src/model/Enrollment.cpp
    ../src/manager/UserManager.cpp
    ../src/manager/CourseManager.cpp
    ../src/manager/EnrollmentManager.cpp
    ../src/util/DataManager.cpp
    ../src/util/Logger.cpp
    ../src/system/SystemException.cpp
    ../src/system/LockGuard.cpp
)
add_test_pch(test_manager)

# 包含头文件目录
target_include_directories(test_manager PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
    ${OPENSSL_INCLUDES}
)

# 链接库
if(OPENSSL_FOUND)
    if(OPENSSL_TARGETS)
        target_link_libraries(test_manager PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_TARGETS}
        )
    else()
        target_include_directories(test_manager PRIVATE ${OPENSSL_INCLUDES})
        target_link_libraries(test_manager PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_LIBS}
        )
    endif()
else()
        target_link_libraries(test_manager PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
        )
endif()

# 添加System测试可执行文件
add_executable(test_system unit/SystemTest.cpp 
    ../src/system/CourseSystem.cpp
    ../src/system/LockGuard.cpp
    ../src/system/SystemException.cpp
    ../src/model/User.cpp
    ../src/model/Course.cpp
    ../src/model/Enrollment.cpp
    ../src/manager/UserManager.cpp
    ../src/manager/CourseManager.cpp
    ../src/manager/EnrollmentManager.cpp
    ../src/util/Logger.cpp
    ../src/util/I18nManager.cpp
    ../src/util/DataManager.cpp
    ../src/util/InputValidator.cpp
)
add_test_pch(test_system)

# 包含头文件目录
target_include_directories(test_system PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
    ${OPENSSL_INCLUDES}
)

# 链接库
if(OPENSSL_FOUND)
    if(OPENSSL_TARGETS)
        target_link_libraries(test_system PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_TARGETS}
        )
    else()
        target_include_directories(test_system PRIVATE ${OPENSSL_INCLUDES})
        target_link_libraries(test_system PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_LIBS}
        )
    endif()
else()
    target_link_libraries(test_system PRIVATE
        ${GTEST_LIBRARIES}
        GTest::Main
    )
endif()

# 添加并发测试可执行文件
add_executable(test_concurrency unit/ConcurrencyTest.cpp 
    ../src/model/User.cpp
    ../src/model/Course.cpp
    ../src/model/Enrollment.cpp
    ../src/manager/UserManager.cpp
    ../src/manager/CourseManager.cpp
    ../src/manager/EnrollmentManager.cpp
    ../src/system/SystemException.cpp
    ../src/system/LockGuard.cpp
    ../src/util/DataManager.cpp
    ../src/util/Logger.cpp
)
add_test_pch(test_concurrency)

# 包含头文件目录
target_include_directories(test_concurrency PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
    ${OPENSSL_INCLUDES}
)

# 链接库
if(OPENSSL_FOUND)
    if(OPENSSL_TARGETS)
        target_link_libraries(test_concurrency PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_TARGETS}
            Threads::Threads
        )
    else()
        target_include_directories(test_concurrency PRIVATE ${OPENSSL_INCLUDES})
        target_link_libraries(test_concurrency PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_LIBS}
            Threads::Threads
        )
    endif()
else()
    target_link_libraries(test_concurrency PRIVATE
        ${GTEST_LIBRARIES}
        GTest::Main
        Threads::Threads
    )
endif()

# 添加集成测试可执行文件
add_executable(test_integration integration/SystemIntegrationTest.cpp
    ../src/model/User.cpp
    ../src/model/Course.cpp
    ../src/model/Enrollment.cpp
    ../src/manager/UserManager.cpp
    ../src/manager/CourseManager.cpp
    ../src/manager/EnrollmentManager.cpp
    ../src/system/CourseSystem.cpp
    ../src/system/SystemException.cpp
    ../src/system/LockGuard.cpp
    ../src/util/DataManager.cpp
    ../src/util/Logger.cpp
    ../src/util/I18nManager.cpp
    ../src/util/InputValidator.cpp
)
add_test_pch(test_integration)

# 添加测试
add_test(NAME UserTest COMMAND test_user)
add_test(NAME CourseTest COMMAND test_course)
add_test(NAME EnrollmentTest COMMAND test_enrollment)
add_test(NAME UtilTest COMMAND test_util)
add_test(NAME ManagerTest COMMAND test_manager)
add_test(NAME SystemTest COMMAND test_system)
add_test(NAME ConcurrencyTest COMMAND test_concurrency)
add_test(NAME IntegrationTest COMMAND test_integration)

# 包含头文件目录
target_include_directories(test_integration PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GTEST_INCLUDE_DIRS}
    ${OPENSSL_INCLUDES}
)

# 链接库
if(OPENSSL_FOUND)
    if(OPENSSL_TARGETS)
        target_link_libraries(test_integration PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_TARGETS}
        )
    else()
        target_include_directories(test_integration PRIVATE ${OPENSSL_INCLUDES})
        target_link_libraries(test_integration PRIVATE
            ${GTEST_LIBRARIES}
            GTest::Main
            ${OPENSSL_LIBS}
        )
    endif()
else()
    target_link_libraries(test_integration PRIVATE
        ${GTEST_LIBRARIES}
        GTest::Main
    )
endif() 