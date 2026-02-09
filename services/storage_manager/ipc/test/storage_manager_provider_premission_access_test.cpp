/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "disk.h"
#include "message_parcel.h"
#include "mock/uece_activation_callback_mock.h"
#include "storage_manager_provider.h"
#include "storage_service_errno.h"
#include "test/common/help_utils.h"
#include "volume_core.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
namespace OHOS {

int g_pStatus  = -1;

namespace Security::AccessToken {
int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    return g_pStatus ;
}
} // namespace Security::AccessToken
namespace StorageManager {
using namespace testing;
using namespace testing::ext;

pid_t g_testCallingUid = 5523;
class ScopedTestUid {
public:
    explicit ScopedTestUid(pid_t newUid) : oldUid(g_testCallingUid) { g_testCallingUid = newUid; }
    ~ScopedTestUid() { g_testCallingUid = oldUid; }
private:
    pid_t oldUid;
};

class StorageManagerProviderTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp();
    void TearDown();

    StorageManagerProvider *storageManagerProviderTest_;
};

void StorageManagerProviderTest::SetUp(void)
{
    storageManagerProviderTest_ = new StorageManagerProvider(STORAGE_MANAGER_MANAGER_ID);
}

void StorageManagerProviderTest::TearDown(void)
{
    if (storageManagerProviderTest_ != nullptr) {
        delete storageManagerProviderTest_;
        storageManagerProviderTest_ = nullptr;
    }
}

