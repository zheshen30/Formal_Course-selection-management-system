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

// 标准库头文件
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>

// 第三方库头文件 - 这些通常是最大且编译最慢的
#include <json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// OpenSSL库头文件 - 根据CMake配置有条件地包含
#ifndef NO_OPENSSL
  #ifdef HAVE_OPENSSL
    #include <openssl/sha.h>
    #include <openssl/evp.h>
  #endif
#endif

// 项目常用的公共头文件
#include "system/SystemException.h"

// 注意：不要在预编译头文件中包含依赖于具体实现的头文件
// 预编译头文件应当包含稳定的、频繁使用的头文件 