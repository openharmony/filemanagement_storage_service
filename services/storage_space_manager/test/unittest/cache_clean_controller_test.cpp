/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include "cache_clean_controller/cache_clean_controller.h"
#include "storage_space_manager_errno.h"
#include "storage_service_constant.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

class CacheCleanControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static CacheCleanController* controller_;
    static std::string testConfigDir_;
    static std::string testConfigFile_;
};

CacheCleanController* CacheCleanControllerTest::controller_ = nullptr;
std::string CacheCleanControllerTest::testConfigDir_ = "/data/service/el1/public/storage_space_manager";
std::string CacheCleanControllerTest::testConfigFile_ =
    "/data/service/el1/public/storage_space_manager/cache_clean_config.json";

void CacheCleanControllerTest::SetUpTestCase()
{
    controller_ = DelayedSingleton<CacheCleanController>::GetInstance().get();

    // Create test directory
    constexpr int dirPermission = 0755;
    mkdir(testConfigDir_.c_str(), DIR_PERMISSION);
}

void CacheCleanControllerTest::TearDownTestCase()
{
    // Clean up test files
    std::remove(testConfigFile_.c_str());
    rmdir(testConfigDir_.c_str());
}

void CacheCleanControllerTest::SetUp()
{
    // Remove test config file if exists
    std::remove(testConfigFile_.c_str());
}

