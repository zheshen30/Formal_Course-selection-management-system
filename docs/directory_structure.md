# 系统目录结构和使用规范

## 目录结构

```
项目根目录/
│
├── build/                  # 构建输出目录
│   ├── course_system       # 主程序可执行文件
│   └── tests/              # 测试可执行文件目录
│       ├── test_user
│       ├── test_course
│       └── ...
│
├── data/                   # 主程序数据目录（JSON文件等）
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
│
├── test_data/              # 测试数据目录
│   ├── users.json
│   ├── courses.json
│   └── ...
│
└── test_log/               # 测试日志目录
    ├── simple_info.log
    ├── simple_warn.log
    └── simple_error.log
```

## 运行规范

### 主程序

1. **运行位置**：主程序必须放在主目录的`build`文件夹中运行
2. **数据存储**：JSON文件必须全部在主目录下的`data`目录中读写
3. **日志存储**：日志文件必须全部在主目录下的`log`目录中读写

### 测试程序

1. **运行位置**：测试程序必须在主目录的`build/tests`目录中运行
2. **数据存储**：测试用JSON文件必须在主目录下的`test_data`目录中读写
3. **日志存储**：测试日志文件必须在主目录下的`test_log`目录中读写

## 目录设置说明

* 主程序会在启动时自动查找或创建所需的`data`和`log`目录
* 测试程序会在构建时自动创建所需的`test_data`和`test_log`目录
* 目录路径使用相对路径，确保程序可以在不同环境下正确找到数据文件
* 主程序和测试程序的目录分离，避免测试数据影响正式环境

## 注意事项

1. 不要手动删除数据目录或修改系统创建的JSON文件，除非你确切知道你在做什么
2. 所有文件读写操作必须通过`DataManager`类进行，不要直接使用文件API操作
3. 日志记录必须通过`Logger`类进行，不要直接写入日志文件
4. 测试过程中产生的数据不会自动清理，需要手动清理或在测试脚本中处理 