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

#include "storage_rdb_adapter_mock.h"

#include <memory>
#include <mutex>
#include <string>
#include <chrono>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "storage_rdb_adapter.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::NativeRdb;

namespace {
    const std::string TAG = "rdbAdapterTest";
    const std::string TEST_TABLE_NAME = "bundle_ext_stats_table_test";
    const std::string SUCCESS_CREATE_TABLE_SQL = "CREATE TABLE IF NOT EXISTS " + TEST_TABLE_NAME + " \
    ( \
        businessName               TEXT NOT NULL, \
        businessSize               LONG NOT NULL, \
        userId                     INTEGER NOT NULL DEFAULT 0, \
        bundleName                 TEXT NOT NULL, \
        lastModifyTime             INTEGER NOT NULL, \
        PRIMARY KEY (businessName, userId) \
    );";
    const std::string FIELD_BUSINESS_NAME = "businessName";
    const std::string FIELD_BUSINESS_SIZE = "businessSize";
    const std::string FIELD_USER_ID = "userId";
    const std::string FIELD_BUNDLE_NAME = "bundleName";
    const std::string FIELD_LAST_MODIFY_TIME = "lastModifyTime";
    const std::string WHERE_CLAUSE_BUSINESS_USER = FIELD_BUSINESS_NAME + " = ? AND " + FIELD_USER_ID + " = ?";
    const std::string TEST_BUSINESS_NAME = "businessName";
    const int64_t TEST_BUSINESS_SIZE = 123;
    const int32_t TEST_USER_ID = 100;
    const std::string TEST_BUNDLE_NAME = "bundleName";
    const int64_t TEST_LAST_MODIFY_TIME = 1234;
    const std::string TEST_QUERY_BUSINESS_NAME = "test_business_common";
    const int32_t TEST_QUERY_USER_ID = 10001;
    const int64_t TEST_UPDATE_BUSINESS_SIZE = 2048 * 512;
    const std::string TEST_UPDATE_BUNDLE_NAME = "com.storage.test.update";
    const int32_t TEST_UPDATE_CHANGED_ROWS_SUCCESS = 1;
    const std::string GET_SQL =
        "SELECT " + FIELD_BUSINESS_NAME + ", "
        "       " + FIELD_BUSINESS_SIZE + ", "
        "       " + FIELD_USER_ID + ", "
        "       " + FIELD_BUNDLE_NAME + ", "
        "       " + FIELD_LAST_MODIFY_TIME + " "
        "FROM " + TEST_TABLE_NAME + " "
        "WHERE " + WHERE_CLAUSE_BUSINESS_USER;
    const int32_t TEST_DB_OLD_VERSION = 1;
    const int32_t TEST_DB_NEW_VERSION = 2;
}

class RdbAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<MockRdbStore> mockStore;
    std::mutex rdbAdapterTestMtx;
};

void RdbAdapterTest::SetUpTestCase(void) {}

void RdbAdapterTest::TearDownTestCase(void) {}

void RdbAdapterTest::SetUp()
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    mockStore = std::make_shared<MockRdbStore>();
    MockGetRdbStore(mockStore);

    int32_t ret = rdbAdapter.Init();
    ASSERT_EQ(ret, OHOS::E_OK);
    rdbAdapter.store_ = mockStore;
}

void RdbAdapterTest::TearDown()
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    StorageRdbAdapter::GetInstance().UnInit();
}

