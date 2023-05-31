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
        std::shared_ptr<Notification> notification = DelayedSingleton<Notification>::GetInstance();
        notification->NotifyVolumeChange(VolumeState::REMOVED, volume);
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
        std::shared_ptr<Notification> notification = DelayedSingleton<Notification>::GetInstance();
        notification->NotifyVolumeChange(VolumeState::UNMOUNTED, volume);
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
        std::shared_ptr<Notification> notification = DelayedSingleton<Notification>::GetInstance();
        notification->NotifyVolumeChange(VolumeState::MOUNTED, volume);
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
        std::shared_ptr<Notification> notification = DelayedSingleton<Notification>::GetInstance();
        notification->NotifyVolumeChange(VolumeState::BAD_REMOVAL, volume);
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
        std::shared_ptr<Notification> notification = DelayedSingleton<Notification>::GetInstance();
        notification->NotifyVolumeChange(VolumeState::EJECTING, volume);
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
        std::shared_ptr<Notification> notification = DelayedSingleton<Notification>::GetInstance();
        notification->NotifyVolumeChange(VolumeState::CHECKING, volume);
        EXPECT_EQ(vc.GetDiskId(), diskId);
    GTEST_LOG_(INFO) << "NotificationTest-end Notification_NotifyVolumeChange_0005";
}
}
