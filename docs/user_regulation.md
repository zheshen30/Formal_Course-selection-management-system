# 使用规范

## 目录结构

```
项目根目录/
│
├── build/                  # 构建输出目录
│   ├── course_system       # 主程序可执行文件
|   ├── test_data/          # 测试数据目录
│   |   ├── users.json
│   |   ├── courses.json    
│   |   └── ...    
│   └── test_log/           # 测试日志目录
|   |   ├── simple_info.log
|   |   ├── simple_warn.log
|   |   └── simple_error.log
|   └── tests/              # 测试可执行文件目录
│       ├── test_user
│       ├── test_course
│       └── ...
│
├── data/                   # 主程序数据目录（JSON文件）
│   ├── users.json
│   ├── courses.json
│   ├── enrollment.json
│   ├── Chinese.json
│   └── English.json
│
├── log/                    # 主程序日志目录
│   ├── simple_info.log
│   ├── simple_warn.log
│   └── simple_error.log
└── ...

```
 完整目录详见系统架构文档[系统架构文档规范](system_arch.md)。
 docs目录下的system_arch.md文件

## 运行规范

### 主程序

1. **运行位置**：主程序必须放在主目录的`build`文件夹中运行
2. **数据存储**：JSON文件必须全部在主目录下的`data`目录中读写
3. **日志存储**：日志文件必须全部在主目录下的`log`目录中读写

### 测试程序

1. **运行位置**：测试程序必须在主目录的`build/tests`目录中运行
2. **数据存储**：测试用JSON文件必须在build目录下的`test_data`目录中读写
3. **日志存储**：测试日志文件必须在build目录下的`test_log`目录中读写


## 注意事项

**不要手动删除或修改系统创建的JSON文件和文件名，除非你确切知道你在做什么**
