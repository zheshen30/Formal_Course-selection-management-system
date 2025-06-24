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

 **build目录由用户创建，build的子目录编译后创建，build子目录下的test_data,test_log在测试程序运行时自动创建，log目录在主程序运行时自动创建，data目录原本就有**

 完整目录详见系统架构文档[系统架构文档](system_arch.md)。
 docs目录下的system_arch.md文件

## 运行规范

### 主程序course_system

 **运行主程序前，请确保主目录下的data目录中，有正确的数据文件**

1. **运行位置**：主程序必须放在主目录的`build`文件夹中运行
2. **数据存储**：JSON文件必须全部在主目录下的`data`目录中读写
3. **日志存储**：日志文件必须全部在主目录下的`log`目录中读写

### 测试程序（在主目录的build/tests)（测试程序的功能详见系统架构文档）
 
 **运行测试程序前，请确保build目录下的test_data和test_log目录为空或者不存在这两个目录**

1. **运行位置**：测试程序必须在主目录的`build/tests`目录中运行
2. **数据存储**：测试用JSON文件必须在build目录下的`test_data`目录中读写
3. **日志存储**：测试日志文件必须在build目录下的`test_log`目录中读写

## 注意事项

 **请通过主程序进行操作，所有直接修改主目录的data目录下的json文件的行为是未定义的！！！**
 **例如添加用户操作：使用主程序管理员用户下的添加用户功能，而不是直接在主目录的data目录下的json文件中添加数据**
 
 **但是在每次运行测试程序后，请先删除build目录下的test_data目录和test_log目录（如果存在），避免上一次测试的残留数据对这一次测试造成影响，有时候测试失败，是由上一次测试的残留数据导致的**