/**
 * @tc.name: StorageManagerProviderTest_MountFileMgrFuse_001
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "../mnt/mtp/device/storage/usb";
    int32_t fuseFd = -1;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    EXPECT_EQ(fuseFd, -1);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountFileMgrFuse_002
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountFileMgrFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/002";
    int32_t fuseFd = -1;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    EXPECT_EQ(fuseFd, -1);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountFileMgrFuse_001
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "../mnt/mtp/device/storage/usb";
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountFileMgrFuse_002
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountFileMgrFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/002";
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_IsFileOccupied_001
 * @tc.desc: Verify the IsFileOccupied function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_IsFileOccupied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFileOccupied_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string path = "../test/file";
    std::vector<std::string> inputList = {"file1", "file2"};
    std::vector<std::string> outputList;
    bool isOccupy = false;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->IsFileOccupied(path, inputList, outputList, isOccupy);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFileOccupied_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_QueryUsbIsInUse_001
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_QueryUsbIsInUse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string diskPath = "../dev/sda1";
    bool isInUse = false;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string diskPath = "../dev/sda1";
    std::string diskPathSec = "/dev/sda1";
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    std::uint32_t userId = 100;
    std::uint32_t level = 2;
    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(userId, diskPath, level);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    ret = storageManagerProviderTest_->SetDirEncryptionPolicy(userId, diskPathSec, level);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeCreated_003
 * @tc.desc: Verify the NotifyVolumeCreated function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeCreated_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_003 start";
    VolumeCore volumeCore;
    auto ret = storageManagerProviderTest_->NotifyVolumeCreated(volumeCore);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeMounted_003
 * @tc.desc: Verify the NotifyVolumeMounted function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeMounted_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_003 start";
    std::string volumeId = "testVolumeId";
    std::string fsType = "ext4";
    std::string fsUuid = "testFsUuid";
    std::string path = "/mnt/testVolume";
    std::string description = "Test Volume";
    auto ret = storageManagerProviderTest_->NotifyVolumeMounted(
        VolumeInfoStr{volumeId, fsType, fsUuid, path, description, false});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_002
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_002 start";
    ScopedTestUid uidGuard(1009);
    std::string volId = "vol-8-1";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-1";
    std::string path = "/";
    std::string description = "My Disk";
    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeStateChanged_003
 * @tc.desc: Verify the NotifyVolumeStateChanged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeStateChanged_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_003 start";
    std::string volumeId = "testVolumeId";
    uint32_t state = DAMAGED_MOUNTED;
    auto ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeStateChanged_004
 * @tc.desc: Verify the NotifyVolumeStateChanged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeStateChanged_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_004 start";
    std::string volumeId = "testVolumeId";
    uint32_t state = DAMAGED;
    auto ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = UNMOUNTED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = CHECKING;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = EJECTING;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = REMOVED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = BAD_REMOVAL;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = FUSE_REMOVED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = DAMAGED_MOUNTED + DAMAGED_MOUNTED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    state = MOUNTED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Mount_003
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Mount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_002 start";
    std::string volumeId = "testVolumeId";
    auto ret = storageManagerProviderTest_->Mount(volumeId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Unmount_003
 * @tc.desc: Verify the Unmount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Unmount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_002 start";
    std::string volumeId = "testVolumeId";
    auto ret = storageManagerProviderTest_->Unmount(volumeId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_TryToFix_002
 * @tc.desc: Verify the TryToFix function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_TryToFix_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_TryToFix_002 start";
    ScopedTestUid uidGuard(1009);
    std::string volId = "vol-8-1";
    std::string fsTypeStr = "exfat";
    std::string uuid = "uuid-1";
    std::string path = "/";
    std::string description = "My Disk";

    auto ret = storageManagerProviderTest_->TryToFix(volId);
    EXPECT_NE(ret, E_ERR);

    int32_t fsType = 1;
    std::string diskId = "disk-1-6";
    VolumeCore vc(volId, fsType, diskId);
    storageManagerProviderTest_->NotifyVolumeCreated(vc);
    ret = storageManagerProviderTest_->TryToFix(volId);
    EXPECT_NE(ret, E_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_TryToFix_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyDiskCreated_003
 * @tc.desc: Verify the NotifyDiskCreated function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyDiskCreated_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskCreated_003 start";
    Disk disk;
    auto ret = storageManagerProviderTest_->NotifyDiskCreated(disk);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskCreated_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyDiskDestroyed_003
 * @tc.desc: Verify the NotifyDiskDestroyed function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyDiskDestroyed_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskDestroyed_003 start";
    std::string diskId = "testDiskId";
    auto ret = storageManagerProviderTest_->NotifyDiskDestroyed(diskId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskDestroyed_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Partition_002
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Partition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Partition_002 start";
    std::string diskId = "testDiskId";
    int32_t type = 1;
    auto ret = storageManagerProviderTest_->Partition(diskId, type);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Partition_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetAllDisks_003
 * @tc.desc: Verify the GetAllDisks function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetAllDisks_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllDisks_003 start";
    std::vector<Disk> disks;
    auto ret = storageManagerProviderTest_->GetAllDisks(disks);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllDisks_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetVolumeByUuid_003
 * @tc.desc: Verify the GetVolumeByUuid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetVolumeByUuid_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeByUuid_003 start";
    std::string fsUuid = "testUuid";
    VolumeExternal volume;
    auto ret = storageManagerProviderTest_->GetVolumeByUuid(fsUuid, volume);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeByUuid_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetVolumeById_003
 * @tc.desc: Verify the GetVolumeById function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetVolumeById_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeById_003 start";
    std::string volumeId = "testVolumeId";
    VolumeExternal volume;
    auto ret = storageManagerProviderTest_->GetVolumeById(volumeId, volume);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeById_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetVolumeDescription_003
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetVolumeDescription_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetVolumeDescription_003 start";
    std::string fsUuid = "testUuid";
    std::string description = "Test Volume Description";
    auto ret = storageManagerProviderTest_->SetVolumeDescription(fsUuid, description);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetVolumeDescription_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Format_003
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Format_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Format_003 start";
    std::string volumeId = "testVolumeId";
    std::string fsType = "ext4";
    auto ret = storageManagerProviderTest_->Format(volumeId, fsType);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Format_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetDiskById_003
 * @tc.desc: Verify the GetDiskById function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetDiskById_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetDiskById_003 start";
    std::string diskId = "testDiskId";
    Disk disk;
    auto ret = storageManagerProviderTest_->GetDiskById(diskId, disk);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetDiskById_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetBundleStats_003
 * @tc.desc: Verify the GetBundleStats function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetBundleStats_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_003 start";
    std::string pkgName = "com.example.test";
    BundleStats bundleStats;
    int32_t appIndex = 0;
    uint32_t statFlag = 0x01;
    auto ret = storageManagerProviderTest_->GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
    EXPECT_NE(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetSystemSize_003
 * @tc.desc: Verify the GetSystemSize function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetSystemSize_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_003 start";
    int64_t systemSize = 0;
    auto ret = storageManagerProviderTest_->GetSystemSize(systemSize);
    EXPECT_NE(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetTotalSize_003
 * @tc.desc: Verify the GetTotalSize function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetTotalSize_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSize_003 start";
    int64_t totalSize = 0;
    auto ret = storageManagerProviderTest_->GetTotalSize(totalSize);
    EXPECT_NE(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSize_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStats_003
 * @tc.desc: Verify the GetUserStorageStats function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStats_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_003 start";
    StorageStats storageStats;
    auto ret = storageManagerProviderTest_->GetUserStorageStats(storageStats);
    EXPECT_NE(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_QueryUsbIsInUse_003
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_QueryUsbIsInUse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_003 start";
    std::string diskPath = "/dev/sda1";
    bool isInUse = false;
    auto ret = storageManagerProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_NE(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetFreeSizeOfVolume_003
 * @tc.desc: Verify the GetFreeSizeOfVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFreeSizeOfVolume_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_003 start";
    std::string volumeUuid = "test-volume-uuid";
    int64_t freeSize = 0;
    auto ret = storageManagerProviderTest_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetTotalSizeOfVolume_003
 * @tc.desc: Verify the GetTotalSizeOfVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetTotalSizeOfVolume_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_003 start";
    const std::string volumeUuid = "test_volume_uuid";
    int64_t totalSize = 0;
    auto ret = storageManagerProviderTest_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStatsIpc_003
 * @tc.desc: Verify the GetUserStorageStatsIpc function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStatsIpc_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsIpc_003 start";
    int32_t userId = 1012;
    StorageStats storageStats;
    auto ret = storageManagerProviderTest_->GetUserStorageStats(userId, storageStats);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsIpc_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStatsByType_003
 * @tc.desc: Verify the GetUserStorageStatsByType function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStatsByType_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsByType_003 start";
    ScopedTestUid uidGuard(1089);
    int32_t userId = 1002;
    StorageStats storageStats;
    std::string type = "exampleType";
    auto ret = storageManagerProviderTest_->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsByType_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_IsFilePathInvalid_001
 * @tc.desc: Verify the IsFilePathInvalid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_IsFilePathInvalid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFilePathInvalid_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    const std::array<std::tuple<std::string, bool>, 15> testCases = {
        std::make_tuple<std::string, bool>("/valid/path/file.txt", true),
        std::make_tuple<std::string, bool>("../invalid/path", false),
        std::make_tuple<std::string, bool>("/valid/../path", false),
        std::make_tuple<std::string, bool>("/path/to/..", false),
        std::make_tuple<std::string, bool>("", true),
        std::make_tuple<std::string, bool>("/", true),
        std::make_tuple<std::string, bool>("../a/../b", false),
        std::make_tuple<std::string, bool>("a/../b", false),
        std::make_tuple<std::string, bool>("../a/b", false),
        std::make_tuple<std::string, bool>("//../a", false),
        std::make_tuple<std::string, bool>("/valid%path/file.txt", true),
        std::make_tuple<std::string, bool>("a../b", true),
        std::make_tuple<std::string, bool>("../", false),
        std::make_tuple<std::string, bool>("/path/..more", true),
        std::make_tuple<std::string, bool>("/a/..b", true)};
    for (auto testCase : testCases) {
        const std::string &input = std::get<0>(testCase);
        const bool expected = std::get<1>(testCase);
        EXPECT_NE(storageManagerProviderTest_->IsFilePathInvalid(input), expected);
    }
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFilePathInvalid_001 end";
}
} // namespace StorageManager
} // namespace OHOS
