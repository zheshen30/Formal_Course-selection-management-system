#!/bin/bash

# 定义许可证头内容
LICENSE_HEADER=$(cat <<'EOF'
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

EOF
)

# 为所有C/C++头文件和源文件添加许可证头
find include src tests -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.cc" \) | while read -r file; do
    # 检查文件是否已包含许可证头
    if ! grep -q "Copyright (C) 2025 哲神" "$file"; then
        echo "Adding license header to $file"
        temp_file=$(mktemp)
        echo "$LICENSE_HEADER" > "$temp_file"
        cat "$file" >> "$temp_file"
        mv "$temp_file" "$file"
    else
        echo "License header already exists in $file"
    fi
done

echo "完成！许可证头已添加到所有源文件。" 