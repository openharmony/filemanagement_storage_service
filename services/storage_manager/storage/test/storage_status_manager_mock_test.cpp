/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <gtest/gtest.h>

#include "storage_rdb_adapter_mock.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_status_manager.h"
#include "storage_service_errno.h"
#include <storage/storage_total_status_service.h>
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "storage_rdb_adapter.h"
#include "gmock/gmock.h"
#include "ext_bundle_stats.h"

namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return Security::AccessToken::TOKEN_HAP;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
{
    nativeTokenInfoRes.processName = "foundation";
    return 0;
}
}

namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::NativeRdb;

const std::string CALLING_BUNDLE_NAME = "callingBundleName";
uint64_t TEST_BUSINESS_SIZE = 123;
const std::string TEST_QUERY_BUSINESS_NAME = "test_business_common";
int32_t ROW_COUNT = 1;
std::string DB_BUNDLE_NAME = "dbBundleName";
const int32_t TEST_QUERY_USER_ID = 10001;
const std::string GET_SQL = "SELECT * FROM bundle_ext_stats_table WHERE businessName = ? AND userId = ? LIMIT 1";
const ExtBundleStats EXT_BUNDLE_STATS;
const int MAX_USER_ID = 10738;
const int MAX_APP_INDEX = 5;

class StorageStatusManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<MockRdbStore> mockStore;
};

void StorageStatusManagerTest::SetUpTestCase(void) {}

void StorageStatusManagerTest::TearDownTestCase(void) {}

void StorageStatusManagerTest::SetUp()
{
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    mockStore = std::make_shared<MockRdbStore>();
    MockGetRdbStore(mockStore);

    int32_t ret = rdbAdapter.Init();
    ASSERT_EQ(ret, OHOS::E_OK);
    rdbAdapter.store_ = mockStore;
}

void StorageStatusManagerTest::TearDown()
{
    MockSetRdbStore();
    mockStore = nullptr;
    StorageRdbAdapter::GetInstance().UnInit();
}