void CacheCleanControllerTest::TearDown()
{
    // Clean up after each test
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0001
 * @tc.name: SaveCacheCleaningTimestamp_Success
 * @tc.desc: Test saving cache cleaning timestamp successfully
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_Success start";

    ASSERT_NE(controller_, nullptr);

    int64_t testTimestamp = 1718515200000LL;  // 2024-06-16 00:00:00

    int32_t ret = controller_->SaveCacheCleaningTimestamp(testTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify file was created and contains correct content
    std::ifstream configFile(testConfigFile_);
    ASSERT_TRUE(configFile.is_open());

    std::string content((std::istreambuf_iterator<char>(configFile)),
                        std::istreambuf_iterator<char>());
    configFile.close();

    EXPECT_NE(content.find("last_cache_clean_timestamp"), std::string::npos);
    EXPECT_NE(content.find("1718515200000"), std::string::npos);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_Success end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0002
 * @tc.name: SaveCacheCleaningTimestamp_MultipleTimes
 * @tc.desc: Test saving timestamp multiple times (should overwrite)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_MultipleTimes, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_MultipleTimes start";

    ASSERT_NE(controller_, nullptr);

    // Save first timestamp
    int64_t firstTimestamp = 1718515200000LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(firstTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Save second timestamp (should overwrite)
    int64_t secondTimestamp = 1718601600000LL;
    ret = controller_->SaveCacheCleaningTimestamp(secondTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify file contains the second timestamp
    std::ifstream configFile(testConfigFile_);
    ASSERT_TRUE(configFile.is_open());

    std::string content((std::istreambuf_iterator<char>(configFile)),
                        std::istreambuf_iterator<char>());
    configFile.close();

    EXPECT_NE(content.find("1718601600000"), std::string::npos);
    EXPECT_EQ(content.find("1718515200000"), std::string::npos);  // First timestamp should be gone

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_MultipleTimes end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0003
 * @tc.name: SaveCacheCleaningTimestamp_MaxTimestamp
 * @tc.desc: Test saving maximum timestamp value
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_MaxTimestamp, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_MaxTimestamp start";

    ASSERT_NE(controller_, nullptr);

    int64_t maxTimestamp = INT64_MAX;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(maxTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify file was created
    std::ifstream configFile(testConfigFile_);
    EXPECT_TRUE(configFile.is_open());
    configFile.close();

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_MaxTimestamp end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0004
 * @tc.name: SaveCacheCleaningTimestamp_ZeroTimestamp
 * @tc.desc: Test saving zero timestamp
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_ZeroTimestamp, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_ZeroTimestamp start";

    ASSERT_NE(controller_, nullptr);

    int64_t zeroTimestamp = 0LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(zeroTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify file was created
    std::ifstream configFile(testConfigFile_);
    EXPECT_TRUE(configFile.is_open());
    configFile.close();

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_ZeroTimestamp end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0005
 * @tc.name: SaveCacheCleaningTimestamp_NegativeTimestamp
 * @tc.desc: Test saving negative timestamp
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_NegativeTimestamp, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_NegativeTimestamp start";

    ASSERT_NE(controller_, nullptr);

    int64_t negativeTimestamp = -1000LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(negativeTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify file was created
    std::ifstream configFile(testConfigFile_);
    EXPECT_TRUE(configFile.is_open());
    configFile.close();

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_NegativeTimestamp end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0006
 * @tc.name: SaveCacheCleaningTimestamp_CreateDirectory
 * @tc.desc: Test saving timestamp when directory doesn't exist
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_CreateDirectory, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_CreateDirectory start";

    ASSERT_NE(controller_, nullptr);

    // Remove directory if it exists
    rmdir(testConfigDir_.c_str());

    int64_t testTimestamp = 1718515200000LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(testTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify directory was created
    struct stat st;
    EXPECT_EQ(stat(testConfigDir_.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));

    // Verify file was created
    std::ifstream configFile(testConfigFile_);
    EXPECT_TRUE(configFile.is_open());
    configFile.close();

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_CreateDirectory end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0007
 * @tc.name: SaveCacheCleaningTimestamp_JsonFormat
 * @tc.desc: Test that saved file has correct JSON format
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_JsonFormat, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_JsonFormat start";

    ASSERT_NE(controller_, nullptr);

    int64_t testTimestamp = 1718515200000LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(testTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify JSON format
    std::ifstream configFile(testConfigFile_);
    ASSERT_TRUE(configFile.is_open());

    std::string content((std::istreambuf_iterator<char>(configFile)),
                        std::istreambuf_iterator<char>());
    configFile.close();

    // Check for proper JSON structure
    EXPECT_NE(content.find("{"), std::string::npos);
    EXPECT_NE(content.find("}"), std::string::npos);
    EXPECT_NE(content.find("\""), std::string::npos);
    EXPECT_NE(content.find(":"), std::string::npos);

    // Check that only one field exists
    size_t timestampCount = 0;
    size_t pos = 0;
    while ((pos = content.find("last_cache_clean_timestamp", pos)) != std::string::npos) {
        timestampCount++;
        pos += strlen("last_cache_clean_timestamp");
    }
    EXPECT_EQ(timestampCount, 1);  // Should appear exactly once

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_JsonFormat end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0008
 * @tc.name: SaveCacheCleaningTimestamp_PrettyPrint
 * @tc.desc: Test that JSON is pretty-printed with proper indentation
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_PrettyPrint, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_PrettyPrint start";

    ASSERT_NE(controller_, nullptr);

    int64_t testTimestamp = 1718515200000LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(testTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify pretty printing
    std::ifstream configFile(testConfigFile_);
    ASSERT_TRUE(configFile.is_open());

    std::string line;
    bool foundNewline = false;
    while (std::getline(configFile, line)) {
        if (!line.empty()) {
            foundNewline = true;
            break;
        }
    }
    configFile.close();

    EXPECT_TRUE(foundNewline);  // Should have newlines for pretty printing

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_PrettyPrint end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0010
 * @tc.name: SaveCacheCleaningTimestamp_OverwriteExisting
 * @tc.desc: Test that saving timestamp overwrites existing file
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_OverwriteExisting, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_OverwriteExisting start";

    ASSERT_NE(controller_, nullptr);

    // Create initial file with some content
    std::ofstream initFile(testConfigFile_);
    initFile << R"({"old_field": "old_value"})";
    initFile.close();

    // Save new timestamp
    int64_t testTimestamp = 1718515200000LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(testTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify old content is gone
    std::ifstream configFile(testConfigFile_);
    ASSERT_TRUE(configFile.is_open());

    std::string content((std::istreambuf_iterator<char>(configFile)),
                        std::istreambuf_iterator<char>());
    configFile.close();

    EXPECT_EQ(content.find("old_field"), std::string::npos);
    EXPECT_EQ(content.find("old_value"), std::string::npos);
    EXPECT_NE(content.find("last_cache_clean_timestamp"), std::string::npos);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_OverwriteExisting end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0011
 * @tc.name: SaveCacheCleaningTimestamp_FilePath
 * @tc.desc: Test that file is saved in correct path
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_FilePath, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_FilePath start";

    ASSERT_NE(controller_, nullptr);

    int64_t testTimestamp = 1718515200000LL;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(testTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify file exists at expected path
    struct stat st;
    EXPECT_EQ(stat(testConfigFile_.c_str(), &st), 0);
    EXPECT_TRUE(S_ISREG(st.st_mode));  // Regular file

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_FilePath end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SaveTimestamp_0012
 * @tc.name: SaveCacheCleaningTimestamp_MinValue
 * @tc.desc: Test saving minimum int64 timestamp value
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, SaveCacheCleaningTimestamp_MinValue, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_MinValue start";

    ASSERT_NE(controller_, nullptr);

    int64_t minTimestamp = INT64_MIN;
    int32_t ret = controller_->SaveCacheCleaningTimestamp(minTimestamp);
    EXPECT_EQ(ret, E_OK);

    // Verify file was created
    std::ifstream configFile(testConfigFile_);
    EXPECT_TRUE(configFile.is_open());
    configFile.close();

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SaveCacheCleaningTimestamp_MinValue end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_FilterAppInfos_0001
 * @tc.name: FilterAppInfosByBundleType_AllApp
 * @tc.desc: Test filtering with all APP type applications
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, FilterAppInfosByBundleType_AllApp, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_AllApp start";

    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    ApplicationInfo app1;
    app1.bundleName = "com.example.app1";
    app1.bundleType = AppExecFwk::BundleType::APP;
    appInfos.push_back(app1);

    ApplicationInfo app2;
    app2.bundleName = "com.example.app2";
    app2.bundleType = AppExecFwk::BundleType::APP;
    appInfos.push_back(app2);

    std::vector<ApplicationInfo> filteredAppInfos;
    int32_t ret = controller_->FilterAppInfosByBundleType(appInfos, filteredAppInfos);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(filteredAppInfos.size(), 2);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_AllApp end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_FilterAppInfos_0002
 * @tc.name: FilterAppInfosByBundleType_Mixed
 * @tc.desc: Test filtering with mixed bundle types
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, FilterAppInfosByBundleType_Mixed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_Mixed start";

    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    ApplicationInfo app1;
    app1.bundleName = "com.example.app1";
    app1.bundleType = AppExecFwk::BundleType::APP;
    appInfos.push_back(app1);

    ApplicationInfo app2;
    app2.bundleName = "com.example.system";
    app2.bundleType = AppExecFwk::BundleType::APP_SERVICE_FWK;
    appInfos.push_back(app2);

    ApplicationInfo app3;
    app3.bundleName = "com.example.app2";
    app3.bundleType = AppExecFwk::BundleType::APP;
    appInfos.push_back(app3);

    std::vector<ApplicationInfo> filteredAppInfos;
    int32_t ret = controller_->FilterAppInfosByBundleType(appInfos, filteredAppInfos);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(filteredAppInfos.size(), 2);
    EXPECT_EQ(filteredAppInfos[0].bundleName, "com.example.app1");
    EXPECT_EQ(filteredAppInfos[1].bundleName, "com.example.app2");

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_Mixed end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_FilterAppInfos_0003
 * @tc.name: FilterAppInfosByBundleType_EmptyInput
 * @tc.desc: Test filtering with empty input
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, FilterAppInfosByBundleType_EmptyInput, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_EmptyInput start";

    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    std::vector<ApplicationInfo> filteredAppInfos;
    int32_t ret = controller_->FilterAppInfosByBundleType(appInfos, filteredAppInfos);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(filteredAppInfos.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_EmptyInput end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_FilterAppInfos_0004
 * @tc.name: FilterAppInfosByBundleType_OnlyNonApp
 * @tc.desc: Test filtering with only non-APP bundle types
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, FilterAppInfosByBundleType_OnlyNonApp, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_OnlyNonApp start";

    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    ApplicationInfo app1;
    app1.bundleName = "com.example.shared";
    app1.bundleType = AppExecFwk::BundleType::SHARED;
    appInfos.push_back(app1);

    ApplicationInfo app2;
    app2.bundleName = "com.example.service";
    app2.bundleType = AppExecFwk::BundleType::APP_SERVICE_FWK;
    appInfos.push_back(app2);

    std::vector<ApplicationInfo> filteredAppInfos;
    int32_t ret = controller_->FilterAppInfosByBundleType(appInfos, filteredAppInfos);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(filteredAppInfos.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterAppInfosByBundleType_OnlyNonApp end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetCurrentTime_0001
 * @tc.name: GetCurrentTime_ReasonableValue
 * @tc.desc: Test GetCurrentTime returns a reasonable timestamp
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetCurrentTime_ReasonableValue, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetCurrentTime_ReasonableValue start";

    ASSERT_NE(controller_, nullptr);

    int64_t currentTime = controller_->GetCurrentTime();
    EXPECT_GT(currentTime, 0);

    // Current time should be after 2020-01-01 (around 1577836800000 ms)
    // and before 2100-01-01 (around 4102444800000 ms)
    EXPECT_GT(currentTime, 1577836800000LL);
    EXPECT_LT(currentTime, 4102444800000LL);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetCurrentTime_ReasonableValue end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetCurrentTime_0002
 * @tc.name: GetCurrentTime_Monotonic
 * @tc.desc: Test GetCurrentTime returns increasing values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetCurrentTime_Monotonic, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetCurrentTime_Monotonic start";

    ASSERT_NE(controller_, nullptr);

    int64_t time1 = controller_->GetCurrentTime();
    int64_t time2 = controller_->GetCurrentTime();
    EXPECT_GE(time2, time1);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetCurrentTime_Monotonic end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_CalculateTimeRange_0001
 * @tc.name: CalculateTimeRange_Normal
 * @tc.desc: Test CalculateTimeRange with normal values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, CalculateTimeRange_Normal, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_CalculateTimeRange_Normal start";

    ASSERT_NE(controller_, nullptr);

    int64_t currentTime = 1718515200000LL;  // 2024-06-16 00:00:00
    int32_t hoursSpan = 24;
    int64_t startTime = 0;
    int64_t endTime = 0;

    controller_->CalculateTimeRange(currentTime, hoursSpan, startTime, endTime);

    // 24 hours in milliseconds = 24 * 60 * 60 * 1000 = 86400000
    EXPECT_EQ(startTime, currentTime - 86400000LL);
    EXPECT_EQ(endTime, currentTime);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_CalculateTimeRange_Normal end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_CalculateTimeRange_0002
 * @tc.name: CalculateTimeRange_ZeroHours
 * @tc.desc: Test CalculateTimeRange with zero hours
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, CalculateTimeRange_ZeroHours, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_CalculateTimeRange_ZeroHours start";

    ASSERT_NE(controller_, nullptr);

    int64_t currentTime = 1718515200000LL;
    int32_t hoursSpan = 0;
    int64_t startTime = -1;
    int64_t endTime = -1;

    controller_->CalculateTimeRange(currentTime, hoursSpan, startTime, endTime);

    EXPECT_EQ(startTime, currentTime);
    EXPECT_EQ(endTime, currentTime);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_CalculateTimeRange_ZeroHours end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_CalculateTimeRange_0003
 * @tc.name: CalculateTimeRange_LargeHours
 * @tc.desc: Test CalculateTimeRange with large hour span
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, CalculateTimeRange_LargeHours, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_CalculateTimeRange_LargeHours start";

    ASSERT_NE(controller_, nullptr);

    int64_t currentTime = 1718515200000LL;
    int32_t hoursSpan = 720;  // 30 days
    int64_t startTime = 0;
    int64_t endTime = 0;

    controller_->CalculateTimeRange(currentTime, hoursSpan, startTime, endTime);

    // 720 hours in milliseconds = 720 * 60 * 60 * 1000 = 2592000000
    EXPECT_EQ(startTime, currentTime - 2592000000LL);
    EXPECT_EQ(endTime, currentTime);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_CalculateTimeRange_LargeHours end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SetStopCleanCacheFlag_0002
 * @tc.name: SetStopCleanCacheFlag_MultipleTimes
 * @tc.desc: Test SetStopCleanCacheFlag multiple times
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, SetStopCleanCacheFlag_MultipleTimes, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SetStopCleanCacheFlag_MultipleTimes start";

    ASSERT_NE(controller_, nullptr);

    controller_->SetStopCleanCacheFlag(true);
    controller_->SetStopCleanCacheFlag(false);
    controller_->SetStopCleanCacheFlag(true);
    controller_->SetStopCleanCacheFlag(true);
    // No crash means success

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SetStopCleanCacheFlag_MultipleTimes end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_PrintOverLongLog_0001
 * @tc.name: PrintOverLongLog_EmptyString
 * @tc.desc: Test PrintOverLongLog with empty string
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, PrintOverLongLog_EmptyString, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_PrintOverLongLog_EmptyString start";

    ASSERT_NE(controller_, nullptr);

    std::string emptyStr = "";
    controller_->PrintOverLongLog(emptyStr);
    // No crash means success

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_PrintOverLongLog_EmptyString end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_PrintOverLongLog_0002
 * @tc.name: PrintOverLongLog_ShortString
 * @tc.desc: Test PrintOverLongLog with string shorter than boundary
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, PrintOverLongLog_ShortString, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_PrintOverLongLog_ShortString start";

    ASSERT_NE(controller_, nullptr);

    std::string shortStr = "This is a short log message";
    EXPECT_LT(shortStr.size(), 3000);
    controller_->PrintOverLongLog(shortStr);
    // No crash means success

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_PrintOverLongLog_ShortString end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_PrintOverLongLog_0003
 * @tc.name: PrintOverLongLog_SlightlyAboveBoundary
 * @tc.desc: Test PrintOverLongLog with string slightly above boundary
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, PrintOverLongLog_SlightlyAboveBoundary, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_PrintOverLongLog_SlightlyAboveBoundary start";

    ASSERT_NE(controller_, nullptr);

    std::string slightlyAboveStr(3500, 'b');
    EXPECT_GT(slightlyAboveStr.size(), 3000);
    controller_->PrintOverLongLog(slightlyAboveStr);
    // No crash means success, should split into 2 parts

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_PrintOverLongLog_SlightlyAboveBoundary end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_WriteCleanInfoToExtra_0001
 * @tc.name: WriteCleanInfoToExtra_Success_AppIndexZero
 * @tc.desc: Test WriteCleanInfoToExtra with success result and appIndex=0 (should not output appIndex field)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, WriteCleanInfoToExtra_Success_AppIndexZero, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Success_AppIndexZero start";

    ASSERT_NE(controller_, nullptr);
    controller_->extraData_.str("");

    CleanCacheInfo cleanInfo;
    cleanInfo.bundleName = "com.example.test";
    cleanInfo.appIndex = 0;
    cleanInfo.cacheThreshold = 1024 * 1024;
    cleanInfo.userId = 100;

    uint64_t before = 10 * 1024 * 1024;
    uint64_t after = 5 * 1024 * 1024;
    bool isSucc = true;

    controller_->WriteCleanInfoToExtra(cleanInfo, before, after, isSucc);

    std::string output = controller_->extraData_.str();
    EXPECT_NE(output.find("BN:com.example.test"), std::string::npos);
    EXPECT_NE(output.find("CT:1048576"), std::string::npos);
    EXPECT_NE(output.find("BS:10485760"), std::string::npos);
    EXPECT_NE(output.find("AS:5242880"), std::string::npos);
    EXPECT_EQ(output.find("IDX:"), std::string::npos);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Success_AppIndexZero end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_WriteCleanInfoToExtra_0002
 * @tc.name: WriteCleanInfoToExtra_Success_AppIndexNonZero
 * @tc.desc: Test WriteCleanInfoToExtra with success result and appIndex!=0 (should output appIndex field)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, WriteCleanInfoToExtra_Success_AppIndexNonZero, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Success_AppIndexNonZero start";

    ASSERT_NE(controller_, nullptr);
    controller_->extraData_.str("");

    CleanCacheInfo cleanInfo;
    cleanInfo.bundleName = "com.example.multi";
    cleanInfo.appIndex = 2;
    cleanInfo.cacheThreshold = 2048 * 1024;
    cleanInfo.userId = 100;

    uint64_t before = 20 * 1024 * 1024;
    uint64_t after = 8 * 1024 * 1024;
    bool isSucc = true;

    controller_->WriteCleanInfoToExtra(cleanInfo, before, after, isSucc);

    std::string output = controller_->extraData_.str();
    EXPECT_NE(output.find("BN:com.example.multi"), std::string::npos);
    EXPECT_NE(output.find(",IDX:2"), std::string::npos);
    EXPECT_NE(output.find("CT:2097152"), std::string::npos);
    EXPECT_NE(output.find("BS:20971520"), std::string::npos);
    EXPECT_NE(output.find("AS:8388608"), std::string::npos);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Success_AppIndexNonZero end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_WriteCleanInfoToExtra_0003
 * @tc.name: WriteCleanInfoToExtra_Failure_AppIndexZero
 * @tc.desc: Test WriteCleanInfoToExtra with failure result and
 *     appIndex=0 (should not output appIndex/beforeSize/afterSize)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, WriteCleanInfoToExtra_Failure_AppIndexZero, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Failure_AppIndexZero start";

    ASSERT_NE(controller_, nullptr);
    controller_->extraData_.str("");

    CleanCacheInfo cleanInfo;
    cleanInfo.bundleName = "com.example.fail";
    cleanInfo.appIndex = 0;
    cleanInfo.cacheThreshold = 3072 * 1024;
    cleanInfo.userId = 100;

    uint64_t before = 30 * 1024 * 1024;
    uint64_t after = 0;
    bool isSucc = false;

    controller_->WriteCleanInfoToExtra(cleanInfo, before, after, isSucc);

    std::string output = controller_->extraData_.str();
    EXPECT_NE(output.find("BN:com.example.fail"), std::string::npos);
    EXPECT_NE(output.find("CT:3145728"), std::string::npos);
    EXPECT_NE(output.find("FAIL"), std::string::npos);
    EXPECT_EQ(output.find("IDX:"), std::string::npos);
    EXPECT_EQ(output.find("BS:"), std::string::npos);
    EXPECT_EQ(output.find("AS:"), std::string::npos);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Failure_AppIndexZero end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_WriteCleanInfoToExtra_0004
 * @tc.name: WriteCleanInfoToExtra_Failure_AppIndexNonZero
 * @tc.desc: Test WriteCleanInfoToExtra with failure result and
 *     appIndex!=0 (should output appIndex but not beforeSize/afterSize)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, WriteCleanInfoToExtra_Failure_AppIndexNonZero, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Failure_AppIndexNonZero start";

    ASSERT_NE(controller_, nullptr);
    controller_->extraData_.str("");

    CleanCacheInfo cleanInfo;
    cleanInfo.bundleName = "com.example.failmulti";
    cleanInfo.appIndex = 3;
    cleanInfo.cacheThreshold = 4096 * 1024;
    cleanInfo.userId = 100;

    uint64_t before = 40 * 1024 * 1024;
    uint64_t after = 0;
    bool isSucc = false;

    controller_->WriteCleanInfoToExtra(cleanInfo, before, after, isSucc);

    std::string output = controller_->extraData_.str();
    EXPECT_NE(output.find("BN:com.example.failmulti"), std::string::npos);
    EXPECT_NE(output.find(",IDX:3"), std::string::npos);
    EXPECT_NE(output.find("CT:4194304"), std::string::npos);
    EXPECT_NE(output.find("FAIL"), std::string::npos);
    EXPECT_EQ(output.find("BS:"), std::string::npos);
    EXPECT_EQ(output.find("AS:"), std::string::npos);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_WriteCleanInfoToExtra_Failure_AppIndexNonZero end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0001
 * @tc.name: GetDefaultQuotaByRank_SmallStorage_Top1_3
 * @tc.desc: Test GetDefaultQuotaByRank with 128GB storage and rank=1 (should match top1-3: 750MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_SmallStorage_Top1_3, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Top1_3 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(1, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 750);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Top1_3 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0002
 * @tc.name: GetDefaultQuotaByRank_SmallStorage_Top4_10
 * @tc.desc: Test GetDefaultQuotaByRank with 128GB storage and rank=5 (should match top4-10: 300MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_SmallStorage_Top4_10, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Top4_10 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(5, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 300);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Top4_10 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0003
 * @tc.name: GetDefaultQuotaByRank_SmallStorage_Top11_20
 * @tc.desc: Test GetDefaultQuotaByRank with 128GB storage and rank=15 (should match top11-20: 150MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_SmallStorage_Top11_20, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Top11_20 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(15, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 150);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Top11_20 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0004
 * @tc.name: GetDefaultQuotaByRank_SmallStorage_Default
 * @tc.desc: Test GetDefaultQuotaByRank with 128GB storage and rank=30 (should fallback to default: 50MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_SmallStorage_Default, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Default start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(30, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 50);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_SmallStorage_Default end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0005
 * @tc.name: GetDefaultQuotaByRank_LargeStorage_Top1_3
 * @tc.desc: Test GetDefaultQuotaByRank with 500GB storage and rank=1 (should match top1-3: 1500MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_LargeStorage_Top1_3, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Top1_3 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(1, 500 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 1500);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Top1_3 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0006
 * @tc.name: GetDefaultQuotaByRank_LargeStorage_Top4_10
 * @tc.desc: Test GetDefaultQuotaByRank with 500GB storage and rank=8 (should match top4-10: 500MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_LargeStorage_Top4_10, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Top4_10 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(8, 500 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 500);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Top4_10 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0007
 * @tc.name: GetDefaultQuotaByRank_LargeStorage_Top11_20
 * @tc.desc: Test GetDefaultQuotaByRank with 500GB storage and rank=15 (should match top11-20: 250MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_LargeStorage_Top11_20, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Top11_20 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(15, 500 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 250);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Top11_20 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0008
 * @tc.name: GetDefaultQuotaByRank_LargeStorage_Default
 * @tc.desc: Test GetDefaultQuotaByRank with 500GB storage and rank=30 (should fallback to default: 50MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_LargeStorage_Default, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Default start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(30, 500 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 50);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_LargeStorage_Default end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0009
 * @tc.name: GetDefaultQuotaByRank_InvalidRank_Zero
 * @tc.desc: Test GetDefaultQuotaByRank with rank=0 (should return E_INVALID_ARGUMENT)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_InvalidRank_Zero, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_InvalidRank_Zero start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = -1;
    int32_t ret = controller_->GetDefaultQuotaByRank(0, 128 * GB, quota);
    EXPECT_EQ(ret, E_INVALID_ARGUMENT);
    EXPECT_EQ(quota, -1);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_InvalidRank_Zero end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0010
 * @tc.name: GetDefaultQuotaByRank_InvalidRank_Negative
 * @tc.desc: Test GetDefaultQuotaByRank with rank=-1 (should return E_INVALID_ARGUMENT)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_InvalidRank_Negative, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_InvalidRank_Negative start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = -1;
    int32_t ret = controller_->GetDefaultQuotaByRank(-1, 128 * GB, quota);
    EXPECT_EQ(ret, E_INVALID_ARGUMENT);
    EXPECT_EQ(quota, -1);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_InvalidRank_Negative end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0011
 * @tc.name: GetDefaultQuotaByRank_Boundary_256GB
 * @tc.desc: Test GetDefaultQuotaByRank with exactly 256GB (should match 0-256 range, top1-3: 750MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_Boundary_256GB, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_Boundary_256GB start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(1, 256 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 750);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_Boundary_256GB end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0012
 * @tc.name: GetDefaultQuotaByRank_Boundary_257GB
 * @tc.desc: Test GetDefaultQuotaByRank with exactly 257GB (should match 257-max range, top1-3: 1500MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_Boundary_257GB, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_Boundary_257GB start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(1, 257 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 1500);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_Boundary_257GB end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0013
 * @tc.name: GetDefaultQuotaByRank_ZeroStorage
 * @tc.desc: Test GetDefaultQuotaByRank with zero storage (should match 0-256 range, default: 50MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_ZeroStorage, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_ZeroStorage start";
    ASSERT_NE(controller_, nullptr);

    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(30, 0, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 50);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_ZeroStorage end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0014
 * @tc.name: GetDefaultQuotaByRank_TopBoundary_Rank3
 * @tc.desc: Test GetDefaultQuotaByRank with rank=3 (upper boundary of top1-3, should match 750MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_TopBoundary_Rank3, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank3 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(3, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 750);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank3 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0015
 * @tc.name: GetDefaultQuotaByRank_TopBoundary_Rank4
 * @tc.desc: Test GetDefaultQuotaByRank with rank=4 (lower boundary of top4-10, should match 300MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_TopBoundary_Rank4, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank4 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(4, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 300);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank4 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0016
 * @tc.name: GetDefaultQuotaByRank_TopBoundary_Rank10
 * @tc.desc: Test GetDefaultQuotaByRank with rank=10 (upper boundary of top4-10, should match 300MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_TopBoundary_Rank10, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank10 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(10, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 300);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank10 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_GetDefaultQuotaByRank_0017
 * @tc.name: GetDefaultQuotaByRank_TopBoundary_Rank11
 * @tc.desc: Test GetDefaultQuotaByRank with rank=11 (lower boundary of top11-20, should match 150MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, GetDefaultQuotaByRank_TopBoundary_Rank11, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank11 start";
    ASSERT_NE(controller_, nullptr);

    constexpr int64_t gb = 1024LL * 1024 * 1024;
    int32_t quota = 0;
    int32_t ret = controller_->GetDefaultQuotaByRank(11, 128 * GB, quota);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(quota, 150);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_GetDefaultQuotaByRank_TopBoundary_Rank11 end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_ConvertAppInfos_0001
 * @tc.name: ConvertAppInfosToCleanCacheInfo_EmptyInput
 * @tc.desc: Test ConvertAppInfosToCleanCacheInfo with empty input
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, ConvertAppInfosToCleanCacheInfo_EmptyInput, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertAppInfosToCleanCacheInfo_EmptyInput start";
    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    std::vector<CleanCacheInfo> result;
    controller_->ConvertAppInfosToCleanCacheInfo(appInfos, 100, result);
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertAppInfosToCleanCacheInfo_EmptyInput end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_ConvertAppInfos_0003
 * @tc.name: ConvertAppInfosToCleanCacheInfo_MultipleApps
 * @tc.desc: Test ConvertAppInfosToCleanCacheInfo with multiple appInfos including different appIndex
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, ConvertAppInfosToCleanCacheInfo_MultipleApps, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertAppInfosToCleanCacheInfo_MultipleApps start";
    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    ApplicationInfo app1;
    app1.bundleName = "com.example.app1";
    app1.appIndex = 0;
    appInfos.push_back(app1);

    ApplicationInfo app2;
    app2.bundleName = "com.example.app2";
    app2.appIndex = 2;
    appInfos.push_back(app2);

    std::vector<CleanCacheInfo> result;
    controller_->ConvertAppInfosToCleanCacheInfo(appInfos, 200, result);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].bundleName, "com.example.app1");
    EXPECT_EQ(result[0].userId, 200);
    EXPECT_EQ(result[0].appIndex, 0);
    EXPECT_EQ(result[0].cacheThreshold, 0);
    EXPECT_EQ(result[1].bundleName, "com.example.app2");
    EXPECT_EQ(result[1].userId, 200);
    EXPECT_EQ(result[1].appIndex, 2);
    EXPECT_EQ(result[1].cacheThreshold, 0);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertAppInfosToCleanCacheInfo_MultipleApps end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_ConvertAppInfos_0004
 * @tc.name: ConvertAppInfosToCleanCacheInfo_AppendToExisting
 * @tc.desc: Test ConvertAppInfosToCleanCacheInfo appends to non-empty output vector
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CacheCleanControllerTest, ConvertAppInfosToCleanCacheInfo_AppendToExisting, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertAppInfosToCleanCacheInfo_AppendToExisting start";
    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    ApplicationInfo app;
    app.bundleName = "com.example.app";
    app.appIndex = 0;
    appInfos.push_back(app);

    std::vector<CleanCacheInfo> result;
    CleanCacheInfo existing;
    existing.bundleName = "existing.app";
    existing.userId = 100;
    result.push_back(existing);

    controller_->ConvertAppInfosToCleanCacheInfo(appInfos, 200, result);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].bundleName, "existing.app");
    EXPECT_EQ(result[1].bundleName, "com.example.app");
    EXPECT_EQ(result[1].userId, 200);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertAppInfosToCleanCacheInfo_AppendToExisting end";
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SortBundleActive_0001
 * @tc.name: SortBundleStatsByInFrontTime_Empty
 * @tc.desc: Test SortBundleStatsByInFrontTime with empty vector
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SortBundleStatsByInFrontTime_Empty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SortBundleStatsByInFrontTime_Empty start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> stats;
    controller_->SortBundleStatsByInFrontTime(stats);
    EXPECT_TRUE(stats.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SortBundleStatsByInFrontTime_Empty end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_SortBundleActive_0004
 * @tc.name: SortBundleStatsByInFrontTime_ReverseSorted
 * @tc.desc: Test SortBundleStatsByInFrontTime with reverse sorted data
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, SortBundleStatsByInFrontTime_ReverseSorted, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SortBundleStatsByInFrontTime_ReverseSorted start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> stats;
    BundleActivePackageStats s1;
    s1.bundleName_ = "com.example.low";
    s1.totalInFrontTime_ = 50;
    stats.push_back(s1);

    BundleActivePackageStats s2;
    s2.bundleName_ = "com.example.high";
    s2.totalInFrontTime_ = 500;
    stats.push_back(s2);

    controller_->SortBundleStatsByInFrontTime(stats);
    ASSERT_EQ(stats.size(), 2);
    EXPECT_EQ(stats[0].bundleName_, "com.example.high");
    EXPECT_EQ(stats[1].bundleName_, "com.example.low");

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_SortBundleStatsByInFrontTime_ReverseSorted end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_FilterBundleStats_0001
 * @tc.name: FilterBundleStatsByAppType_BothEmpty
 * @tc.desc: Test FilterBundleStatsByAppType with both inputs empty
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, FilterBundleStatsByAppType_BothEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterBundleStatsByAppType_BothEmpty start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> bundleStats;
    std::vector<ApplicationInfo> appInfos;
    controller_->FilterBundleStatsByAppType(bundleStats, appInfos);
    EXPECT_TRUE(bundleStats.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterBundleStatsByAppType_BothEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_FilterBundleStats_0003
 * @tc.name: FilterBundleStatsByAppType_AppInfosEmpty
 * @tc.desc: Test FilterBundleStatsByAppType with bundleStats non-empty, appInfos empty
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, FilterBundleStatsByAppType_AppInfosEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterBundleStatsByAppType_AppInfosEmpty start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> bundleStats;
    BundleActivePackageStats s;
    s.bundleName_ = "com.example.test";
    s.appIndex_ = 0;
    bundleStats.push_back(s);

    std::vector<ApplicationInfo> appInfos;
    controller_->FilterBundleStatsByAppType(bundleStats, appInfos);
    EXPECT_TRUE(bundleStats.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterBundleStatsByAppType_AppInfosEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_FilterBundleStats_0005
 * @tc.name: FilterBundleStatsByAppType_PartialMatching
 * @tc.desc: Test FilterBundleStatsByAppType with partial matching
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, FilterBundleStatsByAppType_PartialMatching, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterBundleStatsByAppType_PartialMatching start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> bundleStats;
    BundleActivePackageStats s1;
    s1.bundleName_ = "com.example.matched";
    s1.appIndex_ = 0;
    bundleStats.push_back(s1);

    BundleActivePackageStats s2;
    s2.bundleName_ = "com.example.unmatched";
    s2.appIndex_ = 0;
    bundleStats.push_back(s2);

    std::vector<ApplicationInfo> appInfos;
    ApplicationInfo app;
    app.bundleName = "com.example.matched";
    app.appIndex = 0;
    appInfos.push_back(app);

    controller_->FilterBundleStatsByAppType(bundleStats, appInfos);
    ASSERT_EQ(bundleStats.size(), 1);
    EXPECT_EQ(bundleStats[0].bundleName_, "com.example.matched");

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_FilterBundleStatsByAppType_PartialMatching end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_RemoveAppsByStats_0001
 * @tc.name: RemoveAppsByStats_BothEmpty
 * @tc.desc: Test RemoveAppsByStats with both inputs empty
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, RemoveAppsByStats_BothEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_RemoveAppsByStats_BothEmpty start";
    ASSERT_NE(controller_, nullptr);

    std::vector<ApplicationInfo> appInfos;
    std::vector<BundleActivePackageStats> bundleStatsToRemove;
    auto result = controller_->RemoveAppsByStats(appInfos, bundleStatsToRemove);
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_RemoveAppsByStats_BothEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_RemoveBundleStats_0001
 * @tc.name: RemoveBundleStatsByBundleStats_BothEmpty
 * @tc.desc: Test RemoveBundleStatsByBundleStats with both inputs empty
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, RemoveBundleStatsByBundleStats_BothEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_RemoveBundleStatsByBundleStats_BothEmpty start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> bundleStats;
    std::vector<BundleActivePackageStats> bundleStatsToRemove;
    auto result = controller_->RemoveBundleStatsByBundleStats(bundleStats, bundleStatsToRemove);
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_RemoveBundleStatsByBundleStats_BothEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_ConvertStats_0001
 * @tc.name: ConvertBundleStatsToCleanCacheInfo_Empty
 * @tc.desc: Test ConvertBundleStatsToCleanCacheInfo with empty input
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, ConvertBundleStatsToCleanCacheInfo_Empty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertBundleStatsToCleanCacheInfo_Empty start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> bundleStats;
    std::vector<CleanCacheInfo> result;
    controller_->ConvertBundleStatsToCleanCacheInfo(bundleStats, 100, result);
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertBundleStatsToCleanCacheInfo_Empty end";
}

/**
 * @tc.number: SUB_STORAGE_CacheCleanController_ConvertStats_0003
 * @tc.name: ConvertBundleStatsToCleanCacheInfo_Multiple
 * @tc.desc: Test ConvertBundleStatsToCleanCacheInfo with multiple elements including different appIndex
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CacheCleanControllerTest, ConvertBundleStatsToCleanCacheInfo_Multiple, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertBundleStatsToCleanCacheInfo_Multiple start";
    ASSERT_NE(controller_, nullptr);

    std::vector<BundleActivePackageStats> bundleStats;
    BundleActivePackageStats s1;
    s1.bundleName_ = "com.example.app1";
    s1.appIndex_ = 0;
    bundleStats.push_back(s1);

    BundleActivePackageStats s2;
    s2.bundleName_ = "com.example.app2";
    s2.appIndex_ = 2;
    bundleStats.push_back(s2);

    std::vector<CleanCacheInfo> result;
    controller_->ConvertBundleStatsToCleanCacheInfo(bundleStats, 200, result);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].bundleName, "com.example.app1");
    EXPECT_EQ(result[0].userId, 200);
    EXPECT_EQ(result[0].appIndex, 0);
    EXPECT_EQ(result[1].bundleName, "com.example.app2");
    EXPECT_EQ(result[1].userId, 200);
    EXPECT_EQ(result[1].appIndex, 2);

    GTEST_LOG_(INFO) << "CacheCleanControllerTest_ConvertBundleStatsToCleanCacheInfo_Multiple end";
}
#endif // DEVICE_USAGE_STATISTICS_ENABLE

} // namespace StorageSpaceManager
} // namespace OHOS