/**
 * @tc.name: UnInit001
 * @tc.desc: UnInit success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, UnInit001, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    auto errCode = StorageRdbAdapter::GetInstance().UnInit();
    EXPECT_EQ(errCode, OHOS::E_OK);
}

/**
 * @tc.name: Put001
 * @tc.desc: Put Success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Put001, TestSize.Level1)
{
    EXPECT_CALL(*mockStore, Insert(_, _, _)).WillOnce(Return(OHOS::E_OK));
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    int64_t outRowId = 0;
    ValuesBucket values;
    values.PutString(FIELD_BUSINESS_NAME, TEST_BUSINESS_NAME);
    values.PutLong(FIELD_BUSINESS_SIZE, TEST_BUSINESS_SIZE);
    values.PutInt(FIELD_USER_ID, TEST_USER_ID);
    values.PutString(FIELD_BUNDLE_NAME, TEST_BUNDLE_NAME);
    values.PutLong(FIELD_LAST_MODIFY_TIME, TEST_LAST_MODIFY_TIME);

    int32_t putErrCode = StorageRdbAdapter::GetInstance().Put(outRowId, TEST_TABLE_NAME, values);
    EXPECT_EQ(putErrCode, OHOS::E_OK);
}

/**
 * @tc.name: Put002
 * @tc.desc: Test the insert operation; expected to fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Put002, TestSize.Level1)
{
    EXPECT_CALL(*mockStore, Insert(_, _, _)).Times(1).WillOnce(Return(E_ERR));
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    int64_t outRowId = 0;
    ValuesBucket values;
    values.PutString(FIELD_BUSINESS_NAME, TEST_BUSINESS_NAME);
    values.PutLong(FIELD_BUSINESS_SIZE, TEST_BUSINESS_SIZE);
    values.PutInt(FIELD_USER_ID, TEST_USER_ID);
    values.PutString(FIELD_BUNDLE_NAME, TEST_BUNDLE_NAME);
    values.PutLong(FIELD_LAST_MODIFY_TIME, TEST_LAST_MODIFY_TIME);

    int32_t putErrCode = StorageRdbAdapter::GetInstance().Put(outRowId, TEST_TABLE_NAME, values);
    EXPECT_EQ(putErrCode, E_TB_PUT_ERROR);
}

/**
 * @tc.name: Put003
 * @tc.desc: Put failed, RDBStore_ is null.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Put003, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    int64_t outRowId = 0;
    ValuesBucket values;
    values.PutString(FIELD_BUSINESS_NAME, TEST_BUSINESS_NAME);
    values.PutLong(FIELD_BUSINESS_SIZE, TEST_BUSINESS_SIZE);
    values.PutInt(FIELD_USER_ID, TEST_USER_ID);
    values.PutString(FIELD_BUNDLE_NAME, TEST_BUNDLE_NAME);
    values.PutLong(FIELD_LAST_MODIFY_TIME, TEST_LAST_MODIFY_TIME);

    StorageRdbAdapter::GetInstance().UnInit();
    int32_t putErrCode = StorageRdbAdapter::GetInstance().Put(outRowId, TEST_TABLE_NAME, values);
    EXPECT_EQ(putErrCode, E_RDB_STORE_NULL);
}

/**
 * @tc.name: Delete001
 * @tc.desc: Delete success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Delete001, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    int32_t deleteRows = 0;
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    EXPECT_CALL(*mockStore, Delete(::testing::Ref(deleteRows), ::testing::Eq(TEST_TABLE_NAME),
        ::testing::Eq(WHERE_CLAUSE_BUSINESS_USER), ::testing::Eq(bindArgs))).WillOnce(Return(OHOS::E_OK));
    int32_t deleteErrCode = StorageRdbAdapter::GetInstance().Delete(deleteRows, TEST_TABLE_NAME,
        WHERE_CLAUSE_BUSINESS_USER, bindArgs);
    EXPECT_EQ(deleteErrCode, OHOS::E_OK);
}

/**
 * @tc.name: Delete002
 * @tc.desc: Delete table does not exist failed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Delete002, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    int32_t deleteRows = 0;
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    EXPECT_CALL(*mockStore, Delete(::testing::Ref(deleteRows), ::testing::Eq(TEST_TABLE_NAME),
        ::testing::Eq(WHERE_CLAUSE_BUSINESS_USER), ::testing::Eq(bindArgs))).Times(1).WillOnce(Return(E_ERR));
    
    int32_t deleteErrCode = StorageRdbAdapter::GetInstance().Delete(deleteRows, TEST_TABLE_NAME,
        WHERE_CLAUSE_BUSINESS_USER, bindArgs);
    EXPECT_EQ(deleteErrCode, E_TB_DELETE_ERROR);
}

/**
 * @tc.name: Delete003
 * @tc.desc: Delete failed, RDBStore_ is null.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Delete003, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    int32_t deleteRows = 0;
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    StorageRdbAdapter::GetInstance().UnInit();
    int32_t deleteErrCode = StorageRdbAdapter::GetInstance().Delete(deleteRows, TEST_TABLE_NAME,
        WHERE_CLAUSE_BUSINESS_USER, bindArgs);
    EXPECT_EQ(deleteErrCode, E_RDB_STORE_NULL);
}

/**
 * @tc.name: Update001
 * @tc.desc: Update success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Update001, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    ValuesBucket newValues;
    newValues.PutLong(FIELD_BUSINESS_SIZE, TEST_UPDATE_BUSINESS_SIZE);
    newValues.PutString(FIELD_BUNDLE_NAME, TEST_UPDATE_BUNDLE_NAME);
    const int64_t newModifyTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    newValues.PutInt(FIELD_LAST_MODIFY_TIME, newModifyTime);

    int32_t changedRows = 0;
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    EXPECT_CALL(*mockStore, Update(::testing::Ref(changedRows), ::testing::Eq(TEST_TABLE_NAME), _,
        ::testing::Eq(WHERE_CLAUSE_BUSINESS_USER),
        ::testing::Eq(bindArgs))).WillOnce(DoAll(SetArgReferee<0>(TEST_UPDATE_CHANGED_ROWS_SUCCESS),
        Return(OHOS::E_OK)));
    auto updateErrCode = StorageRdbAdapter::GetInstance().Update(changedRows, TEST_TABLE_NAME, newValues,
        WHERE_CLAUSE_BUSINESS_USER, bindArgs);
    EXPECT_EQ(updateErrCode, OHOS::E_OK);
}

/**
 * @tc.name: Update002
 * @tc.desc: Update table does not exist failed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Update002, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    ValuesBucket newValues;
    newValues.PutLong(FIELD_BUSINESS_SIZE, TEST_UPDATE_BUSINESS_SIZE);
    newValues.PutString(FIELD_BUNDLE_NAME, TEST_UPDATE_BUNDLE_NAME);
    const int64_t newModifyTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    newValues.PutInt(FIELD_LAST_MODIFY_TIME, newModifyTime);

    int32_t changedRows = 0;
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    EXPECT_CALL(*mockStore, Update(::testing::Ref(changedRows), ::testing::Eq(TEST_TABLE_NAME), _,
        ::testing::Eq(WHERE_CLAUSE_BUSINESS_USER), ::testing::Eq(bindArgs))).Times(1).WillOnce(Return(E_ERR));
    auto updateErrCode = StorageRdbAdapter::GetInstance().Update(changedRows, TEST_TABLE_NAME, newValues,
        WHERE_CLAUSE_BUSINESS_USER, bindArgs);
    EXPECT_EQ(updateErrCode, E_TB_UPDATE_ERROR);
}

/**
 * @tc.name: Update003
 * @tc.desc: Update failed, RDBStore_ is null.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Update003, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    ValuesBucket newValues;
    newValues.PutLong(FIELD_BUSINESS_SIZE, TEST_UPDATE_BUSINESS_SIZE);
    newValues.PutString(FIELD_BUNDLE_NAME, TEST_UPDATE_BUNDLE_NAME);
    const int64_t newModifyTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    newValues.PutInt(FIELD_LAST_MODIFY_TIME, newModifyTime);

    int32_t changedRows = 0;
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    StorageRdbAdapter::GetInstance().UnInit();
    auto updateErrCode = StorageRdbAdapter::GetInstance().Update(changedRows, TEST_TABLE_NAME, newValues,
        WHERE_CLAUSE_BUSINESS_USER, bindArgs);
    EXPECT_EQ(updateErrCode, E_RDB_STORE_NULL);
}

/**
 * @tc.name: Get001
 * @tc.desc: Get success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Get001, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    std::shared_ptr<MockResultSet> mockResultSet = std::make_shared<MockResultSet>();
    EXPECT_CALL(*mockStore, QueryByStep(::testing::Eq(GET_SQL), ::testing::Eq(bindArgs),
        _)).WillOnce(Return(mockResultSet));

    std::shared_ptr<ResultSet> resultSet = StorageRdbAdapter::GetInstance().Get(GET_SQL, bindArgs);
    EXPECT_NE(resultSet, nullptr);
}

/**
 * @tc.name: Get002
 * @tc.desc: Get failed, RDBStore_ is null.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, Get002, TestSize.Level1)
{
    std::lock_guard<std::mutex> lock(rdbAdapterTestMtx);
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };

    StorageRdbAdapter::GetInstance().UnInit();
    std::shared_ptr<ResultSet> resultSet = StorageRdbAdapter::GetInstance().Get(GET_SQL, bindArgs);
    EXPECT_EQ(resultSet, nullptr);
}

/**
 * @tc.name: OnCreate001
 * @tc.desc: OnCreate failed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, OnCreate001, TestSize.Level1)
{
    StorageManager::OpenCallback helper;
    EXPECT_CALL(*mockStore, ExecuteSql(_, _)).WillOnce(Return(NativeRdb::E_SQLITE_CORRUPT));
    auto ret = helper.OnCreate(*(RdbAdapterTest::mockStore));
    EXPECT_NE(ret, OHOS::E_OK);
}

/**
 * @tc.name: OnCreate002
 * @tc.desc: OnCreate success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, OnCreate002, TestSize.Level1)
{
    StorageManager::OpenCallback helper;
    EXPECT_CALL(*mockStore, ExecuteSql(_, _)).WillOnce(Return(OHOS::E_OK)).WillOnce(Return(OHOS::E_OK));
    auto ret = helper.OnCreate(*(RdbAdapterTest::mockStore));
    EXPECT_EQ(ret, OHOS::E_OK);
}

/**
 * @tc.name: OnUpgrade001
 * @tc.desc: OnUpgrade success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RdbAdapterTest, OnUpgrade001, TestSize.Level1)
{
    StorageManager::OpenCallback helper;
    auto ret = helper.OnUpgrade(*(RdbAdapterTest::mockStore), TEST_DB_OLD_VERSION, TEST_DB_NEW_VERSION);
    EXPECT_EQ(ret, OHOS::E_OK);
}
} // namespace StorageManager
} // namespace OHOS