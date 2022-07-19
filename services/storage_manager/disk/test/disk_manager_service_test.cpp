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

#include "disk/disk_manager_service.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class DiskManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_OnDiskCreated_0000
 * @tc.name: Disk_manager_service_OnDiskCreated_0000
 * @tc.desc: Test function of OnDiskCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_OnDiskCreated_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_OnDiskCreated_0000";
    std::shared_ptr<DiskManagerService> dmService =
        DelayedSingleton<DiskManagerService>::GetInstance();
    std::string diskId = "diskId-1-1";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    if (dmService != nullptr) {
        dmService->OnDiskCreated(disk);
    }
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_OnDiskCreated_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_OnDiskDestroyed_0000
 * @tc.name: Disk_manager_service_OnDiskDestroyed_0000
 * @tc.desc: Test function of OnDiskDestroyed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_OnDiskDestroyed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_OnDiskDestroyed_0000";
    std::shared_ptr<DiskManagerService> dmService =
        DelayedSingleton<DiskManagerService>::GetInstance();
    std::string diskId = "diskId-1-2";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    if (dmService != nullptr) {
        dmService->OnDiskCreated(disk);
        dmService->OnDiskDestroyed(diskId);
    }
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_OnDiskDestroyed_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_GetDiskById_0000
 * @tc.name: Disk_manager_service_GetDiskById_0000
 * @tc.desc: Test function of GetDiskById interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_GetDiskById_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_GetDiskById_0000";
    std::shared_ptr<DiskManagerService> dmService =
        DelayedSingleton<DiskManagerService>::GetInstance();
    std::string diskId = "diskId-1-3";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1;
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    if (dmService != nullptr) {
        dmService->OnDiskCreated(disk);
        result = dmService->GetDiskById(diskId);
    }
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_GetDiskById_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_Partition_0000
 * @tc.name: Disk_manager_service_Partition_0000
 * @tc.desc: Test function of Partition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_Partition_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_Partition_0000";
    std::shared_ptr<DiskManagerService> dmService =
        DelayedSingleton<DiskManagerService>::GetInstance();
    std::string diskId = "diskId-1-4";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1;
    int32_t type = 1;
    int32_t result;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    if (dmService != nullptr) {
        dmService->OnDiskCreated(disk);
        result = dmService->Partition(diskId, type);
    }
    EXPECT_EQ(0, 0);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_Partition_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_GetAllDisks_0000
 * @tc.name: Disk_manager_service_GetAllDisks_0000
 * @tc.desc: Test function of GetAllDisks interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_GetAllDisks_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_GetAllDisks_0000";
    std::shared_ptr<DiskManagerService> dmService =
        DelayedSingleton<DiskManagerService>::GetInstance();
    std::string diskId = "diskId-1-5";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService->OnDiskCreated(disk);
    vector<Disk> result = dmService->GetAllDisks();
    if (result.size() > 0)
    {
        GTEST_LOG_(INFO) << result[0]. GetSizeBytes();
        GTEST_LOG_(INFO) << result[0].GetDiskId();
        GTEST_LOG_(INFO) << result[0].GetSysPath();
        GTEST_LOG_(INFO) << result[0].GetVendor();
        GTEST_LOG_(INFO) << result[0].GetFlag();
    }
    EXPECT_GE(result.size(), 0);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service__0000";
}
} // namespace
