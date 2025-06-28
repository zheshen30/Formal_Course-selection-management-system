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

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <unordered_map>

class DataManager {
public:
    static DataManager& getInstance();

    std::string loadJsonFromFile(const std::string& filename);

    bool saveJsonToFile(const std::string& filename, const std::string& jsonData);

    bool fileExists(const std::string& filename) const;

    bool createDirectory(const std::string& dirname) const;

    std::string getDataFilePath(const std::string& filename) const;

    void setDataDirectory(const std::string& dataDir);

    const std::string& getDataDirectory() const;

private:
    DataManager();
    
    DataManager(const DataManager&) = delete;

    DataManager& operator=(const DataManager&) = delete;

    std::string dataDirectory_;    // 数据目录
    mutable std::mutex mutex_;     // 互斥锁（mutable表示可以修改）
}; 