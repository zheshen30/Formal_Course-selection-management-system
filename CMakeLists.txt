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

# 项目名称和版本
project(CourseSelectionSystem VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 启用预编译头文件支持
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    # CMAKE 3.16+原生支持预编译头文件
    set(USE_NATIVE_PCH TRUE)
else()
    # 较旧版本使用自定义逻辑
    set(USE_NATIVE_PCH FALSE)
endif()

# 添加编译选项
if(MSVC)
    add_compile_options(/W4)
    set(PCH_FLAG "/Yc")
    set(USE_PCH_FLAG "/Yu")
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 添加本地依赖库路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/nlohmann/cmake")
set(JSON_BuildTests OFF CACHE INTERNAL "")

# 添加子目录
# nlohmann/json是仅头文件的库，不需要add_subdirectory

# 设置OpenSSL路径 - 作为查找提示而非直接指定
set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/dependlib/openssl")

# 尝试查找OpenSSL库 (将REQUIRED改为QUIET)
find_package(OpenSSL QUIET)

# 输出详细的OpenSSL查找信息
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL Found: YES")
    message(STATUS "OpenSSL Version: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include Dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
else()
    message(WARNING "OpenSSL NOT FOUND! Using manual configuration...")
    message(STATUS "检查本地OpenSSL目录: ${OPENSSL_ROOT_DIR}")
    
    # 手动检查OpenSSL目录结构
    if(EXISTS "${OPENSSL_ROOT_DIR}/include/openssl/ssl.h")
        message(STATUS "找到OpenSSL头文件")
        set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
        
        # 检查库文件
        if(WIN32)
            set(SSL_LIBRARY_NAME "libssl")
            set(CRYPTO_LIBRARY_NAME "libcrypto")
            set(LIB_SUFFIX ".lib")
        else()
            set(SSL_LIBRARY_NAME "libssl")
            set(CRYPTO_LIBRARY_NAME "libcrypto")
            set(LIB_SUFFIX ".so")
        endif()
        
        # 尝试找到库文件
        find_library(OPENSSL_SSL_LIBRARY
            NAMES ${SSL_LIBRARY_NAME} ${SSL_LIBRARY_NAME}${LIB_SUFFIX}
            PATHS ${OPENSSL_ROOT_DIR}/lib
            NO_DEFAULT_PATH
        )
        find_library(OPENSSL_CRYPTO_LIBRARY
            NAMES ${CRYPTO_LIBRARY_NAME} ${CRYPTO_LIBRARY_NAME}${LIB_SUFFIX}
            PATHS ${OPENSSL_ROOT_DIR}/lib
            NO_DEFAULT_PATH
        )
        
        # 输出库文件路径
        message(STATUS "SSL库路径: ${OPENSSL_SSL_LIBRARY}")
        message(STATUS "Crypto库路径: ${OPENSSL_CRYPTO_LIBRARY}")
        
        if(OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
            message(STATUS "找到OpenSSL库文件")
            set(OPENSSL_LIBRARIES "${OPENSSL_SSL_LIBRARY};${OPENSSL_CRYPTO_LIBRARY}")
            set(OPENSSL_FOUND TRUE)
        else()
            message(WARNING "找不到OpenSSL库文件!")
            message(STATUS "请检查依赖库是否正确安装")
            message(STATUS "依赖库目录结构:")
            execute_process(
                COMMAND ls -la ${OPENSSL_ROOT_DIR}
                OUTPUT_VARIABLE OPENSSL_DIR_CONTENT
            )
            message(STATUS "${OPENSSL_DIR_CONTENT}")
            
            if(EXISTS "${OPENSSL_ROOT_DIR}/lib")
                execute_process(
                    COMMAND ls -la ${OPENSSL_ROOT_DIR}/lib
                    OUTPUT_VARIABLE OPENSSL_LIB_CONTENT
                )
                message(STATUS "lib目录内容: ${OPENSSL_LIB_CONTENT}")
            endif()
        endif()
    else()
        message(WARNING "找不到OpenSSL头文件!")
        message(STATUS "头文件应位于: ${OPENSSL_ROOT_DIR}/include/openssl/ssl.h")
        message(STATUS "请检查依赖库是否正确安装")
    endif()
endif()

# 查找必要的依赖库
find_package(Threads REQUIRED)

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

# 创建pch.cpp文件
file(WRITE ${PCH_SOURCE} "#include \"${PCH_HEADER}\"\n")

# 添加pch.cpp到构建
target_sources(course_system PRIVATE ${PCH_SOURCE})

# 设置预编译头文件
if(USE_NATIVE_PCH)
    # 使用CMake 3.16+原生支持
    target_precompile_headers(course_system PRIVATE ${PCH_HEADER})
else()
    # 手动配置预编译头文件
    if(MSVC)
        # 对pch.cpp源文件设置/Yc标志来生成PCH
        set_source_files_properties(${PCH_SOURCE} PROPERTIES COMPILE_FLAGS "${PCH_FLAG}${PCH_HEADER}")
        
        # 对其他所有源文件设置/Yu标志使用PCH
        foreach(SOURCE ${SOURCES})
            set_source_files_properties(${SOURCE} PROPERTIES COMPILE_FLAGS "${USE_PCH_FLAG}${PCH_HEADER}")
        endforeach()
    elseif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
         #GCC和Clang使用-include选项
        get_filename_component(PCH_FILENAME ${PCH_HEADER} NAME)
        set(PCH_BINARY "${CMAKE_CURRENT_BINARY_DIR}/${PCH_FILENAME}.gch")
        
        # 生成PCH
        add_custom_command(
            OUTPUT ${PCH_BINARY}
            COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -x c++-header -o ${PCH_BINARY} ${PCH_HEADER}
            DEPENDS ${PCH_HEADER}
            COMMENT "Generating precompiled header"
        )
        
        # 将生成的PCH添加为依赖
        add_custom_target(pch DEPENDS ${PCH_BINARY})
        add_dependencies(course_system pch)
        
        # 对所有源文件使用PCH
        target_compile_options(course_system PRIVATE -Winvalid-pch -include ${PCH_HEADER})
    endif()
endif()

# 包含头文件目录
target_include_directories(course_system PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann
    ${OPENSSL_INCLUDE_DIR}
)

# 链接依赖库
if(OPENSSL_FOUND)
    if(TARGET OpenSSL::SSL AND TARGET OpenSSL::Crypto)
        target_link_libraries(course_system PRIVATE
            OpenSSL::SSL
            OpenSSL::Crypto
            Threads::Threads
        )
    else()
        # 手动配置的情况
        target_include_directories(course_system PRIVATE ${OPENSSL_INCLUDE_DIR})
        target_link_libraries(course_system PRIVATE
            ${OPENSSL_LIBRARIES}
            Threads::Threads
        )
    endif()
    
    # 添加宏定义以启用OpenSSL相关功能
    target_compile_definitions(course_system PRIVATE HAVE_OPENSSL)
else()
    message(WARNING "构建没有OpenSSL支持! 某些加密功能将不可用。")
    
    # 链接其他库，但不包括OpenSSL
    target_link_libraries(course_system PRIVATE
        Threads::Threads
    )
    
    # 添加宏定义以禁用OpenSSL相关功能
    target_compile_definitions(course_system PRIVATE NO_OPENSSL)
endif()

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
    ${CMAKE_CURRENT_BINARY_DIR}/data
    COMMENT "Copying data files to build directory"
)

# 显示项目信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Using precompiled headers: ${USE_NATIVE_PCH}")
