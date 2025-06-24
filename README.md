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


## 环境要求

**本项目的开发环境：Rocky Linux 9.5（5.14.0-503.14.1.el9_5.x86_64内核版本）**

- C++17兼容的编译器(GCC 8+/Clang 7+/MSVC 19.14+)
- CMake 3.16或更高版本
- 以下依赖库：
  - **OpenSSL**（必须安装，用于加密）
  - GoogleTest （单元测试框架）（需要运行测试用例时安装）（选装）
  - nlohmann/json（已包含在源码中）(使用单头文件的json解析库，通过预编译头文件减少编译时间)


## 安装OpenSSL（必须安装）

**Linux下安装OpenSSL的方法：**

```bash
# Debian/Ubuntu
sudo apt-get install libssl-dev

# CentOS/RHEL
sudo yum install openssl-devel
```

**macOS和Windows下的OpenSSL的安装方法请自行搜索**

## 安装GoogleTest（选装）

 **运行测试用例时，必装**

 安装方法请自行搜索，推荐使用github上的源码进行安装
 https://github.com/google/googletest.git

## 构建与运行

### 直接构建

1. 确保已正确安装OpenSSL库

2. 克隆代码库或通过压缩包获得完整源码

   ```
   git clone https://github.com/zheshen30/Formal_Course-selection-management-system.git
   ```

3.  **必须先创建构建目录build，然后进入build目录使用cmake编译**

   ```
   mkdir build && cd build
   cmake ..
   make
   ```

   > 如果OpenSSL安装在非标准位置，可以使用：`cmake -DOPENSSL_ROOT_DIR=/path/to/openssl ..`
   > 如果要进行单元测试和集成测试，需要在cmake后加 `-DCMAKE_BUILD_TYPE=Debug`选项 **先确保正确安装了GoogleTest**

4. 运行程序(build目录下./course_system)

   **请完整阅读使用规范文档**[使用规范](docs/user_regulation.md)
   docs目录下的user_regulation.md文件
   
   **系统定义了严格的目录结构和使用规范,不遵守使用规范程序可能无法正常运行，或者导致未定义行为**

## 数据与日志存储

系统的数据和日志文件统一存储在项目中的指定位置：

 详见使用规范文档[使用规范](docs/user_regulation.md)
 docs目录下的user_regulation.md文件

 **请通过主程序进行操作，所有直接修改主目录的data目录下的json文件的行为是未定义的！！！**
 **例如添加用户操作：使用主程序管理员用户下的添加用户功能，而不是直接在主目录的data目录下的json文件中添加数据**

系统预置了以下用户账号（在主目录的data目录下的user.json文件)：

| 角色   | 用户ID     | 密码     |
| ------ | ---------- | -------- |
| 管理员 | admin001   | admin    |
| 教师   | teacher001 | password |
| 学生   | student001 | password |

 **以上三个特殊账户的密码一经修改，密码的加密方式由纯哈希转化为哈希加盐值**
 **所有用户的密码都不支持找回功能**

## 文件结构

 详见系统架构文档[系统架构文档](docs/system_arch.md)
 docs目录下的system_arch.md文件

## 文档（docs目录下）

- [系统架构system_arch](docs/system_arch.md)：详细设计文档
- [需求分析require](docs/require.md)：系统需求分析
- [API文档api](docs/api.md)：API接口文档
- [类视图view_class](docs/view_class.md)：类图和关系
- [使用规范user_regulation](docs/user_regulation.md)：使用规范文档

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