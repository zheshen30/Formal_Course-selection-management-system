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
#include <functional>
#include <vector>
#include <limits>

class InputValidator {
public:
    static bool validateInteger(const std::string& input, int min, int max, int& result);
    
    static bool validateDouble(const std::string& input, double min, double max, double& result);

    static bool validateChoice(const std::string& input, int min, int max, int& result);

    static bool isEmptyInput(const std::string& input);
}; 