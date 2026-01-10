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
#include <memory>
#include <string>

#include "file_cache_adapter.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;

class FileCacheAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        // 创建测试目录
        std::filesystem::create_directories("/data/service/el1/public/storage_manager/database/");
    };

    static void TearDownTestCase()
    {
        // 清理测试文件
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/bundle_ext_stats.json");
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/bundle_ext_stats.json.tmp");
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/clean_notify.json");
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/clean_notify.json.tmp");
    };

    void SetUp() override
    {
        // 每次测试结束清理文件
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/bundle_ext_stats.json");
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/bundle_ext_stats.json.tmp");
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/clean_notify.json");
        std::filesystem::remove("/data/service/el1/public/storage_manager/database/clean_notify.json.tmp");

        // 重置单例状态
        FileCacheAdapter::GetInstance().UnInit();
    };

    void TearDown() override{};

private:
    static inline BundleExtStats stats;
    static inline CleanNotify cleanNotify;
};

/**
 * @tc.number: SUB_STORAGE_BundleExtStats_FromJson_0001
 * @tc.name: BundleExtStats_FromJson_0001
 * @tc.desc: Test JSON parsing with discarded JSON.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, BundleExtStats_FromJson_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0001 start";

    // 创建一个已丢弃的JSON对象
    nlohmann::json j = nlohmann::json::parse("{invalid json}", nullptr, false);

    bool ret = stats.FromJson(j);

    EXPECT_FALSE(ret);

    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_BundleExtStats_FromJson_0002
 * @tc.name: BundleExtStats_FromJson_0002
 * @tc.desc: Test JSON parsing when businessName is missing or not a string.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, BundleExtStats_FromJson_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0002 start";

    nlohmann::json j;
    // businessName 缺失
    j["businessSize"] = 1024UL;
    j["userId"] = 1000U;

    EXPECT_FALSE(stats.FromJson(j));

    nlohmann::json j2;
    j2["businessName"] = 123; // 不是字符串
    j2["businessSize"] = 1024UL;
    j2["userId"] = 1000U;

    EXPECT_FALSE(stats.FromJson(j2));

    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_BundleExtStats_FromJson_0003
 * @tc.name: BundleExtStats_FromJson_0003
 * @tc.desc: Test JSON parsing when businessSize is missing or not an unsigned integer.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, BundleExtStats_FromJson_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0003 start";

    nlohmann::json j;
    j["businessName"] = "test_business";
    // businessSize 缺失
    j["userId"] = 1000U;

    EXPECT_FALSE(stats.FromJson(j));

    nlohmann::json j2;
    j2["businessName"] = "test_business";
    j2["businessSize"] = -1024; // 负数，不是无符号整数
    j2["userId"] = 1000U;
    EXPECT_FALSE(stats.FromJson(j2));

    nlohmann::json j3;
    j3["businessName"] = "test_business";
    j3["businessSize"] = "not_a_number"; // 不是数字
    j3["userId"] = 1000U;

    EXPECT_FALSE(stats.FromJson(j3));

    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0003 end";
}

/**
 * @tc.number: SUB_STORAGE_BundleExtStats_FromJson_0004
 * @tc.name: BundleExtStats_FromJson_0004
 * @tc.desc: Test JSON parsing when userId is missing or not an unsigned integer.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, BundleExtStats_FromJson_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0004 start";

    nlohmann::json j;
    j["businessName"] = "test_business";
    j["businessSize"] = 1024UL;
    j["userId"] = -1000; // 负数，不是无符号整数

    EXPECT_FALSE(stats.FromJson(j));

    nlohmann::json j2;
    j2["businessName"] = "test_business";
    j2["businessSize"] = 1024UL;
    // userId 缺失
    EXPECT_FALSE(stats.FromJson(j2));

    nlohmann::json j3;
    j3["businessName"] = "test_business";
    j3["businessSize"] = 1024UL;
    j3["userId"] = "not_a_number"; // 不是数字
    EXPECT_FALSE(stats.FromJson(j3));

    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0004 end";
}

/**
 * @tc.number: SUB_STORAGE_BundleExtStats_FromJson_0005
 * @tc.name: BundleExtStats_FromJson_0005
 * @tc.desc: Test JSON parsing when bundleName is missing or not a string.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, BundleExtStats_FromJson_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0005 start";

    nlohmann::json j;
    j["businessName"] = "test_business";
    j["businessSize"] = 1024UL;
    j["userId"] = 1000U;
    // 不包含 bundleName，根据代码逻辑应该会失败
    EXPECT_FALSE(stats.FromJson(j));

    nlohmann::json j2;
    j2["businessName"] = "test_business";
    j2["businessSize"] = 1024UL;
    j2["userId"] = 1000U;
    j2["bundleName"] = 123; // 不是字符串
    EXPECT_FALSE(stats.FromJson(j2));

    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0005 end";
}

/**
 * @tc.number: SUB_STORAGE_BundleExtStats_FromJson_0006
 * @tc.name: BundleExtStats_FromJson_0006
 * @tc.desc: Test JSON parsing when lastModifyTime is missing or not an unsigned integer.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, BundleExtStats_FromJson_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0006 start";

    nlohmann::json j;
    j["businessName"] = "test_business";
    j["businessSize"] = 1024UL;
    j["userId"] = 1000U;
    j["bundleName"] = "com.test.app";
    // 不包含 lastModifyTime，根据代码逻辑应该会失败
    EXPECT_FALSE(stats.FromJson(j));

    nlohmann::json j2;
    j2["businessName"] = "test_business";
    j2["businessSize"] = 1024UL;
    j2["userId"] = 1000U;
    j2["bundleName"] = "com.test.app";
    j2["lastModifyTime"] = -1234567890; // 负数，不是无符号整数
    EXPECT_FALSE(stats.FromJson(j2));

    nlohmann::json j3;
    j3["businessName"] = "test_business";
    j3["businessSize"] = 1024UL;
    j3["userId"] = 1000U;
    j3["bundleName"] = "com.test.app";
    j3["lastModifyTime"] = "not_a_number"; // 不是数字
    EXPECT_FALSE(stats.FromJson(j3));

    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0006 end";
}

/**
 * @tc.number: SUB_STORAGE_BundleExtStats_FromJson_0007
 * @tc.name: BundleExtStats_FromJson_0007
 * @tc.desc: Test JSON parsing when showFlag is missing or not a boolean.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, BundleExtStats_FromJson_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0007 start";

    nlohmann::json j;
    j["businessName"] = "test_business";
    j["businessSize"] = 1024UL;
    j["userId"] = 1000U;
    j["bundleName"] = "com.test.app";
    j["lastModifyTime"] = 1234567890UL;
    // 不包含 showFlag，根据代码逻辑应该会失败
    EXPECT_FALSE(stats.FromJson(j));

    nlohmann::json j2;
    j2["businessName"] = "test_business";
    j2["businessSize"] = 1024UL;
    j2["userId"] = 1000U;
    j2["bundleName"] = "com.test.app";
    j2["lastModifyTime"] = 1234567890UL;
    j2["showFlag"] = "not_a_boolean"; // 不是布尔值
    EXPECT_FALSE(stats.FromJson(j2));

    nlohmann::json j3;
    j3["businessName"] = "test_business";
    j3["businessSize"] = 1024UL;
    j3["userId"] = 1000U;
    j3["bundleName"] = "com.test.app";
    j3["lastModifyTime"] = 1234567890UL;
    j3["showFlag"] = true;
    EXPECT_TRUE(stats.FromJson(j3));

    GTEST_LOG_(INFO) << "BundleExtStats_FromJson_0007 end";
}

/**
 * @tc.number: SUB_STORAGE_CleanNotify_FromJson_0001
 * @tc.name: CleanNotify_FromJson_0001
 * @tc.desc: Test JSON parsing with discarded JSON.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, CleanNotify_FromJson_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotify_FromJson_0001 start";

    // 创建一个已丢弃的JSON对象
    nlohmann::json j = nlohmann::json::parse("{invalid json}", nullptr, false);

    CleanNotify notify;
    bool ret = cleanNotify.FromJson(j);

    EXPECT_FALSE(ret);

    GTEST_LOG_(INFO) << "CleanNotify_FromJson_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_CleanNotify_FromJson_0002
 * @tc.name: CleanNotify_FromJson_0002
 * @tc.desc: Test JSON parsing when cleanLevelName is missing or not a string.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, CleanNotify_FromJson_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotify_FromJson_0002 start";

    nlohmann::json j;
    // cleanLevelName 缺失
    j["lastCleanNotifyTime"] = 1234567890UL;

    CleanNotify notify;
    EXPECT_FALSE(cleanNotify.FromJson(j));

    nlohmann::json j2;
    j2["cleanLevelName"] = 123; // 不是字符串
    j2["lastCleanNotifyTime"] = 1234567890UL;

    EXPECT_FALSE(cleanNotify.FromJson(j2));

    GTEST_LOG_(INFO) << "CleanNotify_FromJson_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_CleanNotify_FromJson_0003
 * @tc.name: CleanNotify_FromJson_0003
 * @tc.desc: Test JSON parsing when lastCleanNotifyTime is missing or not an integer.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, CleanNotify_FromJson_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotify_FromJson_0003 start";

    nlohmann::json j;
    j["cleanLevelName"] = "LEVEL_1";
    // lastCleanNotifyTime 缺失
    EXPECT_FALSE(cleanNotify.FromJson(j));

    nlohmann::json j2;
    j2["cleanLevelName"] = "LEVEL_1";
    j2["lastCleanNotifyTime"] = "not_an_integer"; // 不是整数
    EXPECT_FALSE(cleanNotify.FromJson(j2));

    nlohmann::json j3;
    j3["cleanLevelName"] = "LEVEL_1";
    j3["lastCleanNotifyTime"] = 1234567890UL;

    EXPECT_TRUE(cleanNotify.FromJson(j3));
    EXPECT_EQ(cleanNotify.cleanLevelName, "LEVEL_1");
    EXPECT_EQ(cleanNotify.lastCleanNotifyTime, 1234567890UL);

    GTEST_LOG_(INFO) << "CleanNotify_FromJson_0003 end";
}

/**
 * @tc.number: SUB_STORAGE_file_cache_adapter_UnInit_0001
 * @tc.name: file_cache_adapter_UnInit_0001
 * @tc.desc: Test uninitialization when not initialized.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, file_cache_adapter_UnInit_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "file_cache_adapter_UnInit_0001 start";

    auto &cacheAdapter = FileCacheAdapter::GetInstance();

    // 直接调用 UnInit（未初始化）
    EXPECT_EQ(cacheAdapter.UnInit(), E_OK);

    cacheAdapter.initialized_ = true;
    EXPECT_EQ(cacheAdapter.UnInit(), E_OK);

    cacheAdapter.initialized_ = true;
    cacheAdapter.bundleDirty_ = true;
    EXPECT_EQ(cacheAdapter.UnInit(), E_OK);

    cacheAdapter.initialized_ = true;
    cacheAdapter.bundleDirty_ = true;
    cacheAdapter.cleanDirty_ = true;
    EXPECT_EQ(cacheAdapter.UnInit(), E_OK);

    GTEST_LOG_(INFO) << "file_cache_adapter_UnInit_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_file_cache_adapter_InsertOrUpdate_0001
 * @tc.name: file_cache_adapter_InsertOrUpdate_0001
 * @tc.desc: Test uninitialization when not initialized.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, file_cache_adapter_InsertOrUpdate_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "file_cache_adapter_InsertOrUpdate_0001 start";

    auto &cacheAdapter = FileCacheAdapter::GetInstance();

    // 直接调用 UnInit（未初始化）
    EXPECT_EQ(cacheAdapter.InsertOrUpdateBundleExtStats(stats), E_OK);

    EXPECT_EQ(cacheAdapter.InsertOrUpdateCleanNotify(cleanNotify), E_OK);

    GTEST_LOG_(INFO) << "file_cache_adapter_InsertOrUpdate_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_file_cache_adapter_DeleteBundleExtStats_0001
 * @tc.name: file_cache_adapter_DeleteBundleExtStats_0001
 * @tc.desc: Test DeleteBundleExtStats function.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(FileCacheAdapterTest, file_cache_adapter_DeleteBundleExtStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "file_cache_adapter_DeleteBundleExtStats_0001 start";

    auto &cacheAdapter = FileCacheAdapter::GetInstance();

    // 初始化
    int32_t ret = cacheAdapter.Init();
    EXPECT_EQ(ret, E_OK);

    // 删除不存在的数据
    ret = cacheAdapter.DeleteBundleExtStats("non_existing_app", 9999U);
    EXPECT_EQ(ret, E_OK); // 应该返回成功，删除不存在的记录返回 E_OK

    // 插入一条数据
    BundleExtStats stats;
    stats.businessName = "business1";
    stats.businessSize = 1024U;
    stats.userId = 1000U;
    stats.bundleName = "com.test.app1";
    stats.lastModifyTime = 1234567890UL;
    stats.showFlag = true;

    ret = cacheAdapter.InsertOrUpdateBundleExtStats(stats);
    EXPECT_EQ(ret, E_OK);

    // 验证插入成功
    auto result = cacheAdapter.GetBundleExtStats("business1", 1000U);
    EXPECT_NE(result, nullptr);

    // 删除这条数据
    ret = cacheAdapter.DeleteBundleExtStats("com.test.app1", 1000U);
    EXPECT_EQ(ret, E_OK);

    // 验证数据已被删除
    result = cacheAdapter.GetBundleExtStats("business1", 1000U);
    EXPECT_EQ(result, nullptr);


    GTEST_LOG_(INFO) << "file_cache_adapter_DeleteBundleExtStats_0001 end";
}

} // namespace StorageManager
} // namespace OHOS