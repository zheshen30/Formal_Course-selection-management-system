cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(CourseSelectionSystem VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 查找必要的依赖库
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)

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

# 包含头文件目录
target_include_directories(course_system PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接依赖库
target_link_libraries(course_system PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# 安装目标
install(TARGETS course_system DESTINATION bin)

# 如果是Debug构建，添加测试
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_testing()
    add_subdirectory(tests)
endif()

# 添加自定义目标，复制数据文件到构建目录
add_custom_target(copy_data_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_BINARY_DIR}/data
)

# 确保数据文件在可执行文件生成前复制
add_dependencies(course_system copy_data_files)

# 显示项目信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
