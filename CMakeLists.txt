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

cmake_minimum_required(VERSION 3.16)

# 项目名称和版本
project(CourseSelectionSystem VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 启用预编译头文件支持
set(USE_NATIVE_PCH TRUE)
# 添加编译选项
if(MSVC)
    add_compile_options(/W4)
    set(PCH_FLAG "/Yc")
    set(USE_PCH_FLAG "/Yu")
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 查找OpenSSL库，系统标准位置和环境变量
find_package(OpenSSL REQUIRED)

# 查找线程库
find_package(Threads REQUIRED)

# 输出详细的OpenSSL查找信息
message(STATUS "OpenSSL已找到:")
message(STATUS "  版本: ${OPENSSL_VERSION}")
message(STATUS "  包含目录: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "  库文件: ${OPENSSL_LIBRARIES}")

# 设置源文件
file(GLOB_RECURSE SOURCES 
    "src/model/*.cpp"
    "src/manager/*.cpp"
    "src/system/*.cpp"
    "src/util/*.cpp"
    "src/main.cpp"
)

# 添加可执行文件
add_executable(course_system ${SOURCES})

# 预编译头文件路径
set(PCH_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
set(PCH_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.cpp")

# 添加pch.cpp
target_sources(course_system PRIVATE ${PCH_SOURCE})

# 设置预编译头文件
target_precompile_headers(course_system PRIVATE ${PCH_HEADER})
# 包含头文件目录
target_include_directories(course_system PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann
)

# 链接OpenSSL库，内部需要使用线程库，所以需要链接Threads::Threads
target_link_libraries(course_system PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# 显示项目信息
message(STATUS "项目: ${PROJECT_NAME}")
message(STATUS "版本: ${PROJECT_VERSION}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenSSL已启用")
