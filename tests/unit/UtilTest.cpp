#include <gtest/gtest.h>
#include "../../include/util/DataManager.h"
#include "../../include/util/Logger.h"
#include "../../include/util/I18nManager.h"
#include "../../include/util/InputValidator.h"
#include <filesystem>
#include <fstream>
#include <string>

// 工具类的测试fixture
class UtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        testDir = "./test_util";
        std::filesystem::create_directory(testDir);
    }

    void TearDown() override {
        // 清理测试环境
        std::filesystem::remove_all(testDir);
    }

    std::string testDir;
};

// 测试DataManager类
TEST_F(UtilTest, DataManagerTest) {
    DataManager& dataManager = DataManager::getInstance();
    
    // 设置数据目录
    dataManager.setDataDirectory(testDir);
    EXPECT_EQ(testDir, dataManager.getDataDirectory());
    
    // 测试文件不存在
    EXPECT_FALSE(dataManager.fileExists(testDir + "/nonexistent.json"));
    
    // 测试创建目录
    std::string newDir = testDir + "/subdir";
    EXPECT_TRUE(dataManager.createDirectory(newDir));
    EXPECT_TRUE(std::filesystem::exists(newDir));
    
    // 测试保存和加载JSON
    std::string testFile = testDir + "/test.json";
    std::string jsonData = R"({"test": "value", "number": 123})";
    EXPECT_TRUE(dataManager.saveJsonToFile(testFile, jsonData));
    EXPECT_TRUE(dataManager.fileExists(testFile));
    
    std::string loadedData = dataManager.loadJsonFromFile(testFile);
    EXPECT_FALSE(loadedData.empty());
    EXPECT_EQ(jsonData, loadedData);
    
    // 测试获取数据文件路径
    EXPECT_EQ(testDir + "/file.json", dataManager.getDataFilePath("file.json"));
}

// 测试Logger类
TEST_F(UtilTest, LoggerTest) {
    Logger& logger = Logger::getInstance();
    
    // 初始化日志系统
    std::string logDir = testDir + "/log";
    EXPECT_TRUE(logger.initialize(logDir, LogLevel::INFO));
    
    // 测试日志级别转换
    EXPECT_EQ("DEBUG", Logger::logLevelToString(LogLevel::DEBUG));
    EXPECT_EQ("INFO", Logger::logLevelToString(LogLevel::INFO));
    EXPECT_EQ("WARNING", Logger::logLevelToString(LogLevel::WARNING));
    EXPECT_EQ("ERROR", Logger::logLevelToString(LogLevel::ERROR));
    EXPECT_EQ("CRITICAL", Logger::logLevelToString(LogLevel::CRITICAL));
    
    EXPECT_EQ(LogLevel::DEBUG, Logger::stringToLogLevel("DEBUG"));
    EXPECT_EQ(LogLevel::INFO, Logger::stringToLogLevel("INFO"));
    EXPECT_EQ(LogLevel::WARNING, Logger::stringToLogLevel("WARNING"));
    EXPECT_EQ(LogLevel::ERROR, Logger::stringToLogLevel("ERROR"));
    EXPECT_EQ(LogLevel::CRITICAL, Logger::stringToLogLevel("CRITICAL"));
    
    // 测试记录日志
    logger.info("测试信息日志");
    logger.warning("测试警告日志");
    logger.error("测试错误日志");
    
    // 测试设置日志级别
    logger.setLogLevel(LogLevel::ERROR);
    
    // 注意：这里只是测试API调用，实际日志内容需要检查日志文件
}

// 测试I18nManager类
TEST_F(UtilTest, I18nManagerTest) {
    I18nManager& i18n = I18nManager::getInstance();
    
    // 创建测试语言文件
    std::string zhFile = testDir + "/Chinese.json";
    std::string enFile = testDir + "/English.json";
    
    std::string zhContent = R"({
        "test_key": "测试文本",
        "greeting": "你好，{0}！",
        "info": "这是一个{0}的{1}"
    })";
    
    std::string enContent = R"({
        "test_key": "Test Text",
        "greeting": "Hello, {0}!",
        "info": "This is a {1} of {0}"
    })";
    
    std::ofstream zhOut(zhFile);
    zhOut << zhContent;
    zhOut.close();
    
    std::ofstream enOut(enFile);
    enOut << enContent;
    enOut.close();
    
    // 初始化国际化系统
    EXPECT_TRUE(i18n.initialize(testDir));
    
    // 测试设置语言
    EXPECT_TRUE(i18n.setLanguage(Language::CHINESE));
    EXPECT_EQ(Language::CHINESE, i18n.getCurrentLanguage());
    
    // 测试获取翻译
    EXPECT_EQ("测试文本", i18n.getText("test_key"));
    EXPECT_EQ("你好，世界！", i18n.getFormattedText("greeting", "世界"));
    EXPECT_EQ("这是一个测试的示例", i18n.getFormattedText("info", "测试", "示例"));
    
    // 测试切换语言
    EXPECT_TRUE(i18n.setLanguage(Language::ENGLISH));
    EXPECT_EQ(Language::ENGLISH, i18n.getCurrentLanguage());
    
    // 测试获取翻译
    EXPECT_EQ("Test Text", i18n.getText("test_key"));
    EXPECT_EQ("Hello, World!", i18n.getFormattedText("greeting", "World"));
    EXPECT_EQ("This is a example of test", i18n.getFormattedText("info", "test", "example"));
    
    // 测试不存在的键
    EXPECT_EQ("unknown_key", i18n.getText("unknown_key"));
}

// 测试InputValidator类
TEST_F(UtilTest, InputValidatorTest) {
    InputValidator& validator = InputValidator::getInstance();
    
    // 测试ID验证
    EXPECT_TRUE(validator.validateId("user123"));
    EXPECT_TRUE(validator.validateId("USER_123"));
    EXPECT_FALSE(validator.validateId("user 123"));  // 包含空格
    EXPECT_FALSE(validator.validateId("user@123"));  // 包含特殊字符
    EXPECT_FALSE(validator.validateId(""));          // 空字符串
    
    // 测试名称验证
    EXPECT_TRUE(validator.validateName("张三"));
    EXPECT_TRUE(validator.validateName("John Doe"));
    EXPECT_FALSE(validator.validateName(""));        // 空字符串
    
    // 测试密码验证
    EXPECT_TRUE(validator.validatePassword("password123"));
    EXPECT_TRUE(validator.validatePassword("P@ssw0rd"));
    EXPECT_FALSE(validator.validatePassword("123"));  // 太短
    EXPECT_FALSE(validator.validatePassword(""));     // 空字符串
    
    // 测试数字验证
    EXPECT_TRUE(validator.validateNumber("123"));
    EXPECT_TRUE(validator.validateNumber("0"));
    EXPECT_TRUE(validator.validateNumber("-123"));
    EXPECT_FALSE(validator.validateNumber("12a3"));   // 包含字母
    EXPECT_FALSE(validator.validateNumber(""));       // 空字符串
    
    // 测试浮点数验证
    EXPECT_TRUE(validator.validateFloat("123.45"));
    EXPECT_TRUE(validator.validateFloat("0.0"));
    EXPECT_TRUE(validator.validateFloat("-123.45"));
    EXPECT_FALSE(validator.validateFloat("12a3.45")); // 包含字母
    EXPECT_FALSE(validator.validateFloat(""));        // 空字符串
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
