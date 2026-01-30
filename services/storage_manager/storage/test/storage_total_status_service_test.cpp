/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "directory_ex.h"
#include "nativetoken_kit.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_monitor_service.h"
#include "storage/storage_status_manager.h"
#include "storage/storage_total_status_service.h"
#include "storage_service_errno.h"
#include "token_setproc.h"
#include "utils/string_utils.h"

static void SetNativeToken()
{
    uint64_t tokenId;
    const char *perms[] = {
        "ohos.permission.READ_MEDIA",
        "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED",
        "ohos.permission.MANAGE_LOCAL_ACCOUNTS",
        "ohos.permission.ACCESS_SCREEN_LOCK",
        "ohos.permission.ACCESS_SCREEN_LOCK_INNER",
        "ohos.permission.REMOVE_CACHE_FILES",
        "ohos.permission.PUBLISH_SYSTEM_COMMON_EVENT"
    };
    const char* acls[] = { "ohos.permission.ACCESS_SCREEN_LOCK_INNER" };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 7,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = acls,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "storage_service_test";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
using namespace StorageDaemon;
struct DirInfo {
    const std::string path;
    mode_t mode;
    uid_t uid;
    gid_t gid;
};
constexpr uid_t OID_ROOT = 0;
static constexpr int MODE_0711 = 0711;
static constexpr int MODE_02771 = 02771;
constexpr uid_t OID_FILE_MANAGER = 1006;
constexpr uid_t OID_USER_DATA_RW = 1008;
constexpr uid_t OID_DFS = 1009;
const std::vector<DirInfo> virtualDir_{{"/storage/media/%d", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
        {"/storage/media/%d/local", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
        {"/storage/cloud", MODE_0711, OID_ROOT, OID_ROOT},
        {"/storage/cloud/%d", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
        {"/mnt/share/", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/share/%d/", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/data/%d/", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/data/%d/cloud", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/data/%d/cloud_fuse", MODE_0711, OID_DFS, OID_DFS},
        {"/mnt/data/%d/userExternal", MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
        {"/mnt/data/%d/hmdfs", MODE_0711, OID_FILE_MANAGER, OID_FILE_MANAGER},
        {"/mnt/hmdfs/", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/hmdfs/%d/", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/hmdfs/%d/cloud", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/hmdfs/%d/account", MODE_0711, OID_ROOT, OID_ROOT},
        {"/mnt/hmdfs/%d/non_account", MODE_0711, OID_ROOT, OID_ROOT}};

class StorageTotalStatusServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void) { SetNativeToken(); };
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Storage_total_status_service_GetSystemSize_0000
 * @tc.name: Storage_total_status_service_GetSystemSize_0000
 * @tc.desc: Test function of GetSystemSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_total_status_GetSystemSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_total_status_service_GetSystemSize_0000";
    StorageTotalStatusService& service = StorageTotalStatusService::GetInstance();
    int64_t systemSize;
    int32_t result = service.GetSystemSize(systemSize);
    EXPECT_EQ(result, 0);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_total_status_service_GetSystemSize_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_total_status_service_GetTotalSize_0000
 * @tc.name: Storage_total_status_service_GetTotalSize_0000
 * @tc.desc: Test function of GetTotalSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_total_status_GetTotalSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_total_status_service_GetTotalSize_0000";
    StorageTotalStatusService& service = StorageTotalStatusService::GetInstance();
    int64_t totalSize;
    int32_t result = service.GetTotalSize(totalSize);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_total_status_service_GetTotalSize_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_total_status_service_GetFreeSize_0000
 * @tc.name: Storage_total_status_service_GetFreeSize_0000
 * @tc.desc: Test function of GetFreeSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_total_status_GetFreeSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_total_status_service_GetFreeSize_0000";
    StorageTotalStatusService& service = StorageTotalStatusService::GetInstance();
    int64_t freeSize;
    int32_t result = service.GetFreeSize(freeSize);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_total_status_service_GetFreeSize_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_GetBundleStats_0000
 * @tc.name: Storage_status_service_GetBundleStats_0000
 * @tc.desc: Test function of GetBundleStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_status_GetBundleStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_service_GetBundleStats_0000";
    StorageStatusManager &service = StorageStatusManager::GetInstance();
    string pkgName = "com.test";
    BundleStats bundleStats;
    int32_t result = service.GetBundleStats(pkgName, bundleStats, 0, 0);
    EXPECT_EQ(result, E_BUNDLEMGR_ERROR);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_service_GetBundleStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_GetBundleStats_0001
 * @tc.name: Storage_status_service_GetBundleStats_0001
 * @tc.desc: Test function of GetBundleStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_status_GetBundleStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_service_GetBundleStats_0001";
    StorageStatusManager &service = StorageStatusManager::GetInstance();
    int userId = 100;
    string pkgName = "com.test";
    BundleStats bundleStats;
    int32_t result = service.GetBundleStats(pkgName, userId, bundleStats, 0, 0);
    EXPECT_EQ(result, E_BUNDLEMGR_ERROR);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_service_GetBundleStats_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_GetCurrentBundleStats_0000
 * @tc.name: Storage_status_service_GetCurrentBundleStats_0000
 * @tc.desc: Test function of GetCurrentBundleStats when caller is not a hap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_status_GetCurrentBundleStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_service_GetCurrentBundleStats_0000";
    StorageStatusManager &service = StorageStatusManager::GetInstance();
    BundleStats bundleStats;
    int32_t result = service.GetCurrentBundleStats(bundleStats, 0);
    EXPECT_EQ(result, E_GET_BUNDLE_NAME_FAILED);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_service_GetCurrentBundleStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_ResetBundleMgrProxy_0000
 * @tc.name: Storage_total_status_ResetBundleMgrProxy_0000
 * @tc.desc: Test function of ResetBundleMgrProxy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_total_status_ResetBundleMgrProxy_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_total_status_ResetBundleMgrProxy_0000";
    BundleMgrConnector& service = BundleMgrConnector::GetInstance();
    int64_t result = service.ResetBundleMgrProxy();
    EXPECT_GE(result, 0);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_total_status_ResetBundleMgrProxy_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_StartStorageMonitorTask_0000
 * @tc.name: Storage_status_service_StartStorageMonitorTask_0000
 * @tc.desc: Test function of StartStorageMonitorTask interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_status_StartStorageMonitorTask_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_service_StartStorageMonitorTask_0000";
    StorageMonitorService& service = StorageMonitorService::GetInstance();
    service.StartStorageMonitorTask();
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_service_StartStorageMonitorTask_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_Execute_0000
 * @tc.name: Storage_status_service_Execute_0000
 * @tc.desc: Test function of Execute interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_status_Execute_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_service_Execute_0000";
    StorageMonitorService& service = StorageMonitorService::GetInstance();
    service.Execute();
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_service_Execute_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_MonitorAndManageStorage_0000
 * @tc.name: Storage_status_service_MonitorAndManageStorage_0000
 * @tc.desc: Test function of MonitorAndManageStorage interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_status_MonitorAndManageStorage_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_status_service_MonitorAndManageStorage_0000";
    StorageMonitorService& service = StorageMonitorService::GetInstance();
    service.MonitorAndManageStorage();
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_status_service_MonitorAndManageStorage_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_total_status_service_GetInodeOfPath_0000
 * @tc.name: Storage_total_status_service_GetInodeOfPath_0000
 * @tc.desc: Test function of GetInodeOfPath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageTotalStatusServiceTest, Storage_total_status_GetInodeOfPath_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin Storage_total_status_service_GetInodeOfPath_0000";
    StorageTotalStatusService& service = StorageTotalStatusService::GetInstance();
    int32_t type = static_cast<int32_t>(StorageStatType::TOTAL);
    int64_t inodeCnt = 0;
    int32_t result = service.GetInodeOfPath(PATH_DATA, type, inodeCnt);
    EXPECT_EQ(result, E_OK);

    type = static_cast<int32_t>(StorageStatType::FREE);
    result = service.GetInodeOfPath(PATH_DATA, type, inodeCnt);
    EXPECT_EQ(result, E_OK);

    type = static_cast<int32_t>(StorageStatType::USED);
    result = service.GetInodeOfPath(PATH_DATA, type, inodeCnt);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end Storage_total_status_service_GetInodeOfPath_0000";
}
} // namespace
