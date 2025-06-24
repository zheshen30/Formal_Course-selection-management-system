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

#include <iostream>
#include <string>
#include <filesystem>

namespace TestUtils {
    /**
     * @brief 清理测试目录内容，但保留目录本身
     * @param dir 要清理的目录路径
     * @param verbose 是否打印详细信息
     */
    inline void cleanTestDirectory(const std::string& dir, bool verbose = true) {
        try {
            if (std::filesystem::exists(dir)) {
                if (verbose) {
                    std::cout << "清理测试目录内容: " << dir << std::endl;
                }
                
                // 只删除目录下的文件和子目录，保留目录本身
                for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                    if (verbose) {
                        std::cout << "  - 删除: " << entry.path().filename() << std::endl;
                    }
                    std::filesystem::remove_all(entry.path());
                }
                
                if (verbose) {
                    std::cout << "测试目录内容清理完成: " << dir << std::endl;
                }
            } else if (verbose) {
                std::cout << "测试目录不存在: " << dir << std::endl;
                // 创建目录（如果不存在）
                std::filesystem::create_directories(dir);
                std::cout << "创建测试目录: " << dir << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "清理测试目录异常: " << e.what() << std::endl;
        }
    }
}