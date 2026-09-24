/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifdef DISK_MANAGER
#include "mock/disk_manager_client_mock.h"
#endif
#include "storage_manager_provider.h"
#include "storage_service_constants.h"
#include "storage_service_errno.h"
#include "test/common/help_utils.h"
#include "volume_core.h"
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
namespace OHOS {

int g_pStatus  = -1;
int g_uid = 0;
int32_t g_accessTokenType = 1;

namespace Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    if (g_accessTokenType == -1) {
        return Security::AccessToken::TOKEN_INVALID;
    }
    if (g_accessTokenType == 0) {
        return Security::AccessToken::TOKEN_HAP;
    }
    if (g_accessTokenType == 1) {
        return Security::AccessToken::TOKEN_NATIVE;
    }
    return Security::AccessToken::TOKEN_NATIVE;
}
int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    return g_pStatus ;
}
} // namespace Security::AccessToken

pid_t GetCallingUid()
{
    return g_uid;
}
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
#ifdef DISK_MANAGER
    std::shared_ptr<DiskManager::DiskManagerClientMock> dmClientMock_;
#endif
};

void StorageManagerProviderTest::SetUp(void)
{
    storageManagerProviderTest_ = new StorageManagerProvider(STORAGE_MANAGER_MANAGER_ID);
#ifdef DISK_MANAGER
    dmClientMock_ = std::make_shared<DiskManager::DiskManagerClientMock>();
    DiskManager::IDiskManagerClientMock::diskManagerClientMock = dmClientMock_;
    ON_CALL(*dmClientMock_, Mount(_,_)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, Unmount(_)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, Format(_, _)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, SetVolumeDescription(_, _)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, GetAllVolumes(_)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, GetVolumeByUuid(_, _)).WillByDefault(Return(E_NON_EXIST));
    ON_CALL(*dmClientMock_, GetVolumeById(_, _)).WillByDefault(Return(E_NON_EXIST));
    ON_CALL(*dmClientMock_, GetFreeSizeOfVolume(_, _)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, GetTotalSizeOfVolume(_, _)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, GetAllDisks(_)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, GetDiskById(_, _)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, Partition(_, _)).WillByDefault(Return(E_OK));
    ON_CALL(*dmClientMock_, QueryUsbIsInUse(_, _)).WillByDefault(Return(E_OK));
#endif
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
 * @tc.desc: Verify MountFileMgrFuse returns E_PERMISSION_DENIED when IsCalledByFileMgr fails.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/002";
    int32_t fuseFd = -1;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    EXPECT_EQ(fuseFd, -1);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountFileMgrFuse_001
 * @tc.desc: Verify UMountFileMgrFuse returns E_PERMISSION_DENIED when IsCalledByFileMgr fails.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/002";
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountDlpFuse_001
 * @tc.desc: Verify the MountDlpFuse function with permission granted but invalid path.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountDlpFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDlpFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string dstPath = "../data/service/el1/public/dlp_credential_service/test";
    int32_t fuseFd = -1;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->MountDlpFuse(dstPath, fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    EXPECT_EQ(fuseFd, -1);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDlpFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountDlpFuse_002
 * @tc.desc: Verify the MountDlpFuse function with permission granted but uid not DLP_UID.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountDlpFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDlpFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string dstPath = "/data/service/el1/public/dlp_credential_service/sandbox";
    int32_t fuseFd = -1;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->MountDlpFuse(dstPath, fuseFd);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    EXPECT_EQ(fuseFd, -1);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDlpFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountDlpFuse_001
 * @tc.desc: Verify the UMountDlpFuse function with permission granted but invalid path.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountDlpFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDlpFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string dstPath = "../data/service/el1/public/dlp_credential_service/test";
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->UMountDlpFuse(dstPath);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDlpFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountDlpFuse_002
 * @tc.desc: Verify the UMountDlpFuse function with permission granted but uid not DLP_UID.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountDlpFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDlpFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string dstPath = "/data/service/el1/public/dlp_credential_service/sandbox";
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->UMountDlpFuse(dstPath);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDlpFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_IsFileOccupied_001
 * @tc.desc: Verify the IsFileOccupied function.
 * @tc.type: FUNC
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
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
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
 * @tc.name: StorageManagerProviderTest_Mount_003
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Mount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_003 start";
    std::string volumeId = "testVolumeId";
    auto ret = storageManagerProviderTest_->Mount(volumeId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Unmount_003
 * @tc.desc: Verify the Unmount function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Unmount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_002 start";
    std::string volumeId = "testVolumeId";
    auto ret = storageManagerProviderTest_->Unmount(volumeId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetAllDisks_003
 * @tc.desc: Verify the GetAllDisks function.
 * @tc.type: FUNC
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
 * @tc.name: StorageManagerProviderTest_GetDiskById_003
 * @tc.desc: Verify the GetDiskById function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetDiskById_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetDiskById_003 start";
    std::string diskId = "testDiskId";
    Disk disk;
    auto ret = storageManagerProviderTest_->GetDiskById(diskId, disk);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetDiskById_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetBundleStats_003
 * @tc.desc: Verify the GetBundleStats function.
 * @tc.type: FUNC
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
 * @tc.name: StorageManagerProviderTest_GetFreeSizeOfVolume_003
 * @tc.desc: Verify the GetFreeSizeOfVolume function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFreeSizeOfVolume_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_003 start";
    std::string volumeUuid = "test-volume-uuid";
    int64_t freeSize = 0;
    auto ret = storageManagerProviderTest_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetTotalSizeOfVolume_003
 * @tc.desc: Verify the GetTotalSizeOfVolume function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetTotalSizeOfVolume_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_003 start";
    const std::string volumeUuid = "test_volume_uuid";
    int64_t totalSize = 0;
    auto ret = storageManagerProviderTest_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStatsIpc_003
 * @tc.desc: Verify the GetUserStorageStatsIpc function.
 * @tc.type: FUNC
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
 * @tc.name: StorageManagerProviderTest_NotifyUserChangedEvent_001
 * @tc.desc: Verify the NotifyUserChangedEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyUserChangedEvent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyUserChangedEvent_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    g_pStatus = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    g_uid = 0;
    std::uint32_t userId = 999;
    StorageService::UserChangedEventType enumType = static_cast<StorageService::UserChangedEventType>(4);
    auto ret = storageManagerProviderTest_->NotifyUserChangedEvent(userId, enumType);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    g_uid = 3058; // AccountUid 3058
    ret = storageManagerProviderTest_->NotifyUserChangedEvent(userId, enumType);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    enumType = static_cast<StorageService::UserChangedEventType>(0);
    auto ret = storageManagerProviderTest_->NotifyUserChangedEvent(userId, enumType);
    EXPECT_EQ(ret, E_OK);

    enumType = static_cast<StorageService::UserChangedEventType>(1);
    auto ret = storageManagerProviderTest_->NotifyUserChangedEvent(userId, enumType);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyUserChangedEvent_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetSystemDataSize_002
 * @tc.desc: Verify the GetSystemDataSize function.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetSystemDataSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemDataSize_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    g_uid = 0;
    int64_t systemDataSize = 100;
    auto ret = storageManagerProviderTest_->GetSystemDataSize(systemDataSize);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    g_accessTokenType = 0;
    ret = storageManagerProviderTest_->GetSystemDataSize(systemDataSize);
    EXPECT_EQ(ret, E_SYS_APP_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemDataSize_002 end";
}
} // namespace StorageManager
} // namespace OHOS
