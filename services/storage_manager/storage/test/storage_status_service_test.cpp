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
#include <storage/storage_total_status_service.h>

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "ipc_skeleton.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_status_service.h"
#include "storage_rdb_adapter.h"
#include "storage_service_errno.h"

namespace {
using namespace OHOS;
using namespace OHOS::StorageManager;
const std::vector<int64_t> bundleStatsInfo = {0, 1, std::numeric_limits<int64_t>::max() - 8, 10, 0};
int g_flag = 1;
int g_bundleUid = 0;
int g_bundleFlag  = 1;
int g_sysFlag = 1;
int64_t free_size = 0;
ErrCode g_getNameIndexRet = ERR_OK;
int32_t g_appIndex = 0;
std::string g_bundleName = "com.test.app";
int32_t g_getBundleStatsRet = E_OK;
int32_t accessTokenType = -1;
} // namespace
std::string str = "settings";
namespace OHOS::AppExecFwk {
bool BundleMgrProxy::GetAllBundleStats(int32_t userId, std::vector<int64_t> &bundleStats)
{
    bundleStats = bundleStatsInfo;
    return true;
}

bool BundleMgrProxy::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    bundleName = str;
    return g_bundleFlag ;
}

int BundleMgrProxy::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    return g_bundleUid;
}

ErrCode BundleMgrProxy::GetNameAndIndexForUid(int32_t uid, std::string &bundleName, int32_t &appIndex)
{
    bundleName = g_bundleName;
    appIndex = g_appIndex;
    return g_getNameIndexRet;
}
} // namespace OHOS::AppExecFwk

namespace OHOS::StorageManager {
int32_t StorageTotalStatusService::GetFreeSize(int64_t &freeSize)
{
    freeSize = free_size;
    return g_flag;
}

int32_t StorageTotalStatusService::GetSystemSize(int64_t &systemSize)
{
    return g_sysFlag;
}

int32_t GetMediaStorageStats(StorageStats &storageStats)
{
    return E_OK;
}

int32_t GetFileStorageStats(int32_t userId, StorageStats &storageStats)
{
    return E_OK;
}

int32_t StorageStatusService::GetBundleStats(const std::string &bundleName, int userId, BundleStats &bundleStats,
    int32_t appIndex, uint32_t statFlag)
{
    return g_getBundleStatsRet;
}
}

namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    if (accessTokenType == -1) {
        return Security::AccessToken::TOKEN_INVALID;
    }
    if (accessTokenType == 0) {
        return Security::AccessToken::TOKEN_HAP;
    }
    if (accessTokenType == 1) {
        return Security::AccessToken::TOKEN_NATIVE;
    }
    return Security::AccessToken::TOKEN_NATIVE;
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

class StorageStatusServiceTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

// void StorageStatusServiceTest::SetUp()
// {