/**
 * @tc.number: STORAGE_GetBundleNameFromDB_00001
 * @tc.name: STORAGE_GetBundleNameFromDB_00001
 * @tc.desc: Test function of the GetRowCount function in the GetBundleNameFromDB interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleNameFromDB_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    std::shared_ptr<MockResultSet> mockResultSet = std::make_shared<MockResultSet>();
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };
    EXPECT_CALL(*mockStore, QueryByStep(::testing::Eq(GET_SQL), ::testing::Eq(bindArgs), true))
        .WillOnce(Return(mockResultSet));
    EXPECT_CALL(*mockResultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(ROW_COUNT), Return(E_ERR)));
    int32_t ret = service->GetBundleNameFromDB(TEST_QUERY_USER_ID, TEST_QUERY_BUSINESS_NAME, DB_BUNDLE_NAME, ROW_COUNT);
    EXPECT_NE(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB end";
}

/**
 * @tc.number: STORAGE_GetBundleNameFromDB_00002
 * @tc.name: STORAGE_GetBundleNameFromDB_00002
 * @tc.desc: Test function of the GoToFirstRow function in the GetBundleNameFromDB interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleNameFromDB_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    std::shared_ptr<MockResultSet> mockResultSet = std::make_shared<MockResultSet>();
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };
    EXPECT_CALL(*mockStore, QueryByStep(::testing::Eq(GET_SQL), ::testing::Eq(bindArgs), true))
        .WillOnce(Return(mockResultSet));
    EXPECT_CALL(*mockResultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(ROW_COUNT), Return(OHOS::E_OK)));
    EXPECT_CALL(*mockResultSet, GoToFirstRow()).WillOnce(Return(E_ERR));
    int32_t ret = service->GetBundleNameFromDB(TEST_QUERY_USER_ID, TEST_QUERY_BUSINESS_NAME, DB_BUNDLE_NAME, ROW_COUNT);
    EXPECT_NE(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB end";
}

/**
 * @tc.number: STORAGE_GetBundleNameFromDB_00003
 * @tc.name: STORAGE_GetBundleNameFromDB_00003
 * @tc.desc: Test function of the GetColumnIndex function in the GetBundleNameFromDB interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleNameFromDB_00003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    std::shared_ptr<MockResultSet> mockResultSet = std::make_shared<MockResultSet>();
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };
    EXPECT_CALL(*mockStore, QueryByStep(::testing::Eq(GET_SQL), ::testing::Eq(bindArgs), true))
        .WillOnce(Return(mockResultSet));
    EXPECT_CALL(*mockResultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(ROW_COUNT), Return(OHOS::E_OK)));
    EXPECT_CALL(*mockResultSet, GoToFirstRow()).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*mockResultSet, GetColumnIndex(_, _)).WillOnce(Return(E_ERR));
    int32_t ret = service->GetBundleNameFromDB(TEST_QUERY_USER_ID, TEST_QUERY_BUSINESS_NAME, DB_BUNDLE_NAME, ROW_COUNT);
    EXPECT_NE(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB end";
}

/**
 * @tc.number: STORAGE_GetBundleNameFromDB_00004
 * @tc.name: STORAGE_GetBundleNameFromDB_00004
 * @tc.desc: Test function of the GetString function in the GetBundleNameFromDB interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleNameFromDB_00004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    std::shared_ptr<MockResultSet> mockResultSet = std::make_shared<MockResultSet>();
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };
    EXPECT_CALL(*mockStore, QueryByStep(::testing::Eq(GET_SQL), ::testing::Eq(bindArgs), true))
        .WillOnce(Return(mockResultSet));
    EXPECT_CALL(*mockResultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(ROW_COUNT), Return(OHOS::E_OK)));
    EXPECT_CALL(*mockResultSet, GoToFirstRow()).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*mockResultSet, GetColumnIndex(_, _)).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*mockResultSet, GetString(_, _)).WillOnce(Return(E_ERR));
    int32_t ret = service->GetBundleNameFromDB(TEST_QUERY_USER_ID, TEST_QUERY_BUSINESS_NAME, DB_BUNDLE_NAME, ROW_COUNT);
    EXPECT_NE(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB end";
}

/**
 * @tc.number: STORAGE_GetBundleNameFromDB_00005
 * @tc.name: STORAGE_GetBundleNameFromDB_00005
 * @tc.desc: Test function of GetBundleNameFromDB interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleNameFromDB_00005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    std::shared_ptr<MockResultSet> mockResultSet = std::make_shared<MockResultSet>();
    const std::vector<OHOS::NativeRdb::ValueObject> bindArgs = {
        OHOS::NativeRdb::ValueObject(TEST_QUERY_BUSINESS_NAME),
        OHOS::NativeRdb::ValueObject(TEST_QUERY_USER_ID)
    };
    EXPECT_CALL(*mockStore, QueryByStep(::testing::Eq(GET_SQL), ::testing::Eq(bindArgs), true))
        .WillOnce(Return(mockResultSet));
    EXPECT_CALL(*mockResultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(ROW_COUNT), Return(OHOS::E_OK)));
    EXPECT_CALL(*mockResultSet, GoToFirstRow()).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*mockResultSet, GetColumnIndex(_, _)).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*mockResultSet, GetString(_, _)).WillOnce(Return(OHOS::E_OK));
    int32_t ret = service->GetBundleNameFromDB(TEST_QUERY_USER_ID, TEST_QUERY_BUSINESS_NAME, DB_BUNDLE_NAME, ROW_COUNT);
    EXPECT_EQ(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameFromDB end";
}

/**
 * @tc.number: STORAGE_InsertExtBundleStats_00001
 * @tc.name: STORAGE_InsertExtBundleStats_00001
 * @tc.desc: Test function of InsertExtBundleStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_InsertExtBundleStats_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_InsertExtBundleStats start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    EXPECT_CALL(*mockStore, Insert(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = service->InsertExtBundleStats(TEST_QUERY_USER_ID, EXT_BUNDLE_STATS, CALLING_BUNDLE_NAME);
    EXPECT_NE(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_InsertExtBundleStats end";
}

/**
 * @tc.number: STORAGE_InsertExtBundleStats_00002
 * @tc.name: STORAGE_InsertExtBundleStats_00002
 * @tc.desc: Test function of InsertExtBundleStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_InsertExtBundleStats_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_InsertExtBundleStats start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    EXPECT_CALL(*mockStore, Insert(_, _, _)).WillOnce(Return(OHOS::E_OK));
    int32_t ret = service->InsertExtBundleStats(TEST_QUERY_USER_ID, EXT_BUNDLE_STATS, CALLING_BUNDLE_NAME);
    EXPECT_EQ(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_InsertExtBundleStats end";
}

/**
 * @tc.number: STORAGE_UpdateExtBundleStats_00001
 * @tc.name: STORAGE_UpdateExtBundleStats_00001
 * @tc.desc: Test function of UpdateExtBundleStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_UpdateExtBundleStats_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_UpdateExtBundleStats start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    EXPECT_CALL(*mockStore, Update(_, _, _, _, _)).WillOnce(testing::Return(E_ERR));
    int32_t ret = service->UpdateExtBundleStats(TEST_QUERY_USER_ID, EXT_BUNDLE_STATS, CALLING_BUNDLE_NAME);
    EXPECT_NE(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_UpdateExtBundleStats end";
}

/**
 * @tc.number: STORAGE_UpdateExtBundleStats_00002
 * @tc.name: STORAGE_UpdateExtBundleStats_00002
 * @tc.desc: Test function of UpdateExtBundleStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_UpdateExtBundleStats_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_UpdateExtBundleStats start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    EXPECT_CALL(*mockStore, Update(_, _, _, _, _)).WillOnce(testing::Return(OHOS::E_OK));
    int32_t ret = service->UpdateExtBundleStats(TEST_QUERY_USER_ID, EXT_BUNDLE_STATS, CALLING_BUNDLE_NAME);
    EXPECT_EQ(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_UpdateExtBundleStats end";
}

/**
 * @tc.number: STORAGE_GetCallingPkgName_00001
 * @tc.name: STORAGE_GetCallingPkgName_00001
 * @tc.desc: Test function of GetCallingPkgName interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetCallingPkgName_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetCallingPkgName_00001 start";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto str = service->GetCallingPkgName();
    EXPECT_EQ(str, "");
    GTEST_LOG_(INFO) << "STORAGE_GetCallingPkgName_00001 end";
}

/**
 * @tc.number: STORAGE_GetBundleStats_00001
 * @tc.name: STORAGE_GetBundleStats_00001
 * @tc.desc: Test function of GetBundleStats interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleStats_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleStats_00001 start";
    std::string pkgName = "test";
    BundleStats bundleStats;
    int32_t appIndex = 1;
    uint32_t statFlag = 1;
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto str = service->GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
    EXPECT_NE(str, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleStats_00001 end";
}

/**
 * @tc.number: STORAGE_GetBundleStats_00002
 * @tc.name: STORAGE_GetBundleStats_00002
 * @tc.desc: Test function of GetBundleStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleStats_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleStats_00002 start";
    std::string pkgName = "test";
    BundleStats pkgStats;
    int32_t appIndex = 1;
    uint32_t statFlag = 1;
    int32_t userId = 0;
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto ret = service->GetBundleStats(pkgName, userId, pkgStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_BUNDLEMGR_ERROR);
    userId = -1;
    ret = service->GetBundleStats(pkgName, userId, pkgStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_USERID_RANGE);
    userId = MAX_USER_ID + 1;
    ret = service->GetBundleStats(pkgName, userId, pkgStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_USERID_RANGE);
    userId = 1;
    appIndex = -1;
    ret = service->GetBundleStats(pkgName, userId, pkgStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_APPINDEX_RANGE);
    appIndex = MAX_APP_INDEX + 1;
    ret = service->GetBundleStats(pkgName, userId, pkgStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_APPINDEX_RANGE);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleStats_00002 end";
}

/**
 * @tc.number: STORAGE_GetUserStorageStats_00001
 * @tc.name: STORAGE_GetUserStorageStats_00001
 * @tc.desc: Test function of GetUserStorageStats interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetUserStorageStats_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetUserStorageStats_00001 start";
    StorageStats storageStats;
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto ret = service->GetUserStorageStats(storageStats);
    EXPECT_EQ(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetUserStorageStats_00001 end";
}

/**
 * @tc.number: STORAGE_GetUserStorageStats_00002
 * @tc.name: STORAGE_GetUserStorageStats_00002
 * @tc.desc: Test function of GetUserStorageStats interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetUserStorageStats_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetUserStorageStats_00002 start";
    StorageStats storageStats;
    int32_t userId = 1;
    bool isSchedule = false;
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto ret = service->GetUserStorageStats(userId, storageStats, isSchedule);
    EXPECT_EQ(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetUserStorageStats_00002 end";
}

/**
 * @tc.number: STORAGE_ConvertBytesToMB_00001
 * @tc.name: STORAGE_ConvertBytesToMB_00001
 * @tc.desc: Test function of ConvertBytesToMB interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_ConvertBytesToMB_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00001 start";
    int64_t bytes = 0;
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto ret = service->ConvertBytesToMB(bytes);
    EXPECT_NE(ret, "");
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00001 end";
}

/**
 * @tc.number: STORAGE_GetBundleNameAndUid_00001
 * @tc.name: STORAGE_GetBundleNameAndUid_00001
 * @tc.desc: Test function of GetBundleNameAndUid interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetBundleNameAndUid_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameAndUid_00001 start";
    int32_t userId = 1;
    std::map<int32_t, std::string> bundleNameAndUid;
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto ret = service->GetBundleNameAndUid(userId, bundleNameAndUid);
    EXPECT_NE(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetBundleNameAndUid_00001 end";
}

/**
 * @tc.number: STORAGE_GetUserStorageStatsByType_00001
 * @tc.name: STORAGE_GetUserStorageStatsByType_00001
 * @tc.desc: Test function of GetUserStorageStatsByType interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusManagerTest, STORAGE_GetUserStorageStatsByType_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetUserStorageStatsByType_00001 start";
    int32_t userId = 1;
    StorageStats storageStats;
    std::string type = "media";
    auto service = DelayedSingleton<StorageStatusManager>::GetInstance();
    auto ret = service->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_NE(ret, OHOS::E_OK);
    type = "file";
    ret = service->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_EQ(ret, OHOS::E_OK);
    type = "test";
    ret = service->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_EQ(ret, OHOS::E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetUserStorageStatsByType_00001 end";
}
} // namespace StorageManager
} // namespace OHOS