/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <memory>
#include <mutex>
#include <string>

#include "gtest/gtest.h"
#include "storage_rdb_adapter.h"

namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::NativeRdb;

class RdbAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Storage_rdb_adapter_Put_0000
 * @tc.name: Storage_rdb_adapter_Put_0000
 * @tc.desc: Test for successful single call of the Put interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(RdbAdapterTest, Storage_rdb_adapter_Put_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Put_0000 start";
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    int32_t ret = rdbAdapter.Init();
    EXPECT_EQ(ret, E_OK);

    std::string table = "bundle_ext_stats_table";
    const std::string businessName = "test_business_Put_000";
    const int32_t businessSize = 1024 * 512;
    const int32_t userId = 10001;
    const std::string bundleName = "com.storage.test";
    const int64_t lastModifyTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    ValuesBucket values;
    values.PutString("businessName", businessName);
    values.PutLong("businessSize", businessSize);
    values.PutInt("userId", userId);
    values.PutString("bundleName", bundleName);
    values.PutLong("lastModifyTime", lastModifyTime);
    values.PutInt("showFlag", 1);

    int64_t rowId = 0;
    ret = rdbAdapter.Put(rowId, table, values);
    EXPECT_EQ(ret, E_OK);
    EXPECT_GT(rowId, 0);

    int32_t deleteRows = 0;
    std::string whereClause = "businessName = ? AND userId = ?";
    std::vector<ValueObject> bindArgs = {ValueObject(businessName), ValueObject(userId)};
    ret = rdbAdapter.Delete(deleteRows, table, whereClause, bindArgs);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(deleteRows, 1);

    rdbAdapter.UnInit();
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Put_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Storage_rdb_adapter_Put_0001
 * @tc.name: Storage_rdb_adapter_Put_0001
 * @tc.desc: Test for failure caused by multiple Put interface calls with duplicate data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(RdbAdapterTest, Storage_rdb_adapter_Put_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Put_0001 start";
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    int32_t ret = rdbAdapter.Init();
    EXPECT_EQ(ret, E_OK);

    const std::string table = "bundle_ext_stats_table";
    const std::string businessName = "test_business_Put_001";
    const int32_t userId = 10001;
    ValuesBucket values;
    values.PutString("businessName", businessName);
    values.PutLong("businessSize", 1024 * 512);
    values.PutInt("userId", userId);
    values.PutString("bundleName", "com.storage.test");
    values.PutLong("lastModifyTime", std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    values.PutInt("showFlag", 1);

    int64_t rowId = 0;
    ret = rdbAdapter.Put(rowId, table, values);
    EXPECT_EQ(ret, E_OK);
    EXPECT_GT(rowId, 0);

    for (int i = 0; i < 2; ++i) {
        rowId = 0;
        ret = rdbAdapter.Put(rowId, table, values);
        EXPECT_NE(ret, E_OK);
        EXPECT_EQ(rowId, 0);
    }

    int32_t deleteRows = 0;
    ret = rdbAdapter.Delete(deleteRows, table, "businessName=? AND userId=?",
                            {ValueObject(businessName), ValueObject(userId)});
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(deleteRows, 1);
    rdbAdapter.UnInit();
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Put_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_Storage_rdb_adapter_Update_0000
 * @tc.name: Storage_rdb_adapter_Update_0000
 * @tc.desc: Test for failure caused by multiple Put interface calls with duplicate data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(RdbAdapterTest, Storage_rdb_adapter_Update_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Update_0000 start";
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    int32_t ret = rdbAdapter.Init();
    EXPECT_EQ(ret, E_OK);

    const std::string table = "bundle_ext_stats_table",
                    bsn = "test_business_update_000",
                    origBundle = "com.storage.test.original",
                    updBundle = "com.storage.test.updated";
    const int32_t uid = 10002, origSize = 1024 * 256, updSize = 1024 * 1024;
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    int64_t rowId = 0;
    ValuesBucket insertVals;
    insertVals.PutString("businessName", bsn);
    insertVals.PutLong("businessSize", origSize);
    insertVals.PutInt("userId", uid);
    insertVals.PutString("bundleName", origBundle);
    insertVals.PutLong("lastModifyTime", now);
    insertVals.PutInt("showFlag", 1);
    ret = rdbAdapter.Put(rowId, table, insertVals);
    EXPECT_EQ(ret, E_OK);
    EXPECT_GT(rowId, 0);

    int32_t changedRows = 0;
    const std::string where = "businessName=? AND userId=?";
    const std::vector<ValueObject> args = {bsn, uid};
    ValuesBucket updateVals;
    updateVals.PutLong("businessSize", updSize);
    updateVals.PutString("bundleName", updBundle);
    updateVals.PutLong("lastModifyTime", now);
    updateVals.PutInt("showFlag", 1);
    ret = rdbAdapter.Update(changedRows, table, updateVals, where, args);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(changedRows, 1);

    int32_t delRows = 0;
    ret = rdbAdapter.Delete(delRows, table, where, args);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(delRows, 1);

    rdbAdapter.UnInit();
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Update_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Storage_rdb_adapter_Get_0000
 * @tc.name: Storage_rdb_adapter_Get_0000
 * @tc.desc: Test the function of the Get interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(RdbAdapterTest, Storage_rdb_adapter_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Get_0000 start";
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    int32_t ret = rdbAdapter.Init();
    EXPECT_EQ(ret, E_OK);

    std::string table = "bundle_ext_stats_table";
    const std::string businessName = "test_business_get_000";
    const int32_t businessSize = 1024 * 512;
    const int32_t userId = 10001;
    const std::string bundleName = "com.storage.test";
    const int64_t lastModifyTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    ValuesBucket values;
    values.PutString("businessName", businessName);
    values.PutLong("businessSize", businessSize);
    values.PutInt("userId", userId);
    values.PutString("bundleName", bundleName);
    values.PutLong("lastModifyTime", lastModifyTime);
    values.PutInt("showFlag", 1);

    int64_t rowId = 0;
    ret = rdbAdapter.Put(rowId, table, values);
    EXPECT_EQ(ret, E_OK);
    EXPECT_GT(rowId, 0);

    std::string querySql = "SELECT businessName, businessSize, userId, bundleName "
                           "FROM bundle_ext_stats_table "
                           "WHERE businessName = ? AND userId = ?";
    auto resultSet = rdbAdapter.Get(querySql, {ValueObject(businessName), ValueObject(userId)});
    ASSERT_NE(resultSet, nullptr);
    ASSERT_EQ(resultSet->GoToFirstRow(), E_OK);
    std::string actBizName, actBundleName;
    int32_t actBizSize = 0, actUserId = 0;
    bool parseOk = (resultSet->GetString(0, actBizName) == E_OK)
                && (resultSet->GetInt(1, actBizSize) == E_OK)
                && (resultSet->GetInt(2, actUserId) == E_OK)
                && (resultSet->GetString(3, actBundleName) == E_OK);
    EXPECT_TRUE(parseOk);
    EXPECT_EQ(actBizName, businessName);
    EXPECT_EQ(actBizSize, businessSize);
    EXPECT_EQ(actUserId, userId);
    EXPECT_EQ(actBundleName, bundleName);
    resultSet->Close();
    std::string whereClause = "businessName = ? AND userId = ?";
    std::vector<ValueObject> bindArgs = {ValueObject(businessName), ValueObject(userId)};
    int32_t deleteRows = 0;
    ret = rdbAdapter.Delete(deleteRows, table, whereClause, bindArgs);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(deleteRows, 1);

    rdbAdapter.UnInit();
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Get_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Storage_rdb_adapter_Delete_0000
 * @tc.name: Storage_rdb_adapter_Delete_0000
 * @tc.desc: Test the function of the Delete interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(RdbAdapterTest, Storage_rdb_adapter_Delete_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Delete_0000 start";
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    int32_t ret = rdbAdapter.Init();
    EXPECT_EQ(ret, E_OK);

    std::string table = "bundle_ext_stats_table";
    const std::string businessName = "test_business_delete_000";
    const int32_t businessSize = 1024 * 512;
    const int32_t userId = 10001;
    const std::string bundleName = "com.storage.test";
    const int64_t lastModifyTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    ValuesBucket values;
    values.PutString("businessName", businessName);
    values.PutLong("businessSize", businessSize);
    values.PutInt("userId", userId);
    values.PutString("bundleName", bundleName);
    values.PutLong("lastModifyTime", lastModifyTime);
    values.PutInt("showFlag", 1);

    int64_t rowId = 0;
    ret = rdbAdapter.Put(rowId, table, values);
    EXPECT_EQ(ret, E_OK);
    EXPECT_GT(rowId, 0);

    std::string whereClause = "businessName = ? AND userId = ?";
    std::vector<ValueObject> bindArgs = {ValueObject(businessName), ValueObject(userId)};
    int32_t deleteRows = 0;
    ret = rdbAdapter.Delete(deleteRows, table, whereClause, bindArgs);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(deleteRows, 1);

    rdbAdapter.UnInit();
    GTEST_LOG_(INFO) << "Storage_rdb_adapter_Delete_0000 end";
}

} // namespace StorageManager
} // namespace OHOS