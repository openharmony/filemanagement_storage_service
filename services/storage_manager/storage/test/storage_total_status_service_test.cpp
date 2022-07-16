/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "storage/storage_status_service.h"
#include "storage/storage_total_status_service.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class storage_total_status_service_test : public testing::Test {
public:
    static void SetUpTestCase(void) {};
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
HWTEST_F(storage_total_status_service_test, Storage_total_status_GetSystemSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_total_status_service_GetSystemSize_0000";
    std::shared_ptr<StorageTotalStatusService> service = DelayedSingleton<StorageTotalStatusService>::GetInstance();
    int64_t result = service->GetSystemSize();
    EXPECT_GE(result, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_total_status_service_GetSystemSize_0000";
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
HWTEST_F(storage_total_status_service_test, Storage_total_status_GetTotalSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_total_status_service_GetTotalSize_0000";
    std::shared_ptr<StorageTotalStatusService> service = DelayedSingleton<StorageTotalStatusService>::GetInstance();
    int64_t result = service->GetTotalSize();
    EXPECT_GE(result, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_total_status_service_GetTotalSize_0000";
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
HWTEST_F(storage_total_status_service_test, Storage_total_status_GetFreeSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_total_status_service_GetFreeSize_0000";
    std::shared_ptr<StorageTotalStatusService> service = DelayedSingleton<StorageTotalStatusService>::GetInstance();
    int64_t result = service->GetFreeSize();
    EXPECT_GE(result, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_total_status_service_GetFreeSize_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_GetUserStorageStats_0000
 * @tc.name: Storage_status_service_GetUserStorageStats_0000
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(storage_total_status_service_test, Storage_status_GetUserStorageStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_status_service_GetUserStorageStats_0000";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    StorageStats result = service->GetUserStorageStats();
    EXPECT_GE(result.total_, 0);
    EXPECT_GE(result.audio_, 0);
    EXPECT_GE(result.video_, 0);
    EXPECT_GE(result.image_, 0);
    EXPECT_GE(result.file_, 0);
    EXPECT_GE(result.app_, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_status_service_GetUserStorageStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_GetUserStorageStats_0001
 * @tc.name: Storage_status_service_GetUserStorageStats_0001
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(storage_total_status_service_test, Storage_status_GetUserStorageStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_status_service_GetUserStorageStats_0001";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    int32_t userId = 200;
    StorageStats result = service->GetUserStorageStats(userId);
    EXPECT_GE(result.total_, 0);
    EXPECT_GE(result.audio_, 0);
    EXPECT_GE(result.video_, 0);
    EXPECT_GE(result.image_, 0);
    EXPECT_GE(result.file_, 0);
    EXPECT_GE(result.app_, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_status_service_GetUserStorageStats_0001";
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
HWTEST_F(storage_total_status_service_test, Storage_status_GetBundleStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_status_service_GetBundleStats_0000";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    string pkgName = "com.test";
    BundleStats result = service->GetBundleStats(pkgName);
    GTEST_LOG_(INFO) << result.appSize_;
    GTEST_LOG_(INFO) << result.cacheSize_;
    GTEST_LOG_(INFO) << result.dataSize_;
    EXPECT_EQ(result.appSize_, 0);
    EXPECT_EQ(result.cacheSize_, 0);
    EXPECT_EQ(result.dataSize_, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_status_service_GetBundleStats_0000";
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
HWTEST_F(storage_total_status_service_test, Storage_status_GetBundleStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_status_service_GetBundleStats_0001";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    int userId = 100;
    string pkgName = "com.test";
    BundleStats result = service->GetBundleStats(pkgName, userId);
    GTEST_LOG_(INFO) << result.appSize_;
    GTEST_LOG_(INFO) << result.cacheSize_;
    GTEST_LOG_(INFO) << result.dataSize_;
    EXPECT_EQ(result.appSize_, 0);
    EXPECT_EQ(result.cacheSize_, 0);
    EXPECT_EQ(result.dataSize_, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_status_service_GetBundleStats_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_status_service_GetCurrentBundleStats_0000
 * @tc.name: Storage_status_service_GetCurrentBundleStats_0000
 * @tc.desc: Test function of GetCurrentBundleStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(storage_total_status_service_test, Storage_status_GetCurrentBundleStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_total_status_service_test-begin Storage_status_service_GetCurrentBundleStats_0000";
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    BundleStats result = service->GetCurrentBundleStats();
    GTEST_LOG_(INFO) << result.appSize_;
    GTEST_LOG_(INFO) << result.cacheSize_;
    GTEST_LOG_(INFO) << result.dataSize_;
    EXPECT_EQ(result.appSize_, 0);
    EXPECT_EQ(result.cacheSize_, 0);
    EXPECT_EQ(result.dataSize_, 0);
    GTEST_LOG_(INFO) << "storage_total_status_service_test-end Storage_status_service_GetCurrentBundleStats_0000";
}
} // namespace
