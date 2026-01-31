/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "disk.h"
#include "disk/disk_manager_service.h"
#include "disk/disk_notification.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class DiskNotificationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: DiskNotification_NotifyDiskChange_0000
 * @tc.desc: Verify the NotifyDiskChange function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskNotificationTest, DiskNotification_NotifyDiskChange_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskNotificationTest-begin DiskNotification_NotifyDiskChange_0000";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-11-0";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-3";
    int32_t flag = 1;
    std::shared_ptr<Disk> diskPtr = nullptr;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    diskPtr = dmService.GetDiskById(diskId);
    EXPECT_NE(diskPtr, nullptr);
    DiskNotification::GetInstance().NotifyDiskChange(StorageDaemon::DiskInfo::DiskState::REMOVED, diskPtr);
    EXPECT_EQ(diskPtr->GetDiskId(), diskId);

    dmService.OnDiskDestroyed(diskId);
    diskPtr = dmService.GetDiskById(diskId);
    EXPECT_EQ(diskPtr, nullptr);
    GTEST_LOG_(INFO) << "DiskNotificationTest-end DiskNotification_NotifyDiskChange_0000";
}

/**
 * @tc.name: DiskNotification_NotifyDiskChange_0001
 * @tc.desc: Verify the NotifyDiskChange function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskNotificationTest, DiskNotification_NotifyDiskChange_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskNotificationTest-begin DiskNotification_NotifyDiskChange_0001";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-11-0";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-3";
    int32_t flag = 1;
    std::shared_ptr<Disk> diskPtr = nullptr;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    diskPtr = dmService.GetDiskById(diskId);
    EXPECT_NE(diskPtr, nullptr);
    DiskNotification::GetInstance().NotifyDiskChange(StorageDaemon::DiskInfo::DiskState::MOUNTED, diskPtr);
    EXPECT_EQ(diskPtr->GetDiskId(), diskId);

    dmService.OnDiskDestroyed(diskId);
    diskPtr = dmService.GetDiskById(diskId);
    EXPECT_EQ(diskPtr, nullptr);
    GTEST_LOG_(INFO) << "DiskNotificationTest-end DiskNotification_NotifyDiskChange_0001";
}

/**
 * @tc.name: DiskNotification_NotifyDiskChange_0002
 * @tc.desc: Verify the NotifyDiskChange function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskNotificationTest, DiskNotification_NotifyDiskChange_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskNotificationTest-begin DiskNotification_NotifyDiskChange_0002";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-11-0";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-3";
    int32_t flag = 1;
    std::shared_ptr<Disk> diskPtr = nullptr;
    DiskNotification::GetInstance().NotifyDiskChange(StorageDaemon::DiskInfo::DiskState::MOUNTED, diskPtr);
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    diskPtr = dmService.GetDiskById(diskId);
    EXPECT_NE(diskPtr, nullptr);
    StorageDaemon::DiskInfo::DiskState diskState = static_cast<StorageDaemon::DiskInfo::DiskState>(-1);
    DiskNotification::GetInstance().NotifyDiskChange(diskState, diskPtr);
    EXPECT_EQ(diskPtr->GetDiskId(), diskId);

    dmService.OnDiskDestroyed(diskId);
    diskPtr = dmService.GetDiskById(diskId);
    EXPECT_EQ(diskPtr, nullptr);
    GTEST_LOG_(INFO) << "DiskNotificationTest-end DiskNotification_NotifyDiskChange_0002";
}
}
