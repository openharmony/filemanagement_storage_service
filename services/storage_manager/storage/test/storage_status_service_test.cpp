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

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_status_service.h"
#include "storage_service_errno.h"
#include <storage/storage_total_status_service.h>

namespace {
using namespace OHOS;
using namespace OHOS::StorageManager;
const std::vector<int64_t> bundleStatsInfo = {0, 1, std::numeric_limits<int64_t>::max() - 8, 10, 0};
int g_flag = 1;
int g_bundleUid = 0;
int g_bundleFlag  = 1;
int g_sysFlag = 1;
int64_t free_size = 0;
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
 * @tc.number: SUB_STORAGE_QueryOccupiedSpaceForSa_0001
 * @tc.name: QueryOccupiedSpaceForSa_0001
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, QueryOccupiedSpaceForSa_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin QueryOccupiedSpaceForSa_0001";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int32_t userId = 100;
    int32_t result = service->QueryOccupiedSpaceForSa(storageStats, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end QueryOccupiedSpaceForSa_0001";
} // namespace AppExecFwk

/**
 * @tc.number: SUB_STORAGE_QueryOccupiedSpaceForSa_0002
 * @tc.name: QueryOccupiedSpaceForSa_0002
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, QueryOccupiedSpaceForSa_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin QueryOccupiedSpaceForSa_0002";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
        StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int32_t userId = 100;
    int32_t result = service->QueryOccupiedSpaceForSa(storageStats, userId);
    EXPECT_EQ(result, E_OK);
    result = service->QueryOccupiedSpaceForSa(storageStats, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end QueryOccupiedSpaceForSa_0002";
} // namespace AppExecFwk

/**
 * @tc.number: SUB_STORAGE_QueryOccupiedSpaceForSa_0003
 * @tc.name: QueryOccupiedSpaceForSa_0003
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, QueryOccupiedSpaceForSa_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin QueryOccupiedSpaceForSa_0003";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
        StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    g_flag = false;
    int32_t userId = 100;
    int32_t result = service->QueryOccupiedSpaceForSa(storageStats, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end QueryOccupiedSpaceForSa_0003";
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
 * @tc.number: SUB_STORAGE_CheckBundlePermissions_0001
 * @tc.name: STORAGE_CheckBundlePermissions_0001
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_CheckBundlePermissions_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin STORAGE_CheckBundlePermissions_0001";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    int userId = 100;
    std::string bundleName = "test";
    g_bundleFlag  = 0;
    bool isSchedule = true;
    int32_t result = service->CheckBundlePermissions(userId, bundleName, isSchedule);
    EXPECT_EQ(result, 0);

    g_bundleFlag  = 1;
    isSchedule = false;
    result = service->CheckBundlePermissions(userId, bundleName, isSchedule);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    g_bundleFlag  = 1;
    isSchedule = true;
    result = service->CheckBundlePermissions(userId, bundleName, isSchedule);
    EXPECT_EQ(result, E_OK);

    g_bundleFlag  = 0;
    isSchedule = false;
    str = "111";
    result = service->CheckBundlePermissions(userId, bundleName, isSchedule);
    EXPECT_EQ(result, E_BUNDLEMGR_ERROR);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end STORAGE_CheckBundlePermissions_0001";
}

/**
 * @tc.number: SUB_STORAGE_ProcessStorageStatus_0001
 * @tc.name: STORAGE_ProcessStorageStatus_0001
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageStatusServiceTest, STORAGE_ProcessStorageStatus_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-begin STORAGE_ProcessStorageStatus_0001";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    StorageStats storageStats = {0, 0, 0, 0, 0, 0};
    int userId = 0;
    bool isSchedule = true;
    g_flag = 0;
    g_sysFlag = 1;
    service->ProcessStorageStatus(storageStats, userId, isSchedule);
    EXPECT_TRUE(true);
    free_size = 999999999999999;
    isSchedule = false;
    service->ProcessStorageStatus(storageStats, userId, isSchedule);
    EXPECT_TRUE(true);
    free_size = 0;
    isSchedule = true;
    service->ProcessStorageStatus(storageStats, userId, isSchedule);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest-end STORAGE_ProcessStorageStatus_0001";
}
