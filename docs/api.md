# 大学选课系统API文档

## 目录

- [用户管理API](#用户管理API)
- [课程管理API](#课程管理API)
- [选课管理API](#选课管理API)
- [系统管理API](#系统管理API)
- [数据管理API](#数据管理API)
- [国际化支持API](#国际化支持API)
- [审计日志API](#审计日志API)
- [用户管理器API](#用户管理器API)
- [课程管理器API](#课程管理器API)
- [选课管理器API](#选课管理器API)
- [线程安全工具API](#线程安全工具API)
- [异常处理API](#异常处理API)
- [统一错误处理](#统一错误处理)

---

## 用户管理API

### User类

#### 构造函数

```cpp
User(const std::string& userId, const std::string& password, const std::string& name, const std::string& contactInfo);
```

- **描述**: 创建基本用户对象
- **参数**:
  - `userId`: 用户ID，系统唯一标识符
  - `password`: 用户密码（将被哈希存储）
  - `name`: 用户姓名
  - `contactInfo`: 用户联系信息（通常为邮箱）
- **返回**: 不适用

#### 用户认证

```cpp
bool authenticate(const std::string& password) const;
```

- **描述**: 验证用户密码
- **参数**:
  - `password`: 待验证的密码
- **返回**: 如果密码正确，返回`true`，否则返回`false`

#### 获取用户ID

```cpp
std::string getUserId() const;
```

- **描述**: 获取用户ID
- **参数**: 无
- **返回**: 用户ID字符串

#### 获取用户类型

```cpp
User::UserType getUserType() const;
```

- **描述**: 获取用户类型
- **参数**: 无
- **返回**: 用户类型枚举值（STUDENT, TEACHER, ADMIN）

### Student类

#### 选课

```cpp
bool enrollCourse(const std::string& courseId);
```

- **描述**: 学生选课操作
- **参数**:
  - `courseId`: 课程ID
- **返回**: 选课成功返回`true`，否则返回`false`

#### 退课

```cpp
bool dropCourse(const std::string& courseId);
```

- **描述**: 学生退课操作
- **参数**:
  - `courseId`: 课程ID
- **返回**: 退课成功返回`true`，否则返回`false`

#### 检查是否已选课

```cpp
bool isEnrolledIn(const std::string& courseId) const;
```

- **描述**: 检查学生是否已选某课程
- **参数**:
  - `courseId`: 课程ID
- **返回**: 已选返回`true`，否则返回`false`

### Teacher类

#### 添加教授课程

```cpp
bool addCourse(const std::string& courseId);
```

- **描述**: 添加教师教授的课程
- **参数**:
  - `courseId`: 课程ID
- **返回**: 添加成功返回`true`，否则返回`false`

#### 检查是否教授课程

```cpp
bool isTeaching(const std::string& courseId) const;
```

- **描述**: 检查教师是否教授某课程
- **参数**:
  - `courseId`: 课程ID
- **返回**: 教授返回`true`，否则返回`false`

---

## 课程管理API

### Course类

#### 构造函数

```cpp
Course(const std::string& courseId, const std::string& courseName, const std::string& courseType,
       int totalHours, float credits, const std::string& semester, int capacity);
```

- **描述**: 创建课程对象
- **参数**:
  - `courseId`: 课程ID，系统唯一标识符
  - `courseName`: 课程名称
  - `courseType`: 课程类型（如"必修"、"选修"）
  - `totalHours`: 课程总学时
  - `credits`: 课程学分
  - `semester`: 开课学期
  - `capacity`: 课程容量（最大选课人数）
- **返回**: 不适用

#### 添加学生

```cpp
bool addStudent(const std::string& studentId);
```

- **描述**: 将学生添加到课程
- **参数**:
  - `studentId`: 学生ID
- **返回**: 添加成功返回`true`，否则返回`false`

#### 移除学生

```cpp
bool removeStudent(const std::string& studentId);
```

- **描述**: 将学生从课程中移除
- **参数**:
  - `studentId`: 学生ID
- **返回**: 移除成功返回`true`，否则返回`false`

#### 检查是否已满

```cpp
bool isFull() const;
```

- **描述**: 检查课程是否已满
- **参数**: 无
- **返回**: 已满返回`true`，否则返回`false`

#### 检查学生是否已选课

```cpp
bool isStudentEnrolled(const std::string& studentId) const;
```

- **描述**: 检查指定学生是否已选此课程
- **参数**:
  - `studentId`: 学生ID
- **返回**: 已选返回`true`，否则返回`false`

---

## 选课管理API

### CourseSystem类（与选课相关的方法）

#### 学生选课

```cpp
bool enrollCourse(const std::string& studentId, const std::string& courseId);
```

- **描述**: 为学生选课
- **参数**:
  - `studentId`: 学生ID
  - `courseId`: 课程ID
- **返回**: 选课成功返回`true`，否则返回`false`

#### 学生退课

```cpp
bool dropCourse(const std::string& studentId, const std::string& courseId);
```

- **描述**: 学生退课操作
- **参数**:
  - `studentId`: 学生ID
  - `courseId`: 课程ID
- **返回**: 退课成功返回`true`，否则返回`false`

#### 获取可选课程

```cpp
std::vector<std::shared_ptr<Course>> getAvailableCourses() const;
```

- **描述**: 获取所有未满的课程
- **参数**: 无
- **返回**: 可选课程列表

#### 获取学生课程

```cpp
std::vector<std::shared_ptr<Course>> getCoursesByStudent(const std::string& studentId) const;
```

- **描述**: 获取学生已选的课程列表
- **参数**:
  - `studentId`: 学生ID
- **返回**: 学生已选课程列表

#### 获取教师课程

```cpp
std::vector<std::shared_ptr<Course>> getCoursesByTeacher(const std::string& teacherId) const;
```

- **描述**: 获取教师教授的课程列表
- **参数**:
  - `teacherId`: 教师ID
- **返回**: 教师教授的课程列表

#### 获取课程学生名单

```cpp
std::vector<std::shared_ptr<Student>> getStudentsByCourse(const std::string& courseId) const;
```

- **描述**: 获取选修指定课程的学生列表
- **参数**:
  - `courseId`: 课程ID
- **返回**: 选修该课程的学生列表

---

## 系统管理API

### CourseSystem类（系统管理方法）

#### 初始化系统

```cpp
CourseSystem();
```

- **描述**: 初始化课程系统，加载数据和配置
- **参数**: 无
- **返回**: 不适用

#### 登录

```cpp
bool login(const std::string& userId, const std::string& password);
```

- **描述**: 用户登录
- **参数**:
  - `userId`: 用户ID
  - `password`: 密码
- **返回**: 登录成功返回`true`，否则返回`false`

#### 注销

```cpp
void logout();
```

- **描述**: 用户注销
- **参数**: 无
- **返回**: 无

#### 添加课程

```cpp
bool addCourse(const std::string& courseId, const std::string& courseName,
               const std::string& teacherId, const std::string& courseType,
               int totalHours, float credits, const std::string& semester, int capacity);
```

- **描述**: 添加新课程到系统
- **参数**:
  - `courseId`: 课程ID
  - `courseName`: 课程名称
  - `teacherId`: 教师ID
  - `courseType`: 课程类型
  - `totalHours`: 总学时
  - `credits`: 学分
  - `semester`: 学期
  - `capacity`: 容量
- **返回**: 添加成功返回`true`，否则返回`false`

#### 添加学生

```cpp
bool addStudent(const std::string& userId, const std::string& password,
               const std::string& name, const std::string& contactInfo,
               const std::string& gender, int age,
               const std::string& department, const std::string& className);
```

- **描述**: 添加新学生到系统
- **参数**:
  - `userId`: 用户ID
  - `password`: 密码
  - `name`: 姓名
  - `contactInfo`: 联系信息
  - `gender`: 性别
  - `age`: 年龄
  - `department`: 院系
  - `className`: 班级
- **返回**: 添加成功返回`true`，否则返回`false`

#### 添加教师

```cpp
bool addTeacher(const std::string& userId, const std::string& password,
               const std::string& name, const std::string& contactInfo,
               const std::string& department, const std::string& title);
```

- **描述**: 添加新教师到系统
- **参数**:
  - `userId`: 用户ID
  - `password`: 密码
  - `name`: 姓名
  - `contactInfo`: 联系信息
  - `department`: 院系
  - `title`: 职称
- **返回**: 添加成功返回`true`，否则返回`false`

#### 添加管理员

```cpp
bool addAdmin(const std::string& userId, const std::string& password,
             const std::string& name, const std::string& contactInfo,
             const std::string& role, int permissionLevel);
```

- **描述**: 添加新管理员到系统
- **参数**:
  - `userId`: 用户ID
  - `password`: 密码
  - `name`: 姓名
  - `contactInfo`: 联系信息
  - `role`: 角色
  - `permissionLevel`: 权限级别
- **返回**: 添加成功返回`true`，否则返回`false`

---

## 数据管理API

### DataManager类

#### 保存用户数据

```cpp
static bool saveUsers(const std::unordered_map<std::string, std::shared_ptr<User>>& users,
                      const std::string& filePath = "data/users.json");
```

- **描述**: 将用户数据保存到文件
- **参数**:
  - `users`: 用户数据映射
  - `filePath`: 文件路径
- **返回**: 保存成功返回`true`，否则返回`false`

#### 保存课程数据

```cpp
static bool saveCourses(const std::unordered_map<std::string, std::shared_ptr<Course>>& courses,
                       const std::string& filePath = "data/courses.json");
```

- **描述**: 将课程数据保存到文件
- **参数**:
  - `courses`: 课程数据映射
  - `filePath`: 文件路径
- **返回**: 保存成功返回`true`，否则返回`false`

#### 保存选课数据

```cpp
static bool saveEnrollments(const std::vector<Enrollment>& enrollments,
                          const std::string& filePath = "data/enrollments.json");
```

- **描述**: 将选课数据保存到文件
- **参数**:
  - `enrollments`: 选课记录列表
  - `filePath`: 文件路径
- **返回**: 保存成功返回`true`，否则返回`false`

#### 加载用户数据

```cpp
static bool loadUsers(std::unordered_map<std::string, std::shared_ptr<User>>& users,
                     const std::string& filePath = "data/users.json");
```

- **描述**: 从文件加载用户数据
- **参数**:
  - `users`: [输出]用户数据映射
  - `filePath`: 文件路径
- **返回**: 加载成功返回`true`，否则返回`false`

#### 备份数据

```cpp
static bool backupData(const std::string& directory = "backup");
```

- **描述**: 备份系统数据
- **参数**:
  - `directory`: 备份目录
- **返回**: 备份成功返回`true`，否则返回`false`

---

## 国际化支持API

### I18nManager类

#### 获取单例实例

```cpp
static std::shared_ptr<I18nManager> getInstance();
```

- **描述**: 获取I18nManager的单例实例
- **参数**: 无
- **返回**: I18nManager实例的共享指针

#### 设置语言区域

```cpp
bool setLocale(const std::string& locale);
```

- **描述**: 设置当前使用的语言
- **参数**:
  - `locale`: 语言代码（如"en_US", "zh_CN"）
- **返回**: 设置成功返回`true`，否则返回`false`

#### 获取当前语言

```cpp
std::string getCurrentLocale() const;
```

- **描述**: 获取当前设置的语言
- **参数**: 无
- **返回**: 当前语言代码

#### 获取文本翻译

```cpp
std::string getText(const std::string& key, const std::string& defaultText = "") const;
```

- **描述**: 获取指定键的本地化文本
- **参数**:
  - `key`: 文本键
  - `defaultText`: 默认文本（如果找不到翻译）
- **返回**: 本地化文本

#### 从文件加载语言资源

```cpp
bool loadFromFile(const std::string& locale, const std::string& filePath);
```

- **描述**: 从JSON文件加载语言资源
- **参数**:
  - `locale`: 语言代码
  - `filePath`: 资源文件路径
- **返回**: 加载成功返回`true`，否则返回`false`

---

## 审计日志API

### AuditLog类

#### 获取单例实例

```cpp
static std::shared_ptr<AuditLog> getInstance();
```

- **描述**: 获取AuditLog的单例实例
- **参数**: 无
- **返回**: AuditLog实例的共享指针

#### 记录操作日志

```cpp
void log(const std::string& userId, const std::string& action,
         const std::string& targetId, const std::string& targetType,
         const std::string& details, const std::string& status,
         const std::string& ipAddress = "");
```

- **描述**: 记录一条审计日志
- **参数**:
  - `userId`: 执行操作的用户ID
  - `action`: 操作类型（如LOGIN, LOGOUT, ENROLL等）
  - `targetId`: 操作目标ID（如被创建的用户ID）
  - `targetType`: 目标类型（如User, Course等）
  - `details`: 操作详情
  - `status`: 操作状态（SUCCESS或FAILED）
  - `ipAddress`: IP地址（可选）
- **返回**: 无

#### 查询审计日志

```cpp
std::vector<AuditRecord> query(const std::string& userId = "",
                              const std::string& startTime = "",
                              const std::string& endTime = "",
                              const std::string& action = "",
                              int limit = 100) const;
```

- **描述**: 根据条件查询审计日志
- **参数**:
  - `userId`: 用户ID过滤（可选）
  - `startTime`: 开始时间过滤（可选）
  - `endTime`: 结束时间过滤（可选）
  - `action`: 操作类型过滤（可选）
  - `limit`: 结果数量限制
- **返回**: 符合条件的审计记录列表

#### 导出审计日志

```cpp
bool exportToFile(const std::string& filePath) const;
```

- **描述**: 将审计日志导出到文件
- **参数**:
  - `filePath`: 导出文件路径
- **返回**: 导出成功返回`true`，否则返回`false`

#### 清理过期审计日志

```cpp
int purgeOldRecords(int daysToKeep = 365);
```

- **描述**: 清理指定天数之前的审计日志
- **参数**:
  - `daysToKeep`: 保留的天数
- **返回**: 清理的记录数量

## 用户管理器API

### UserManager类

#### 构造函数

```cpp
UserManager();
```

- **描述**: 创建用户管理器实例
- **参数**: 无
- **返回**: 不适用

#### 添加学生

```cpp
bool addStudent(const std::string& userId, const std::string& password,
               const std::string& name, const std::string& contactInfo,
               const std::string& gender, int age,
               const std::string& department, const std::string& className);
```

- **描述**: 向系统添加学生用户
- **参数**:
  - `userId`: 用户ID
  - `password`: 密码
  - `name`: 姓名
  - `contactInfo`: 联系信息
  - `gender`: 性别
  - `age`: 年龄
  - `department`: 院系
  - `className`: 班级
- **返回**: 添加成功返回`true`，否则返回`false`

#### 更新学生信息

```cpp
bool updateStudent(const std::string& userId, 
                  const std::optional<std::string>& name = std::nullopt,
                  const std::optional<std::string>& contactInfo = std::nullopt,
                  const std::optional<std::string>& gender = std::nullopt,
                  const std::optional<int>& age = std::nullopt,
                  const std::optional<std::string>& department = std::nullopt,
                  const std::optional<std::string>& className = std::nullopt);
```

- **描述**: 更新学生信息
- **参数**:
  - `userId`: 学生ID
  - `name`: 姓名（可选）
  - `contactInfo`: 联系信息（可选）
  - `gender`: 性别（可选）
  - `age`: 年龄（可选）
  - `department`: 院系（可选）
  - `className`: 班级（可选）
- **返回**: 更新成功返回`true`，否则返回`false`

#### 添加教师

```cpp
bool addTeacher(const std::string& userId, const std::string& password,
               const std::string& name, const std::string& contactInfo,
               const std::string& department, const std::string& title);
```

- **描述**: 向系统添加教师用户
- **参数**:
  - `userId`: 用户ID
  - `password`: 密码
  - `name`: 姓名
  - `contactInfo`: 联系信息
  - `department`: 院系
  - `title`: 职称
- **返回**: 添加成功返回`true`，否则返回`false`

#### 查找用户

```cpp
User* findUserById(const std::string& userId) const;
```

- **描述**: 根据ID查找用户
- **参数**:
  - `userId`: 用户ID
- **返回**: 返回用户指针，如果用户不存在则返回`nullptr`

#### 保存用户数据

```cpp
bool saveUsers(const std::string& filePath = "data/users.json") const;
```

- **描述**: 将用户数据保存到指定文件
- **参数**:
  - `filePath`: 文件路径
- **返回**: 保存成功返回`true`，否则返回`false`

## 课程管理器API

### CourseManager类

#### 构造函数

```cpp
CourseManager();
```

- **描述**: 创建课程管理器实例
- **参数**: 无
- **返回**: 不适用

#### 添加课程

```cpp
bool addCourse(const std::string& courseId, const std::string& courseName,
              const std::string& courseType, int totalHours, float credits,
              const std::string& semester, int capacity, const std::string& teacherId = "");
```

- **描述**: 向系统添加课程
- **参数**:
  - `courseId`: 课程ID
  - `courseName`: 课程名称
  - `courseType`: 课程类型
  - `totalHours`: 总学时
  - `credits`: 学分
  - `semester`: 学期
  - `capacity`: 容量
  - `teacherId`: 教师ID（可选）
- **返回**: 添加成功返回`true`，否则返回`false`

#### 更新课程信息

```cpp
bool updateCourse(const std::string& courseId,
                 const std::optional<std::string>& courseName = std::nullopt,
                 const std::optional<std::string>& courseType = std::nullopt,
                 const std::optional<int>& totalHours = std::nullopt,
                 const std::optional<float>& credits = std::nullopt,
                 const std::optional<std::string>& semester = std::nullopt,
                 const std::optional<int>& capacity = std::nullopt,
                 const std::optional<std::string>& teacherId = std::nullopt);
```

- **描述**: 更新课程信息
- **参数**:
  - `courseId`: 课程ID
  - `courseName`: 课程名称（可选）
  - `courseType`: 课程类型（可选）
  - `totalHours`: 总学时（可选）
  - `credits`: 学分（可选）
  - `semester`: 学期（可选）
  - `capacity`: 容量（可选）
  - `teacherId`: 教师ID（可选）
- **返回**: 更新成功返回`true`，否则返回`false`

#### 删除课程

```cpp
bool deleteCourse(const std::string& courseId);
```

- **描述**: 从系统中删除课程
- **参数**:
  - `courseId`: 课程ID
- **返回**: 删除成功返回`true`，否则返回`false`

#### 查找课程

```cpp
Course* findCourseById(const std::string& courseId) const;
```

- **描述**: 根据ID查找课程
- **参数**:
  - `courseId`: 课程ID
- **返回**: 返回课程指针，如果课程不存在则返回`nullptr`

## 选课管理器API

### EnrollmentManager类

#### 构造函数

```cpp
EnrollmentManager();
```

- **描述**: 创建选课管理器实例
- **参数**: 无
- **返回**: 不适用

#### 学生选课

```cpp
bool enrollStudent(const std::string& studentId, const std::string& courseId, 
                  UserManager& userManager, CourseManager& courseManager);
```

- **描述**: 为学生选课
- **参数**:
  - `studentId`: 学生ID
  - `courseId`: 课程ID
  - `userManager`: 用户管理器引用
  - `courseManager`: 课程管理器引用
- **返回**: 选课成功返回`true`，否则返回`false`

#### 学生退课

```cpp
bool dropCourse(const std::string& studentId, const std::string& courseId,
               UserManager& userManager, CourseManager& courseManager);
```

- **描述**: 学生退课
- **参数**:
  - `studentId`: 学生ID
  - `courseId`: 课程ID
  - `userManager`: 用户管理器引用
  - `courseManager`: 课程管理器引用
- **返回**: 退课成功返回`true`，否则返回`false`

#### 设置学生成绩

```cpp
bool setGrade(const std::string& studentId, const std::string& courseId, 
             float grade, UserManager& userManager, CourseManager& courseManager);
```

- **描述**: 设置学生在某课程的成绩
- **参数**:
  - `studentId`: 学生ID
  - `courseId`: 课程ID
  - `grade`: 成绩
  - `userManager`: 用户管理器引用
  - `courseManager`: 课程管理器引用
- **返回**: 设置成功返回`true`，否则返回`false`

#### 查询学生选课

```cpp
std::vector<Enrollment> getEnrollmentsByStudent(const std::string& studentId) const;
```

- **描述**: 获取指定学生的所有选课记录
- **参数**:
  - `studentId`: 学生ID
- **返回**: 选课记录列表

## 线程安全工具API

### LockGuard类

#### 构造函数（互斥锁）

```cpp
LockGuard(std::mutex& mutex, 
          unsigned int timeoutMs = 0, 
          const char* file = __FILE__, 
          int line = __LINE__);
```

- **描述**: 创建互斥锁的RAII封装
- **参数**:
  - `mutex`: 要锁定的互斥锁
  - `timeoutMs`: 锁定超时时间（毫秒），0表示永不超时
  - `file`: 源文件名（用于调试）
  - `line`: 行号（用于调试）
- **返回**: 不适用
- **异常**: 如果超时，抛出`std::runtime_error`

#### 构造函数（共享互斥锁）

```cpp
LockGuard(std::shared_mutex& mutex,
          Mode mode = Mode::EXCLUSIVE,
          unsigned int timeoutMs = 0,
          const char* file = __FILE__,
          int line = __LINE__);
```

- **描述**: 创建共享互斥锁的RAII封装，支持读写锁
- **参数**:
  - `mutex`: 要锁定的共享互斥锁
  - `mode`: 锁定模式（EXCLUSIVE或SHARED）
  - `timeoutMs`: 锁定超时时间（毫秒），0表示永不超时
  - `file`: 源文件名（用于调试）
  - `line`: 行号（用于调试）
- **返回**: 不适用
- **异常**: 如果超时，抛出`std::runtime_error`

#### 手动解锁

```cpp
void unlock();
```

- **描述**: 手动解锁
- **参数**: 无
- **返回**: 无

## 异常处理API

### SystemException类

#### 构造函数

```cpp
SystemException(const std::string& message, ErrorType errorType, int errorCode = 0);
```

- **描述**: 创建系统异常对象
- **参数**:
  - `message`: 错误消息
  - `errorType`: 错误类型
  - `errorCode`: 错误代码（可选）
- **返回**: 不适用

#### 带文件和行号的构造函数

```cpp
SystemException(const std::string& message, ErrorType errorType, 
              const std::string& file, int line, int errorCode = 0);
```

- **描述**: 创建带有源码位置信息的系统异常对象
- **参数**:
  - `message`: 错误消息
  - `errorType`: 错误类型
  - `file`: 源文件名
  - `line`: 行号
  - `errorCode`: 错误代码（可选）
- **返回**: 不适用

#### 获取完整错误信息

```cpp
std::string getFullMessage() const;
```

- **描述**: 获取包含所有上下文的完整错误信息
- **参数**: 无
- **返回**: 格式化的错误信息字符串

## 统一错误处理

本系统采用统一的错误处理机制，主要分为以下几种方式：

### 1. 返回值错误处理

适用于非关键性操作或可恢复的错误情况：

```cpp
bool addStudent(const std::string& userId, const std::string& password, const std::string& name, 
               const std::string& contactInfo, const std::string& gender, int age, 
               const std::string& department, const std::string& className);
```

- **成功**：返回`true`
- **失败**：返回`false`
- **使用场景**：用户输入验证失败、对象已存在、权限不足等可预期的错误

### 2. 异常处理

适用于严重错误或不可恢复的错误情况：

```cpp
void enrollStudent(const std::string& studentId, const std::string& courseId) 
    throw(SystemException);
```

- **成功**：正常返回
- **失败**：抛出`SystemException`异常，包含以下信息：
  - 错误类型（`ErrorType`）
  - 错误消息
  - 错误代码（可选）
  - 错误位置（文件名和行号，可选）
- **使用场景**：
  - 数据验证严重错误（如非法ID格式、负数年龄等）
  - 文件系统错误（如文件不存在、权限不足）
  - 网络错误
  - 并发冲突
  - 其他严重的系统错误

### 3. 空指针返回

适用于查询操作：

```cpp
User* findUserById(const std::string& userId) const;
```

- **成功**：返回有效指针
- **失败**：返回`nullptr`
- **使用场景**：查找不存在的对象

### 4. 空容器返回

适用于列表查询操作：

```cpp
std::vector<User*> searchUsersByName(const std::string& name) const;
```

- **成功**：返回包含结果的容器
- **失败**：返回空容器
- **使用场景**：没有符合条件的结果

### 5. 可选值返回

适用于可能不存在的单个值：

```cpp
std::optional<float> getGrade() const;
```

- **成功**：返回包含值的`std::optional`
- **失败**：返回空的`std::optional`
- **使用场景**：可能不存在的属性值

### 错误类型映射

各API方法在出现错误时，会使用以下错误类型：

| 错误情况       | 返回值方法             | 异常方法（ErrorType）     |
| -------------- | ---------------------- | ------------------------- |
| 用户ID不存在   | 返回`false`或`nullptr` | `DATA_NOT_FOUND`          |
| 用户ID已存在   | 返回`false`            | `DATA_ALREADY_EXISTS`     |
| 课程ID不存在   | 返回`false`或`nullptr` | `DATA_NOT_FOUND`          |
| 课程ID已存在   | 返回`false`            | `DATA_ALREADY_EXISTS`     |
| 课程已满       | 返回`false`            | `COURSE_FULL`             |
| 学生已选该课程 | 返回`false`            | `ALREADY_ENROLLED`        |
| 学生未选该课程 | 返回`false`            | `NOT_ENROLLED`            |
| 输入参数无效   | 返回`false`            | `DATA_INVALID`            |
| 权限不足       | 返回`false`            | `PERMISSION_DENIED`       |
| 用户验证失败   | 返回`false`            | `AUTHENTICATION_FAILED`   |
| 文件不存在     | 返回`false`            | `FILE_NOT_FOUND`          |
| 文件访问被拒绝 | 返回`false`            | `FILE_ACCESS_DENIED`      |
| 锁定超时       | -                      | `LOCK_TIMEOUT`            |
| 并发修改冲突   | -                      | `CONCURRENT_MODIFICATION` |

### 使用建议

1. **选择合适的错误处理方式**：
   - 对于可恢复的错误，使用返回值表示成功/失败
   - 对于不可恢复的错误或程序逻辑错误，使用异常

2. **异常捕获层次**：
   - 在适当的层次捕获异常，通常是在调用栈的上层
   - 底层方法抛出具体异常，上层捕获并决定如何处理

3. **错误信息详细度**：
   - 确保错误消息包含足够信息来定位问题
   - 对于用户可见的错误，使用适合的语言和描述

4. **资源清理**：
   - 使用RAII模式（如`LockGuard`）确保资源正确释放
   - 在异常路径中也要确保资源释放

5. **事务完整性**：
   - 在可能抛出异常的操作中，确保系统状态保持一致
   - 使用事务或回滚机制确保操作的原子性 