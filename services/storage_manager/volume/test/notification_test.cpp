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

#include "volume/notification.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class NotificationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0000
 * @tc.name: Notification_NotifyVolumeChange_0000
 * @tc.desc: Test function of NotifyVolumeChange interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */

HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0000";
        std::string volumeId = "vol-1-1";
        int32_t fsType = 1;
        std::string diskId = "disk-1-1";
        VolumeCore vc(volumeId, fsType, diskId);
        std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
        auto &notification = Notification::GetInstance();
        notification.NotifyVolumeChange(VolumeState::REMOVED, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0000";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0001
 * @tc.name: Notification_NotifyVolumeChange_0001
 * @tc.desc: Test function of NotifyVolumeChange interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */

HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTestt-begin Notification_NotifyVolumeChange_0001";
        std::string volumeId = "vol-1-1";
        int32_t fsType = 1;
        std::string diskId = "disk-1-1";
        VolumeCore vc(volumeId, fsType, diskId);
        std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
        auto &notification = Notification::GetInstance();
        notification.NotifyVolumeChange(VolumeState::UNMOUNTED, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0001";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0002
 * @tc.name: Notification_NotifyVolumeChange_0002
 * @tc.desc: Test function of NotifyVolumeChange interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */

HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0002";
        std::string volumeId = "vol-1-1";
        int32_t fsType = 1;
        std::string diskId = "disk-1-1";
        VolumeCore vc(volumeId, fsType, diskId);
        std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
        volume->SetFsUuid("uuid1");
        volume->SetPath("path1");
        auto &notification = Notification::GetInstance();
        notification.NotifyVolumeChange(VolumeState::MOUNTED, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0002";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0003
 * @tc.name: Notification_NotifyVolumeChange_0003
 * @tc.desc: Test function of NotifyVolumeChange interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */

HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0003";
        std::string volumeId = "vol-1-1";
        int32_t fsType = 1;
        std::string diskId = "disk-1-1";
        VolumeCore vc(volumeId, fsType, diskId);
        std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
        auto &notification = Notification::GetInstance();
        notification.NotifyVolumeChange(VolumeState::BAD_REMOVAL, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0003";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0004
 * @tc.name: Notification_NotifyVolumeChange_0004
 * @tc.desc: Test function of NotifyVolumeChange interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */

HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0004";
        std::string volumeId = "vol-1-1";
        int32_t fsType = 1;
        std::string diskId = "disk-1-1";
        VolumeCore vc(volumeId, fsType, diskId);
        std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
        auto &notification = Notification::GetInstance();
        notification.NotifyVolumeChange(VolumeState::EJECTING, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0004";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0005
 * @tc.name: Notification_NotifyVolumeChange_0005
 * @tc.desc: Test function of NotifyVolumeChange interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */

HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0005";
        std::string volumeId = "vol-1-1";
        int32_t fsType = 1;
        std::string diskId = "disk-1-1";
        VolumeCore vc(volumeId, fsType, diskId);
        std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
        auto &notification = Notification::GetInstance();
        notification.NotifyVolumeChange(VolumeState::CHECKING, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0005";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0006
 * @tc.name: Notification_NotifyVolumeChange_0006
 * @tc.desc: Test function of NotifyVolumeChange interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */

HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0006";
        std::string volumeId = "vol-1-1";
        int32_t fsType = 1;
        std::string diskId = "disk-1-1";
        VolumeCore vc(volumeId, fsType, diskId);
        std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
        auto &notification = Notification::GetInstance();
        notification.NotifyVolumeChange(VolumeState::FUSE_REMOVED, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0006";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0007
 * @tc.name: Notification_NotifyVolumeChange_0007
 * @tc.desc: Test function of NotifyVolumeChange with DAMAGED state.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0007";
    std::string volumeId = "vol-damaged-1";
    int32_t fsType = FsType::NTFS;
    std::string diskId = "disk-damaged-1";
    VolumeCore vc(volumeId, fsType, diskId);
    std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
    auto &notification = Notification::GetInstance();
    notification.NotifyVolumeChange(VolumeState::DAMAGED, volume);
    EXPECT_EQ(volume->GetId(), volumeId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0007";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0008
 * @tc.name: Notification_NotifyVolumeChange_0008
 * @tc.desc: Test function of NotifyVolumeChange with DAMAGED_MOUNTED state.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0008";
    std::string volumeId = "vol-damaged-2";
    int32_t fsType = FsType::EXFAT;
    std::string diskId = "disk-damaged-2";
    VolumeCore vc(volumeId, fsType, diskId);
    std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
    auto &notification = Notification::GetInstance();
    notification.NotifyVolumeChange(VolumeState::DAMAGED_MOUNTED, volume);
    EXPECT_EQ(volume->GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0008";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0009
 * @tc.name: Notification_NotifyVolumeChange_0009
 * @tc.desc: Test MOUNTED event notification with empty UUID.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0009";
    std::string volumeId = "vol-empty-uuid-1";
    int32_t fsType = FsType::NTFS;
    std::string diskId = "disk-empty-uuid-1";
    std::string fsUuid = "";
    std::string path = "/mnt/data/external/empty-uuid-1";

    VolumeCore vc(volumeId, fsType, diskId);
    std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
    volume->SetFsUuid(fsUuid);
    volume->SetPath(path);

    auto &notification = Notification::GetInstance();
    notification.NotifyVolumeChange(VolumeState::MOUNTED, volume);

    EXPECT_EQ(volume->GetUuid(), "");
    EXPECT_EQ(volume->GetPath(), path);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0009";
}

/**
 * @tc.number: SUB_STORAGE_Notification_NotifyVolumeChange_0010
 * @tc.name: Notification_NotifyVolumeChange_0010
 * @tc.desc: Test MOUNTED event notification with empty path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(NotificationTest, Notification_NotifyVolumeChange_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_NotifyVolumeChange_0010";
    std::string volumeId = "vol-empty-path-1";
    int32_t fsType = FsType::EXFAT;
    std::string diskId = "disk-empty-path-1";
    std::string fsUuid = "uuid-empty-path-1";
    std::string path = "";

    VolumeCore vc(volumeId, fsType, diskId);
    std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);
    volume->SetFsUuid(fsUuid);
    volume->SetPath(path);

    auto &notification = Notification::GetInstance();
    notification.NotifyVolumeChange(VolumeState::MOUNTED, volume);

    EXPECT_EQ(volume->GetPath(), "");
    EXPECT_EQ(volume->GetUuid(), fsUuid);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0010";
}

/**
 * @tc.number: SUB_STORAGE_Notification_VolumeExternal_FreeSize_0001
 * @tc.name: Notification_VolumeExternal_FreeSize_0001
 * @tc.desc: Test VolumeExternal SetFreeSize and GetFreeSize methods.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(NotificationTest, Notification_VolumeExternal_FreeSize_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_VolumeExternal_FreeSize_0001";
    std::string volumeId = "vol-fs-1";
    int32_t fsType = FsType::NTFS;
    std::string diskId = "disk-fs-1";

    VolumeCore vc(volumeId, fsType, diskId);
    std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);

    // Test default freesize is 0
    EXPECT_EQ(volume->GetFreeSize(), 0);

    // Test SetFreeSize and GetFreeSize
    int64_t testFreeSize = 1024 * 1024 * 100;  // 100MB
    volume->SetFreeSize(testFreeSize);
    EXPECT_EQ(volume->GetFreeSize(), testFreeSize);

    // Test updating freesize
    int64_t newFreeSize = 1024 * 1024 * 200;  // 200MB
    volume->SetFreeSize(newFreeSize);
    EXPECT_EQ(volume->GetFreeSize(), newFreeSize);

    // Test setting to zero
    volume->SetFreeSize(0);
    EXPECT_EQ(volume->GetFreeSize(), 0);

    GTEST_LOG_(INFO) << "NotificationTest-end Notification_VolumeExternal_FreeSize_0001";
}

/**
 * @tc.number: SUB_STORAGE_Notification_VolumeExternal_FreeSize_Max_0001
 * @tc.name: Notification_VolumeExternal_FreeSize_Max_0001
 * @tc.desc: Test VolumeExternal FreeSize with maximum values.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(NotificationTest, Notification_VolumeExternal_FreeSize_Max_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationTest-begin Notification_VolumeExternal_FreeSize_Max_0001";
    std::string volumeId = "vol-fs-max-1";
    int32_t fsType = FsType::EXFAT;
    std::string diskId = "disk-fs-max-1";

    VolumeCore vc(volumeId, fsType, diskId);
    std::shared_ptr<VolumeExternal> volume = make_shared<VolumeExternal>(vc);

    // Test with large freesize value (1TB)
    int64_t largeFreeSize = 1024LL * 1024 * 1024 * 1024;
    volume->SetFreeSize(largeFreeSize);
    EXPECT_EQ(volume->GetFreeSize(), largeFreeSize);

    // Test with negative value (error scenario)
    int64_t negativeFreeSize = -1;
    volume->SetFreeSize(negativeFreeSize);
    EXPECT_EQ(volume->GetFreeSize(), negativeFreeSize);

    GTEST_LOG_(INFO) << "NotificationTest-end Notification_VolumeExternal_FreeSize_Max_0001";
}
}
