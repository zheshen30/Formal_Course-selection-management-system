**# 类图设计文档**

下图展示了系统的主要类及其关系：

\```mermaid

classDiagram

  class User {

​    <<abstract>>

​    +string userId

​    +string passwordHash

​    +string name

​    +string contactInfo

​    +User(userId, password, name, contactInfo)

​    +string getUserId()

​    +UserType getUserType()

​    +bool authenticate(password)

​    +void displayInfo()

​    +string toJson()

  }

  

  class Student {

​    +string studentId

​    +string gender

​    +int age

​    +string department

​    +string className

​    +vector~string~ enrolledCourses

​    +Student(userId, password, name, contactInfo, gender, age, department, className)

​    +bool enrollCourse(courseId)

​    +bool dropCourse(courseId)

​    +bool isEnrolledIn(courseId)

  }

  

  class Teacher {

​    +string teacherId

​    +string department

​    +string title

​    +vector~string~ teachingCourses

​    +Teacher(userId, password, name, contactInfo, department, title)

​    +bool addCourse(courseId)

​    +bool removeCourse(courseId)

​    +bool isTeaching(courseId)

  }

  

  class Admin {

​    +string role

​    +int permissionLevel

​    +Admin(userId, password, name, contactInfo, role, permissionLevel)

  }

  

  class Course {

​    +string courseId

​    +string courseName

​    +string courseType

​    +int totalHours

​    +float credits

​    +string semester

​    +int capacity

​    +int currentEnrollment

​    +string teacherId

​    +unordered_set~string~ enrolledStudents

​    +Course(courseId, courseName, courseType, totalHours, credits, semester, capacity)

​    +bool addStudent(studentId)

​    +bool removeStudent(studentId)

​    +bool isStudentEnrolled(studentId)

​    +bool isFull()

  }

  

  class Enrollment {

​    +string studentId

​    +string courseId

​    +string status

​    +optional~float~ grade

​    +Enrollment(studentId, courseId, status)

​    +string getStudentId()

​    +string getCourseId()

​    +string getStatus()

​    +void setStatus(status)

​    +optional~float~ getGrade()

​    +void setGrade(grade)

  }

  

  class CourseSystem {

​    -unique_ptr~UserManager~ userManager

​    -unique_ptr~CourseManager~ courseManager

​    -unique_ptr~EnrollmentManager~ enrollmentManager

​    -User* currentUser

​    -mutex systemMutex

​    -shared_ptr~Logger~ logger

​    +CourseSystem()

​    +void loadData()

​    +void saveData()

​    +bool login(userId, password)

​    +void logout()

​    +User::UserType getCurrentUserType()

​    +bool enrollCourse(studentId, courseId)

​    +bool dropCourse(studentId, courseId)

​    +bool setLanguage(locale)

​    +string getCurrentLanguage()

  }

  

  class UserManager {

​    -unordered_map~string, unique_ptr~User~~ users

​    -mutex userMutex

​    -shared_ptr~Logger~ logger

​    +UserManager()

​    +bool addStudent(userId, password, name, contactInfo, gender, age, department, className)

​    +bool updateStudent(userId, name, contactInfo, gender, age, department, className)

​    +bool addTeacher(userId, password, name, contactInfo, department, title)

​    +bool updateTeacher(userId, name, contactInfo, department, title)

​    +bool deleteUser(userId)

​    +User* findUserById(userId)

​    +bool saveUsers(filePath)

​    +bool loadUsers(filePath)

  }

  

  class CourseManager {

​    -unordered_map~string, unique_ptr~Course~~ courses

​    -mutex courseMutex

​    -shared_ptr~Logger~ logger

​    +CourseManager()

​    +bool addCourse(courseId, courseName, courseType, totalHours, credits, semester, capacity, teacherId)

​    +bool updateCourse(courseId, courseName, courseType, totalHours, credits, semester, capacity, teacherId)

​    +bool deleteCourse(courseId)

​    +Course* findCourseById(courseId)

​    +bool saveCourses(filePath)

​    +bool loadCourses(filePath)

​    +vector~Course*~ searchCoursesByName(name)

  }

  

  class EnrollmentManager {

​    -vector~Enrollment~ enrollments

​    -mutex enrollmentMutex

​    -shared_ptr~Logger~ logger

​    +EnrollmentManager()

​    +bool enrollStudent(studentId, courseId, userManager, courseManager)

​    +bool dropCourse(studentId, courseId, userManager, courseManager)

​    +bool setGrade(studentId, courseId, grade, userManager, courseManager)

​    +vector~Enrollment~ getEnrollmentsByStudent(studentId)

​    +vector~Enrollment~ getEnrollmentsByCourse(courseId)

​    +bool saveEnrollments(filePath)

​    +bool loadEnrollments(filePath)

  }

  

  class DataManager {

​    <<static>>

​    +static bool saveUsers(users, filePath)

​    +static bool saveCourses(courses, filePath)

​    +static bool saveEnrollments(enrollments, filePath)

​    +static bool loadUsers(users, filePath)

​    +static bool loadCourses(courses, filePath)

​    +static bool loadEnrollments(enrollments, filePath)

​    +static bool backupData(directory)

  }

  

  class Logger {

​    <<singleton>>

​    -shared_ptr~Logger~ instance

​    -mutex instanceMutex

​    -shared_ptr~spdlog::logger~ logger

​    -Logger()

​    +static shared_ptr~Logger~ getInstance()

​    +void info(message, ...)

​    +void warn(message, ...)

​    +void error(message, ...)

  }

  

  class I18nManager {

​    <<singleton>>

​    -shared_ptr~I18nManager~ instance

​    -mutex instanceMutex

​    -string currentLocale

​    -vector~string~ supportedLocales

​    -unordered_map resources

​    -I18nManager()

​    +static shared_ptr~I18nManager~ getInstance()

​    +bool setLocale(locale)

​    +string getText(key, defaultText)

​    +bool loadFromFile(locale, filePath)

  }

  

  class AuditLog {

​    <<singleton>>

​    -shared_ptr~AuditLog~ instance

​    -mutex instanceMutex

​    -vector~AuditRecord~ records

​    -AuditLog()

​    +static shared_ptr~AuditLog~ getInstance()

​    +void log(userId, action, targetId, targetType, details, status)

​    +vector~AuditRecord~ query(userId, startTime, endTime, action, limit)

  }

  

  class InputValidator {

​    <<static>>

​    +static bool isValidUserId(userId)

​    +static bool isValidPassword(password)

​    +static bool isValidEmail(email)

  }

  

  class LockGuard {

​    -mutex* m_mutex

​    -shared_mutex* m_sharedMutex

​    -Mode m_mode

​    -bool m_locked

​    +LockGuard(mutex, timeoutMs, file, line)

​    +LockGuard(shared_mutex, mode, timeoutMs, file, line)

​    +~LockGuard()

​    +void unlock()

​    +bool isLocked()

  }

  

  class SystemException {

​    -ErrorType m_errorType

​    -int m_errorCode

​    -string m_file

​    -int m_line

​    +SystemException(message, errorType, errorCode)

​    +SystemException(message, errorType, file, line, errorCode)

​    +ErrorType getErrorType()

​    +int getErrorCode()

​    +string getFile()

​    +int getLine()

​    +string getFullMessage()

​    +static string errorTypeToString(type)

  }

  

  class ObjectPool~T~ {

​    -queue~T*~ pool

​    -mutex poolMutex

​    -size_t poolSize

​    -function~void(T*)~ cleanupFunc

​    +ObjectPool(initialSize, maxSize, cleanupFunc)

​    +~ObjectPool()

​    +T* acquire()

​    +void release(object)

​    +size_t getAvailableCount()

  }

  

  User <|-- Student

  User <|-- Teacher

  User <|-- Admin

  CourseSystem --> UserManager

  CourseSystem --> CourseManager

  CourseSystem --> EnrollmentManager

  CourseSystem --> Logger

  CourseSystem --> AuditLog

  CourseSystem --> I18nManager

  

  UserManager ..> User

  UserManager ..> Student

  UserManager ..> Teacher

  UserManager ..> Admin

  UserManager ..> Logger

  

  CourseManager ..> Course

  CourseManager ..> Logger

  

  EnrollmentManager ..> Enrollment

  EnrollmentManager ..> Logger

  EnrollmentManager ..> UserManager

  EnrollmentManager ..> CourseManager

  

  UserManager ..> DataManager

  CourseManager ..> DataManager

  EnrollmentManager ..> DataManager

\```

**## 类说明**

**### 核心类**

1. ***\*User\****（用户基类）

  \- 抽象基类，为所有用户类型提供基本属性和方法

  \- 包含用户ID、密码哈希、姓名、联系方式等基本信息

  \- 提供用户类型识别和密码验证功能

  \- 支持移动语义，优化性能

2. ***\*Student\****（学生类）

  \- 继承自User类

  \- 包含学生特有的属性：性别、年龄、院系、班级等

  \- 管理选课信息，包括已选课程列表

  \- 提供选课、退课和查询选课状态方法

3. ***\*Teacher\****（教师类）

  \- 继承自User类

  \- 包含教师特有的属性：所在院系、职称等

  \- 管理授课课程列表

  \- 提供添加课程、移除课程和查询教授课程状态方法

4. ***\*Admin\****（管理员类）

  \- 继承自User类

  \- 包含管理员特有的属性：角色和权限级别

  \- 可以管理系统中的所有用户、课程和选课记录

5. ***\*Course\****（课程类）

  \- 管理课程基本信息：课程编号、名称、类型、学时、学分等

  \- 维护选课学生列表和当前选课人数

  \- 提供学生选课、退课和容量检查功能

  \- 支持移动语义，优化性能

6. ***\*Enrollment\****（选课记录类）

  \- 记录学生选课信息：学生ID、课程ID、选课状态

  \- 管理选课状态（已选、已退等）

  \- 包含可选的成绩信息

  \- 支持移动语义，优化性能

7. ***\*CourseSystem\****（系统核心类）

  \- 系统的中央控制器，协调各管理器的工作

  \- 管理用户登录和会话状态

  \- 委托具体业务操作给专门的管理器处理

  \- 提供系统初始化和配置功能

**### 管理器类**

1. ***\*UserManager\****（用户管理器）

  \- 负责所有用户相关的操作

  \- 管理用户的增删改查

  \- 处理用户认证

  \- 负责用户数据的持久化

2. ***\*CourseManager\****（课程管理器）

  \- 负责所有课程相关的操作

  \- 管理课程的增删改查

  \- 提供课程搜索和筛选功能

  \- 负责课程数据的持久化

3. ***\*EnrollmentManager\****（选课管理器）

  \- 负责所有选课相关的操作

  \- 处理学生选课和退课请求

  \- 管理成绩录入和查询

  \- 负责选课数据的持久化

**### 辅助类**

1. ***\*DataManager\****（数据管理类）

  \- 静态类，处理数据持久化操作

  \- 管理用户、课程和选课记录的文件读写

  \- 提供数据备份和恢复功能

2. ***\*Logger\****（日志类）

  \- 单例模式实现的日志记录器

  \- 基于spdlog库，提供不同级别的日志记录

  \- 线程安全设计，支持并发日志记录

3. ***\*I18nManager\****（国际化管理类）

  \- 单例模式实现的多语言支持

  \- 管理多语言资源文件和语言切换

  \- 提供文本翻译和语言设置功能

4. ***\*AuditLog\****（审计日志类）

  \- 单例模式实现的安全审计功能

  \- 记录关键操作：用户登录、选课、用户管理等

  \- 提供审计日志查询和导出功能

5. ***\*InputValidator\****（输入验证类）

  \- 静态工具类，提供各类输入验证方法

  \- 验证用户ID、密码、邮箱等输入的合法性

  \- 防止非法输入和注入攻击

6. ***\*LockGuard\****（锁管理类）

  \- RAII风格的锁封装，自动管理锁的获取和释放

  \- 支持普通互斥锁和共享互斥锁（读写锁）

  \- 提供超时机制，防止死锁

  \- 支持锁定位置跟踪，便于调试

7. ***\*SystemException\****（系统异常类）

  \- 统一的异常处理基类，继承自std::runtime_error

  \- 提供详细的错误类型和错误代码

  \- 记录异常发生的文件和行号

  \- 提供格式化的错误信息生成

8. ***\*ObjectPool\****（对象池类）

  \- 模板类，用于管理频繁创建和销毁的对象

  \- 减少内存碎片和提高性能

  \- 线程安全设计，支持并发访问

  \- 提供对象生命周期管理和清理功能

**## 类关系解释**

1. ***\*继承关系\****

  \- Student、Teacher和Admin类都继承自User类

  \- 遵循"是一种"关系，共享基本用户属性和行为

2. ***\*组合关系\****

  \- CourseSystem包含UserManager、CourseManager和EnrollmentManager

  \- 采用unique_ptr管理这些组件的生命周期

3. ***\*依赖关系\****

  \- UserManager依赖User及其子类，进行用户管理

  \- CourseManager依赖Course，进行课程管理

  \- EnrollmentManager依赖Enrollment、UserManager和CourseManager，处理选课逻辑

  \- 各管理器依赖Logger进行日志记录

  \- 各管理器依赖DataManager进行数据持久化

4. ***\*聚合关系\****

  \- UserManager聚合多个User对象

  \- CourseManager聚合多个Course对象

  \- EnrollmentManager聚合多个Enrollment对象

5. ***\*工具类依赖\****

  \- 所有组件都可能使用LockGuard进行线程同步

  \- 所有组件都可能抛出和处理SystemException

  \- 特定场景下使用ObjectPool优化对象创建和销毁 