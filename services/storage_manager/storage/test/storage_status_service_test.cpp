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

namespace {
using namespace OHOS;
using namespace OHOS::StorageManager;
const std::vector<int64_t> bundleStatsInfo = {0, 1, std::numeric_limits<int64_t>::max() - 8, 10, 0};
} // namespace

namespace OHOS::AppExecFwk {
bool BundleMgrProxy::GetAllBundleStats(int32_t userId, std::vector<int64_t> &bundleStats)
{
    std::cout << "mock get All bundle stats" << std::endl;
    bundleStats = bundleStatsInfo;
    return true;
}
} // namespace OHOS::AppExecFwk


class StorageStatusServiceTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

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
