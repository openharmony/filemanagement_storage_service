/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <linux/kdev_t.h>

#include "external_volume_info.h"
#include "external_volume_info_mock.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "volume/volume_manager.h"
#include "volume_info_mock.h"
#include "parameter.h"

namespace {
uint32_t g_FindParameter = 0;
}

uint32_t FindParameter(const char *key)
{
    return g_FindParameter;
}

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class VolumeManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Instance_001
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Instance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Instance_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Instance_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Instance_002
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Instance_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Instance_002 start";

    VolumeManager *volumeManagerFirst = VolumeManager::Instance();
    ASSERT_TRUE(volumeManagerFirst != nullptr);
    VolumeManager *volumeManagerSecond = VolumeManager::Instance();
    ASSERT_TRUE(volumeManagerSecond != nullptr);

    ASSERT_TRUE(volumeManagerFirst == volumeManagerSecond);
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Instance_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_CreateVolume_001
 * @tc.desc: Verify the CreateVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_CreateVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-1";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    std::string result = volumeManager->CreateVolume(diskId, device, isUserdata);
    GTEST_LOG_(INFO) << result;

    volumeManager->DestroyVolume(result);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_CreateVolume_002
 * @tc.desc: Verify the CreateVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_CreateVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-1";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    std::string result = volumeManager->CreateVolume(diskId, device, isUserdata);
    std::string res = volumeManager->CreateVolume(diskId, device, isUserdata);
    GTEST_LOG_(INFO) << result;
    EXPECT_TRUE(res.empty());
    volumeManager->DestroyVolume(result);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_DestroyVolume_001
 * @tc.desc: Verify the DestroyVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_DestroyVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-2";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 2); // 1 is major device number, 2 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device, isUserdata);
    int32_t result = volumeManager->DestroyVolume(volId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_DestroyVolume_002
 * @tc.desc: Verify the DestroyVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_DestroyVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "vol-2-1";
    int32_t result = volumeManager->DestroyVolume(volId);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Check_001
 * @tc.desc: Verify the Check function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Check_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-3";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 3); // 1 is major device number, 3 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device, isUserdata);
    int32_t result = volumeManager->Check(volId);
    EXPECT_EQ(result, E_CHECK);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Check_002
 * @tc.desc: Verify the Check function not existing situation.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Check_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "vol-2-2";
    int32_t result = volumeManager->Check(volId);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Mount_001
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Mount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-4";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 4); // 1 is major device number, 4 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device, isUserdata);
    uint32_t flags = 1; // disk type
    int32_t result = volumeManager->Mount(volId, flags);
    EXPECT_EQ(result, E_VOL_STATE);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Mount_002
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Mount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "vol-2-3";
    uint32_t flags = 1; // disk type
    int32_t result = volumeManager->Mount(volId, flags);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Mount_003
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Mount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_003 start";
    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "mount_test";
    uint32_t flags = 1;
    auto volumeInfoMock = std::make_shared<VolumeInfoMock>();
    volumeInfoMock->id_ = "vol-1-1";
    volumeInfoMock->type_ = EXTERNAL;
    volumeInfoMock->mountState_ = MOUNTED;
    volumeInfoMock->mountFlags_ = 0;
    volumeInfoMock->userIdOwner_ = 100;
    volumeInfoMock->isUserdata_ = false;

    volumeManager->volumes_.Insert(volId, volumeInfoMock);
    EXPECT_CALL(*volumeInfoMock, DoTryToCheck()).WillOnce(Return(0));
    EXPECT_EQ(volumeManager->Mount(volId, flags), 0);

    EXPECT_CALL(*volumeInfoMock, DoTryToCheck()).WillOnce(Return(E_VOL_NEED_FIX));
    EXPECT_EQ(volumeManager->Mount(volId, flags), 0);
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_003 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_UMount_001
 * @tc.desc: Verify the UMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_UMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-5";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 5); // 1 is major device number, 5 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device, isUserdata);
    uint32_t flags = 1; // disk type
    volumeManager->Mount(volId, flags);
    int32_t result = volumeManager->UMount(volId);
    EXPECT_EQ(result, E_OK);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_UMount_002
 * @tc.desc: Verify the UMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_UMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "vol-2-4";
    uint32_t flags = 1; // disk type
    volumeManager->Mount(volId, flags);
    int32_t result = volumeManager->UMount(volId);
    EXPECT_EQ(result, E_NON_EXIST);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Format_001
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Format_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-6";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 6); // 1 is major device number, 6 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device, isUserdata);
    string fsType = "ext2";
    int32_t result = volumeManager->Format(volId, fsType);
    EXPECT_EQ(result, E_NOT_SUPPORT);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Format_002
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Format_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "vol-2-5";
    std::string fsType = "ext2";
    int32_t result = volumeManager->Format(volId, fsType);
    EXPECT_EQ(result, E_NON_EXIST);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_SetVolumeDescription_001
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_SetVolumeDescription_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskId = "diskId-1-7";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 7); // 1 is major device number, 7 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device, isUserdata);
    string description = "description-1";
    int32_t result = volumeManager->SetVolumeDescription(volId, description);
    EXPECT_EQ(result, E_NOT_SUPPORT);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_SetVolumeDescription_002
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_SetVolumeDescription_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "vol-2-6";
    string description = "description-1";
    int32_t result = volumeManager->SetVolumeDescription(volId, description);
    EXPECT_EQ(result, E_NON_EXIST);

    volumeManager->DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_QueryUsbIsInUse_002
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR20250226995120
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_QueryUsbIsInUse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string diskPath = "";
    bool isInUse = true;
    int32_t result = volumeManager->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_TryToFix_001
 * @tc.desc: Verify the TryToFix function.
 * @tc.type: FUNC
 * @tc.require: AR20250226995120
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_TryToFix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_TryToFix_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volId = "volId";
    uint32_t flags = 1;
    volumeManager->volumes_.Clear();
    EXPECT_EQ(volumeManager->TryToFix(volId, flags), E_NON_EXIST);

    auto volumeInfoMock = std::make_shared<VolumeInfoMock>();
    volumeManager->volumes_.Insert(volId, volumeInfoMock);
    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(-1));
    volumeInfoMock->mountState_ = MOUNTED;
    EXPECT_EQ(volumeManager->TryToFix(volId, flags), -1);

    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(0));
    volumeInfoMock->mountState_ = REMOVED;
    EXPECT_EQ(volumeManager->TryToFix(volId, flags), E_VOL_STATE);

    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(0));
    volumeInfoMock->mountState_ = UNMOUNTED;
    EXPECT_CALL(*volumeInfoMock, DoCheck()).WillOnce(Return(-1));
    EXPECT_EQ(volumeManager->TryToFix(volId, flags), -1);

    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(0));
    volumeInfoMock->mountState_ = UNMOUNTED;
    EXPECT_CALL(*volumeInfoMock, DoCheck()).WillOnce(Return(0));
    g_FindParameter = -1;
    EXPECT_CALL(*volumeInfoMock, DoMount(_)).WillOnce(Return(-1));
    EXPECT_EQ(volumeManager->TryToFix(volId, flags), -1);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_TryToFix_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_001
 * @tc.desc: Verify the MountUsbFuse function with non-existing volume.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_001 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volumeId = "vol-non-exist";
    int fuseFd = -1;
    std::string fsUuid;

    // Test with non-existing volume - should return E_NON_EXIST
    int32_t result = volumeManager->MountUsbFuse(volumeId, fsUuid, fuseFd);
    EXPECT_EQ(result, E_NON_EXIST);
    EXPECT_EQ(fuseFd, -1);  // fuseFd should remain unchanged

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_002
 * @tc.desc: Verify the MountUsbFuse function with existing volume but ReadVolumUuid fails.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_002 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    // Create a volume for testing
    std::string diskId = "diskId-usb-fuse-001";
    bool isUserdata = false;
    dev_t device = MKDEV(8, 1); // USB device typically uses major 8
    std::string volumeId = volumeManager->CreateVolume(diskId, device, isUserdata);
    ASSERT_FALSE(volumeId.empty());

    int fuseFd = -1;
    std::string fsUuid;
    
    // Test MountUsbFuse - ReadVolumUuid will likely fail for test volume
    int32_t result = volumeManager->MountUsbFuse(volumeId, fsUuid, fuseFd);
    // Since ReadVolumUuid will fail for test volumes, expect an error
    EXPECT_NE(result, E_OK);

    // Cleanup
    volumeManager->DestroyVolume(volumeId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_003
 * @tc.desc: Verify the MountUsbFuse function with empty volumeId.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_003 start";

    VolumeManager *volumeManager = VolumeManager::Instance();
    ASSERT_TRUE(volumeManager != nullptr);

    std::string volumeId = "";
    int fuseFd = -1;
    std::string fsUuid;
    
    int32_t result = volumeManager->MountUsbFuse(volumeId, fsUuid, fuseFd);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_003 end";
}
} // STORAGE_DAEMON
} // OHOS
