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

// 处理GTest动态库链接的宏定义
#ifdef GTEST_LINKED_AS_SHARED_LIBRARY
// 如果已定义，保持不变
#else
// 如果没有定义，定义它以保持一致性
#define GTEST_LINKED_AS_SHARED_LIBRARY
#endif

// GTest相关头文件
#include <gtest/gtest.h>

// C++标准库
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <chrono>
#include <thread>
#include <mutex>

// 项目主要头文件
#include "../include/model/User.h"
#include "../include/model/Course.h"
#include "../include/model/Enrollment.h"
#include "../include/manager/UserManager.h"
#include "../include/manager/CourseManager.h"
#include "../include/manager/EnrollmentManager.h"
#include "../include/system/CourseSystem.h"
#include "../include/system/SystemException.h"
#include "../include/system/LockGuard.h"
#include "../include/util/DataManager.h"
#include "../include/util/Logger.h"
#include "../include/util/I18nManager.h"
#include "../include/util/InputValidator.h"


// 使用命名空间
using namespace std::chrono_literals; 