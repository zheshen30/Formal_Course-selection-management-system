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
#include <gtest/gtest.h>
#include "../../include/util/DataManager.h"
#include "../../include/util/Logger.h"
#include "../../include/util/I18nManager.h"
#include "../../include/util/InputValidator.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include "../test_pch.h"

// 工具类的测试fixture
class UtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        testDir = "../test_data";
        testLogDir = "../test_log";
        
        try {
            std::filesystem::create_directories(testDir);
            std::filesystem::create_directories(testLogDir);
        } catch (const std::exception& e) {
            std::cerr << "创建测试目录异常: " << e.what() << std::endl;
        }
    }

    void TearDown() override {
        // 清理测试环境
        try {
            // 延迟一小段时间，确保文件操作完成
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // 彻底清理测试目录和日志目录
            TestUtils::cleanTestDirectory(testDir);
            TestUtils::cleanTestDirectory(testLogDir);
        } catch (const std::exception& e) {
            std::cerr << "清理测试目录异常: " << e.what() << std::endl;
        }
    }

    std::string testDir;
    std::string testLogDir;
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
    std::string testFile = testDir + "/test_util.json";
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
    EXPECT_TRUE(logger.initialize(testLogDir, LogLevel::INFO));
    
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
        "simple_text": "简单文本"
    })";
    
    std::string enContent = R"({
        "test_key": "Test Text",
        "greeting": "Hello, {0}!",
        "simple_text": "Simple Text"
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
    EXPECT_EQ("简单文本", i18n.getText("simple_text"));
    
    // 使用std::string进行单参数测试
    std::string world_zh = "世界";
    EXPECT_EQ("你好，世界！", i18n.getFormattedText("greeting", world_zh));
    
    // 测试切换语言
    EXPECT_TRUE(i18n.setLanguage(Language::ENGLISH));
    EXPECT_EQ(Language::ENGLISH, i18n.getCurrentLanguage());
    
    // 测试获取翻译
    EXPECT_EQ("Test Text", i18n.getText("test_key"));
    EXPECT_EQ("Simple Text", i18n.getText("simple_text"));
    
    // 使用std::string进行单参数测试
    std::string world_en = "World";
    EXPECT_EQ("Hello, World!", i18n.getFormattedText("greeting", world_en));
    
    // 测试不存在的键
    EXPECT_EQ("unknown_key", i18n.getText("unknown_key"));
}

// 测试InputValidator类
TEST_F(UtilTest, InputValidatorTest) {
    // 测试整数验证
    int value;
    EXPECT_TRUE(InputValidator::validateInteger("123", std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), value));
    EXPECT_EQ(value, 123);
    EXPECT_TRUE(InputValidator::validateInteger("0", std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), value));
    EXPECT_EQ(value, 0);
    EXPECT_TRUE(InputValidator::validateInteger("-123", std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), value));
    EXPECT_EQ(value, -123);
    EXPECT_FALSE(InputValidator::validateInteger("12a3", std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), value));
    EXPECT_FALSE(InputValidator::validateInteger("", std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), value));
    
    // 测试浮点数验证
    double dvalue;
    EXPECT_TRUE(InputValidator::validateDouble("123.45", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), dvalue));
    EXPECT_DOUBLE_EQ(dvalue, 123.45);
    EXPECT_TRUE(InputValidator::validateDouble("0.0", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), dvalue));
    EXPECT_DOUBLE_EQ(dvalue, 0.0);
    EXPECT_TRUE(InputValidator::validateDouble("-123.45", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), dvalue));
    EXPECT_DOUBLE_EQ(dvalue, -123.45);
    EXPECT_FALSE(InputValidator::validateDouble("12a3.45", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), dvalue));
    EXPECT_FALSE(InputValidator::validateDouble("", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), dvalue));
    
    // 测试选项验证
    EXPECT_TRUE(InputValidator::validateChoice("1", 1, 5, value));
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(InputValidator::validateChoice("5", 1, 5, value));
    EXPECT_EQ(value, 5);
    EXPECT_FALSE(InputValidator::validateChoice("0", 1, 5, value));
    EXPECT_FALSE(InputValidator::validateChoice("6", 1, 5, value));
    EXPECT_FALSE(InputValidator::validateChoice("a", 1, 5, value));
    EXPECT_FALSE(InputValidator::validateChoice("", 1, 5, value));
    
    // 测试空输入验证
    EXPECT_TRUE(InputValidator::isEmptyInput(""));
    EXPECT_TRUE(InputValidator::isEmptyInput("   "));
    EXPECT_TRUE(InputValidator::isEmptyInput("\t\n"));
    EXPECT_FALSE(InputValidator::isEmptyInput("abc"));
    EXPECT_FALSE(InputValidator::isEmptyInput(" abc "));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
