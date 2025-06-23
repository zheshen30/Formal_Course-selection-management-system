# 大学选课管理系统

## 项目概述

大学选课系统是一个面向高校学生、教师和管理员的综合性教务管理平台，主要功能包括用户管理、课程管理、选课管理、数据持久化、多语言支持等。系统采用面向对象设计方法，使用C++语言实现，支持跨平台运行。

## 主要特性

- **多角色支持**：管理员、教师、学生三种角色，权限分离
- **完整选课流程**：课程查询、选课、退课等功能
- **数据持久化**：使用JSON格式存储数据
- **国际化支持**：支持中英文界面
- **安全认证**：使用SHA-256加盐哈希保存密码
- **并发控制**：使用互斥锁保护共享资源
- **日志系统**：分级别记录系统操作和错误信息

## 系统架构

大学选课系统采用分层架构设计，主要分为以下几层：

1. **用户界面层**：负责与用户的交互，显示信息和接收输入
2. **业务逻辑层**：实现系统的核心功能，处理业务规则和流程
3. **数据访问层**：负责数据的持久化和检索

## 核心模块

- **用户管理**：用户创建、认证和信息维护
- **课程管理**：课程创建、修改和查询
- **选课管理**：选课、退课和选课状态查询
- **数据管理**：数据序列化和反序列化
- **国际化**：多语言资源管理
- **日志系统**：事件记录和错误追踪

## 技术栈

- **C++17**：核心编程语言
- **CMake**：构建系统
- **nlohmann/json**：JSON解析库
- **OpenSSL**：加密库（必须）
- **GoogleTest**：单元测试框架（可选）

## 环境要求

- C++17兼容的编译器(GCC 8+/Clang 7+/MSVC 19.14+)
- CMake 3.16或更高版本
- 以下依赖库：
  - nlohmann/json（已包含在源码中）
  - **OpenSSL**（必须安装，用于密码哈希）
  - GoogleTest：单元测试框架（需要运行测试用例时安装）

## 安装OpenSSL

不同系统下安装OpenSSL的方法：

### Linux

```bash
# Debian/Ubuntu
sudo apt-get install libssl-dev

# CentOS/RHEL
sudo yum install openssl-devel
```

### macOS

```bash
brew install openssl
```

### Windows

可通过以下方式获取：

1. 使用vcpkg：`vcpkg install openssl`
2. 直接下载预编译版本：https://slproweb.com/products/Win32OpenSSL.html
3. 使用MSYS2：`pacman -S mingw-w64-x86_64-openssl`

## 构建与运行

### 直接构建

1. 确保已安装OpenSSL库

2. 克隆代码库

   ```
   git clone https://github.com/zheshen30/Formal_Course-selection-management-system.git
   ```

3. 创建构建目录并构建

   ```
   mkdir build && cd build
   cmake ..
   make
   ```

   > 如果OpenSSL安装在非标准位置，可以使用：`cmake -DOPENSSL_ROOT_DIR=/path/to/openssl ..`
   > 如果想要进行单元测试和集成测试，需要在cmake后加-DCMAKE_BUILD_TYPE=Debug选项

4. 运行程序

   ```
   ./course_system
   ```

## 目录结构与使用规范

系统定义了严格的目录结构和使用规范，详细说明请查看[目录结构和使用规范](docs/directory_structure.md)。

主要原则：

- 主程序必须在`build`目录下运行
- 数据文件存储在项目根目录的`data`文件夹
- 日志文件存储在项目根目录的`log`文件夹
- 测试程序在`build/tests`目录下运行

## 数据与日志存储

系统的数据和日志文件统一存储在项目主目录中的指定位置：

- **数据文件**：存储在项目主目录下的`data`目录中
  - 例如：`/path/to/project/data/users.json`

- **日志文件**：存储在项目主目录下的`log`目录中
  - 例如：`/path/to/project/log/simple_info.log`

> **注意**：系统会自动查找并使用项目主目录中的data和log文件夹，不会在build目录中创建或使用数据文件。这样设计是为了避免在清理build目录时意外删除数据。

如果您从build目录运行程序，系统会自动定位到项目根目录的data和log文件夹。如果这些目录不存在，系统会自动创建它们。

- 对于开发和生产环境：无需手动创建log目录，系统将自动处理;**但是主目录下的data目录中的语言数据文件（Chinese.json和English.json)不能删除和更改，用户数据文件中（users.json）预置了三个账户，这三个账户的内容最好不要修改和删除，除非你明确知道自己在做什么，不要修改Chinese.json,English.json和users.json的文件名**

系统预置了以下用户账号用于测试：

| 角色   | 用户ID     | 密码     |
| ------ | ---------- | -------- |
| 管理员 | admin001   | admin    |
| 教师   | teacher001 | password |
| 学生   | student001 | password |

## 文件结构

```
project/
├── CMakeLists.txt          # 项目构建配置
├── build_script.sh         # 构建脚本
├── include/                # 头文件目录
│   ├── model/              # 数据模型
│   ├── manager/            # 管理器类
│   ├── system/             # 系统类
│   └── util/               # 工具类
├── src/                    # 源文件目录
│   ├── model/              # 数据模型实现
│   ├── manager/            # 管理器类实现
│   ├── system/             # 系统类实现
│   ├── util/               # 工具类实现
│   └── main.cpp            # 主函数
├── data/                   # 数据文件目录
│   ├── Chinese.json        # 中文语言文件
│   ├── English.json        # 英文语言文件
│   ├── users.json          # 用户数据
│   ├── courses.json        # 课程数据
│   └── enrollment.json     # 选课数据
├── log/                    # 日志文件目录
│   ├── info.log            # 信息日志
│   ├── warn.log            # 警告日志
│   └── error.log           # 错误日志
├── tests/                  # 测试目录
│   ├── unit/               # 单元测试
│   └── integration/        # 集成测试
└── docs/                   # 文档目录
    ├── api.md              # API文档
    ├── view_class.md       # 类视图文档
    └── system_arch.md      # 系统架构文档
```

## 文档

- [系统架构](docs/system_arch.md)：详细设计文档
- [需求分析](docs/require.md)：系统需求分析
- [API文档](docs/api.md)：API接口文档
- [类视图](docs/view_class.md)：类图和关系

## 开发者

- 哲神

## 许可证

本项目采用GNU通用公共许可证第3版(GPLv3)

Copyright (C) 2025 哲神

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.