/**
 * @tc.number: SUB_STORAGE_Storage_status_GetAppSize_0001
 * @tc.name: Storage_status_GetAppSize_0001
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, Storage_status_GetAppSize_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_GetAppSize_0001";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    int32_t userId = 100;
    int64_t appSize = 0;
    int32_t result = service->GetAppSize(userId, appSize);

    std::cout << "appsize is : " << appSize << "max int is " << std::numeric_limits<int64_t>::max() << std::endl;
    EXPECT_EQ(result, E_CALCULATE_OVERFLOW_UP);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_GetAppSize_0001";
} // namespace AppExecFwk

/**
 * @tc.number: SUB_STORAGE_GetMediaAndFileStorageStats_0001
 * @tc.name: STORAGE_GetMediaAndFileStorageStats_0001
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_GetMediaAndFileStorageStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin STORAGE_GetMediaAndFileStorageStats_0001";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
        StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end STORAGE_GetMediaAndFileStorageStats_0001";
} // namespace AppExecFwk

/**
 * @tc.number: SUB_STORAGE_GetMediaAndFileStorageStats_0002
 * @tc.name: STORAGE_GetMediaAndFileStorageStats_0002
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_GetMediaAndFileStorageStats_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin STORAGE_GetMediaAndFileStorageStats_0002";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    str = "99";
    int32_t result = service->GetMediaAndFileStorageStats(userId, storageStats);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end STORAGE_GetMediaAndFileStorageStats_0002";
} // namespace AppExecFwk

/**
 * @tc.number: STORAGE_GetCurrentBundleStats_0001
 * @tc.name: STORAGE_GetCurrentBundleStats_0001
 * @tc.desc: Test function of GetCurrentBundleStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_GetCurrentBundleStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0001 start: bundleMgr is null";
    g_getNameIndexRet = ERR_OK;
    g_appIndex = 1;
    g_bundleName = "com.test.app";
    g_getBundleStatsRet = E_OK;

    auto service = DelayedSingleton<StorageStatusService>::GetInstance();
    BundleStats bundleStats;
    uint32_t statFlag = 0x1;
    int32_t result = service->GetCurrentBundleStats(bundleStats, statFlag);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0001 end";
}

/**
 * @tc.number: STORAGE_GetCurrentBundleStats_0002
 * @tc.name: STORAGE_GetCurrentBundleStats_0002
 * @tc.desc: Test function of GetCurrentBundleStats interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_GetCurrentBundleStats_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0002 start: bundleMgr is null";
    g_getNameIndexRet = ERR_APPEXECFWK_PARCEL_ERROR;
    g_appIndex = 1;
    g_bundleName = "";

    auto service = DelayedSingleton<StorageStatusService>::GetInstance();
    BundleStats bundleStats;
    uint32_t statFlag = 0x1;
    int32_t result = service->GetCurrentBundleStats(bundleStats, statFlag);

    EXPECT_EQ(result, E_GET_BUNDLE_NAME_FAILED);
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0002 end";
}

/**
 * @tc.number: STORAGE_GetCurrentBundleStats_0003
 * @tc.name: STORAGE_GetCurrentBundleStats_0003
 * @tc.desc: Test function of GetCurrentBundleStats interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_GetCurrentBundleStats_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0003 start: bundleMgr is null";
    g_getNameIndexRet = ERR_OK;
    g_appIndex = 1;
    g_bundleName = "";

    auto service = DelayedSingleton<StorageStatusService>::GetInstance();
    BundleStats bundleStats;
    uint32_t statFlag = 0x1;
    int32_t result = service->GetCurrentBundleStats(bundleStats, statFlag);

    EXPECT_EQ(result, E_GET_BUNDLE_NAME_FAILED);
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0003 end";
}

/**
 * @tc.number: STORAGE_GetCurrentBundleStats_0004
 * @tc.name: STORAGE_GetCurrentBundleStats_0004
 * @tc.desc: Test function of GetCurrentBundleStats interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_GetCurrentBundleStats_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0004 start: bundleMgr is null";
    g_getNameIndexRet = ERR_OK;
    g_appIndex = 1;
    g_bundleName = "com.test.app";
    g_getBundleStatsRet = E_BUNDLEMGR_ERROR;

    auto service = DelayedSingleton<StorageStatusService>::GetInstance();
    BundleStats bundleStats;
    uint32_t statFlag = 0x1;
    int32_t result = service->GetCurrentBundleStats(bundleStats, statFlag);

    EXPECT_EQ(result, E_BUNDLEMGR_ERROR);
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentBundleStats_0004 end";
}

/**
 * @tc.number: STORAGE_SetExtBundleStats_00001
 * @tc.name: STORAGE_SetExtBundleStats_00001
 * @tc.desc: Test function of GetCurrentBundleStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_SetExtBundleStats_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_SetExtBundleStats_00001 start";
    auto service = DelayedSingleton<StorageStatusService>::GetInstance();
    uint32_t userId = 10;
    std::string businessName= "test";;
    uint64_t bundleSize = 1;
    int32_t ret = service->SetExtBundleStats(userId, businessName, bundleSize);
    EXPECT_EQ(ret, E_GET_CALL_BUNDLE_NAME_ERROR);
    accessTokenType = 0;
    ret = service->SetExtBundleStats(userId, businessName, bundleSize);
    EXPECT_NE(ret, E_OK);
    accessTokenType = 1;
    ret = service->SetExtBundleStats(userId, businessName, bundleSize);
    EXPECT_NE(ret, E_OK);
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    rdbAdapter.Init();
    ret = service->SetExtBundleStats(userId, businessName, bundleSize);
    EXPECT_EQ(ret, E_OK);
    ret = service->SetExtBundleStats(userId, businessName, bundleSize);
    EXPECT_EQ(ret, E_OK);
    rdbAdapter.UnInit();
    GTEST_LOG_(INFO) << "STORAGE_SetExtBundleStats_00001 end";
}

/**
 * @tc.number: STORAGE_GetExtBundleStats_00001
 * @tc.name: STORAGE_GetExtBundleStats_00001
 * @tc.desc: Test function of GetCurrentBundleStats interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_GetExtBundleStats_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetExtBundleStats_00001 start";
    auto service = DelayedSingleton<StorageStatusService>::GetInstance();
    uint32_t userId = 10;
    std::string businessName = "test";
    uint64_t bundleSize = 0;
    int32_t ret = service->GetExtBundleStats(userId, businessName, bundleSize);
    EXPECT_EQ(ret, E_TB_GET_ERROR);
    auto &rdbAdapter = StorageRdbAdapter::GetInstance();
    rdbAdapter.Init();
    ret = service->GetExtBundleStats(userId, businessName, bundleSize);
    EXPECT_EQ(ret, OHOS::E_OK);
    EXPECT_EQ(bundleSize, 1);
    rdbAdapter.UnInit();
    GTEST_LOG_(INFO) << "STORAGE_GetExtBundleStats_00001 end";
}
