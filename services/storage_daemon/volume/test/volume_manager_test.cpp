/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "volume/volume_manager.h"
#include "volume_info_mock.h"

namespace OHOS {
namespace StorageDaemon {
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
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    std::string result = volumeManager->CreateVolume(diskId, device);
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
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    std::string result = volumeManager->CreateVolume(diskId, device);
    std::string res = volumeManager->CreateVolume(diskId, device);
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
    dev_t device = MKDEV(1, 2); // 1 is major device number, 2 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device);
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
    dev_t device = MKDEV(1, 3); // 1 is major device number, 3 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device);
    int32_t result = volumeManager->Check(volId);
    EXPECT_EQ(result, E_ERR);

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
    dev_t device = MKDEV(1, 4); // 1 is major device number, 4 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device);
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
    dev_t device = MKDEV(1, 5); // 1 is major device number, 5 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device);
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
    dev_t device = MKDEV(1, 6); // 1 is major device number, 6 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device);
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
    dev_t device = MKDEV(1, 7); // 1 is major device number, 7 is minor device number
    std::string volId = volumeManager->CreateVolume(diskId, device);
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
} // STORAGE_DAEMON
} // OHOS
