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

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <system_ability_definition.h>
#include <storage/storage_total_status_service.h>

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "datashare_helper_mock.h"
#include "datashare_result_set_mock.h"
#include "disk.h"
#include "ext_bundle_stats.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_status_manager.h"
#include "storage_rdb_adapter.h"
#include "storage_service_errno.h"

using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::DataShare;
namespace OHOS {
namespace StorageManager {
class MockBundleMgr : public AppExecFwk::IBundleMgr {
public:
    bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        bundleName = "com.example.fake";
        return true;
    }
    sptr<IRemoteObject> AsObject() override { return nullptr; }
};

BundleMgrConnector::BundleMgrConnector() {}
BundleMgrConnector::~BundleMgrConnector() {}

sptr<AppExecFwk::IBundleMgr> g_testBundleMgrProxy = nullptr;
const int MEDIA_TYPE_IMAGE = 1;
const int MEDIA_TYPE_AUDIO = 3;
const int MEDIA_TYPE_VIDEO = 2;
sptr<AppExecFwk::IBundleMgr> BundleMgrConnector::GetBundleMgrProxy()
{
    return g_testBundleMgrProxy;
}

class StorageStatusManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline shared_ptr<DataShareHelperMock> dataShareHelperMock_ = nullptr;
    static inline shared_ptr<DataShareResultSetMock> resultSetMock_ = nullptr;
};

void StorageStatusManagerTest::SetUpTestCase()
{
    dataShareHelperMock_ = std::make_shared<DataShareHelperMock>();
    resultSetMock_ = std::make_shared<DataShareResultSetMock>();
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    DataShareResultSetMock::proxy_ = resultSetMock_;
}

void StorageStatusManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataShareHelperMock_ = nullptr;
    DataShareHelperMock::proxy_ = nullptr;
    resultSetMock_ = nullptr;
    DataShareResultSetMock::proxy_ = nullptr;
}

void StorageStatusManagerTest::SetUp(void)
{
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    DataShareResultSetMock::proxy_ = resultSetMock_;
}

void StorageStatusManagerTest::TearDown(void) {}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0001
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0001
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0001";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillOnce(Return(E_ERR));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, E_GETROWCOUNT);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0002
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0002
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0002";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(E_ERR), Return(OHOS::E_OK)));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, E_GETROWCOUNT);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0002";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0003
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0003
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0003";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(E_ERR), Return(E_ERR)));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, E_GETROWCOUNT);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0003";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0004
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0004
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0004";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_ERR));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0004";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0005
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0005
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0005";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillOnce(Return(E_ERR));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0005";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0006
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0006
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0006";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillOnce(Return(E_ERR));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0006";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0007
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0007
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0007";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(OHOS::E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(E_ERR));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0007";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0008
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0008
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0008";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(OHOS::E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillRepeatedly(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillOnce(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(E_ERR));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0008";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0009
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0009
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0009";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(OHOS::E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillRepeatedly(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(MEDIA_TYPE_IMAGE),
        Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(OHOS::E_OK));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0009";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0010
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0010
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0010";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillRepeatedly(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(E_ERR),
        Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(OHOS::E_OK));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0010";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0011
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0011
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0011";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillRepeatedly(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(MEDIA_TYPE_AUDIO),
        Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(OHOS::E_OK));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0011";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0012
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0012
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0012";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillRepeatedly(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(MEDIA_TYPE_VIDEO),
        Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(OHOS::E_OK));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0012";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetMediaAndFileStorageStats_0013
 * @tc.name: Storage_status_GetMediaAndFileStorageStats_0013
 * @tc.desc: Test function of GetMediaAndFileStorageStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */

HWTEST_F(StorageStatusManagerTest, Storage_status_GetMediaAndFileStorageStats_0013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetMediaAndFileStorageStats_0013";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(OHOS::E_OK)).WillRepeatedly(Return(E_ERR));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillRepeatedly(Return(OHOS::E_OK));
    EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(MEDIA_TYPE_VIDEO + 1),
        Return(OHOS::E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(OHOS::E_OK));
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetMediaAndFileStorageStats_0013";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetBundleStats_0001
 * @tc.name: Storage_status_GetBundleStats_0001
 * @tc.desc: Test function of GetBundleStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusManagerTest, Storage_status_GetBundleStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetBundleStats_0001";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    std::string pkgName = "test";
    BundleStats pkgStats;
    int32_t appIndex = 1;
    uint32_t statFlag = 1;
    int32_t userId = 0;
    auto ret = service->GetBundleStats(pkgName, userId, pkgStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetBundleStats_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetAppSize_0001
 * @tc.name: Storage_status_GetAppSize_0001
 * @tc.desc: Test function of GetAppSize interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusManagerTest, Storage_status_GetAppSize_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetAppSize_0001";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    int32_t userId = 0;
    int64_t appSize = 0;
    auto ret = service->GetAppSize(userId, appSize);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetAppSize_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetCurrentBundleStats_0001
 * @tc.name: Storage_status_GetCurrentBundleStats_0001
 * @tc.desc: Test function of GetUserStorageStats interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusManagerTest, Storage_status_GetCurrentBundleStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetCurrentBundleStats_0001";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    BundleStats bundleStats;
    uint32_t statFlag = 0;
    auto ret = service->GetCurrentBundleStats(bundleStats, statFlag);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetCurrentBundleStats_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_GetBundleNameAndUid_0001
 * @tc.name: Storage_status_GetBundleNameAndUid_0001
 * @tc.desc: Test function of GetBundleNameAndUid interface for error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusManagerTest, Storage_status_GetBundleNameAndUid_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetBundleNameAndUid_0001";
    std::shared_ptr<StorageStatusManager> service = DelayedSingleton<StorageStatusManager>::GetInstance();
    int32_t userId = 0;
    std::map<int32_t, std::string> bundleNameAndUid;
    auto ret = service->GetBundleNameAndUid(userId, bundleNameAndUid);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetBundleNameAndUid_0001";
}
}
}