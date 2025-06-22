# 大学选课系统 API 文档

本文档详细描述了大学选课系统的主要API接口。

## 用户管理 API

### UserManager 类

单例类，负责用户的管理和认证。

#### 获取单例

```cpp
static UserManager& getInstance();
```

返回UserManager的单例实例。

#### 用户添加

```cpp
bool addStudent(std::unique_ptr<Student> student);
bool addTeacher(std::unique_ptr<Teacher> teacher);
bool addAdmin(std::unique_ptr<Admin> admin);
```

添加不同类型的用户，成功返回true，失败返回false。

#### 用户移除

```cpp
bool removeUser(const std::string& userId);
```

根据用户ID移除用户，成功返回true，失败返回false。

#### 用户获取

```cpp
User* getUser(const std::string& userId);
Student* getStudent(const std::string& studentId);
Teacher* getTeacher(const std::string& teacherId);
Admin* getAdmin(const std::string& adminId);
```

根据ID获取不同类型的用户，不存在则返回nullptr。

#### 用户认证

```cpp
User* authenticate(const std::string& userId, const std::string& password);
```

验证用户身份，认证成功返回用户指针，失败返回nullptr。

#### 密码管理

```cpp
bool changeUserPassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword);
```

修改用户密码，需要提供正确的旧密码，修改成功返回true，失败返回false。密码将使用盐值加密存储。

#### 用户查询

```cpp
std::vector<std::string> getAllStudentIds() const;
std::vector<std::string> getAllTeacherIds() const;
std::vector<std::string> getAllAdminIds() const;
std::vector<std::string> findUsers(const std::function<bool(const User&)>& predicate) const;
```

获取不同类型的用户ID列表或根据条件查找用户。

#### 数据持久化

```cpp
bool loadData();
bool saveData();
```

从文件加载用户数据或保存用户数据，成功返回true，失败返回false。

## 课程管理 API

### CourseManager 类

单例类，负责课程的管理和查询。

#### 获取单例

```cpp
static CourseManager& getInstance();
```

返回CourseManager的单例实例。

#### 课程操作

```cpp
bool addCourse(std::unique_ptr<Course> course);
bool removeCourse(const std::string& courseId);
Course* getCourse(const std::string& courseId);
bool updateCourseInfo(const Course& course);
```

添加、移除、获取和更新课程，操作成功返回true，失败返回false。

#### 课程查询

```cpp
std::vector<std::string> getAllCourseIds() const;
std::vector<std::string> getTeacherCourseIds(const std::string& teacherId) const;
std::vector<std::string> getStudentEnrolledCourseIds(const std::string& studentId) const;
std::vector<std::string> findCourses(const std::function<bool(const Course&)>& predicate) const;
```

获取课程ID列表或根据条件查找课程。

#### 数据持久化

```cpp
bool loadData();
bool saveData();
```

从文件加载课程数据或保存课程数据，成功返回true，失败返回false。

## 选课管理 API

### EnrollmentManager 类

单例类，负责选课关系的管理和查询。

#### 获取单例

```cpp
static EnrollmentManager& getInstance();
```

返回EnrollmentManager的单例实例。

#### 选课操作

```cpp
bool enrollCourse(const std::string& studentId, const std::string& courseId);
bool dropCourse(const std::string& studentId, const std::string& courseId);
```

学生选课或退课，操作成功返回true，失败返回false。

#### 选课查询

```cpp
Enrollment* getEnrollment(const std::string& studentId, const std::string& courseId);
std::vector<Enrollment*> getStudentEnrollments(const std::string& studentId) const;
std::vector<Enrollment*> getCourseEnrollments(const std::string& courseId) const;
bool isEnrolled(const std::string& studentId, const std::string& courseId) const;
```

获取选课记录或检查是否已选课。

#### 数据持久化

```cpp
bool loadData();
bool saveData();
```

从文件加载选课数据或保存选课数据，成功返回true，失败返回false。

## 系统管理 API

### CourseSystem 类

单例类，系统核心控制器。

#### 获取单例

```cpp
static CourseSystem& getInstance();
```

返回CourseSystem的单例实例。

#### 系统生命周期

```cpp
bool initialize(const std::string& dataDir, const std::string& logDir);
int run();
void shutdown();
```

初始化、运行和关闭系统。

#### 用户会话管理

```cpp
bool login(const std::string& userId, const std::string& password);
void logout();
User* getCurrentUser() const;
bool checkPermission(UserType requiredUserType) const;
```

用户登录、注销、获取当前用户和权限检查。

#### 密码管理

```cpp
bool changePassword(const std::string& userId, const std::string& oldPassword, 
                    const std::string& newPassword, const std::string& confirmPassword);
```

修改用户密码，需要提供正确的旧密码，并确保新密码和确认密码一致。密码长度需要至少6位。
修改成功返回true，失败返回false。密码将使用盐值加密存储，确保安全性。

#### 国际化支持

```cpp
bool selectLanguage(Language language);
Language getCurrentLanguage() const;
std::string getText(const std::string& key) const;
```

语言选择和多语言文本获取。

## 辅助工具 API

### DataManager 类

单例类，负责数据文件的读写。

```cpp
std::string loadJsonFromFile(const std::string& filename);
bool saveJsonToFile(const std::string& filename, const std::string& jsonData);
```

从文件加载JSON数据或保存JSON数据。

### Logger 类

单例类，负责日志记录。

```cpp
void debug(const std::string& message);
void info(const std::string& message);
void warning(const std::string& message);
void error(const std::string& message);
void critical(const std::string& message);
```

不同级别的日志记录。

### I18nManager 类

单例类，负责国际化资源管理。

```cpp
bool setLanguage(Language language);
std::string getText(const std::string& key) const;
```

设置语言和获取翻译文本。

### InputValidator 类

静态工具类，负责输入验证。

```cpp
static bool validateInteger(const std::string& input, int min, int max, int& result);
static bool validateDouble(const std::string& input, double min, double max, double& result);
static bool validateId(const std::string& id);
static bool validateName(const std::string& name);
static bool validatePassword(const std::string& password);
```

各种数据类型和格式的验证。

## 异常处理

### SystemException 类

系统异常类，统一异常处理。

```cpp
SystemException(ErrorType type, const std::string& message, int errorCode = 0);
ErrorType getType() const;
int getErrorCode() const;
std::string getTypeString() const;
std::string getFormattedMessage() const;
```

创建和获取异常信息。 