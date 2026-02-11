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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "bundle_manager_connector.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "disk.h"
#include "ext_bundle_stats.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "storage_manager_provider.h"
#include "storage_service_errno.h"
#include "test/common/help_utils.h"
#include "mock/uece_activation_callback_mock.h"
#include "bundle_mgr_client.h"
#include "volume_core.h"
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <cstdint>
int32_t g_accessTokenType = 1;
namespace OHOS::Security::AccessToken {
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

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
{
    nativeTokenInfoRes.processName = "foundation";
    return 0;
}
}

pid_t g_testCallingUid = 5523;
const uint32_t TOP_USER_ID = 10738;
pid_t g_testUpdateServiceUid = 6666;
bool g_returnUpdateService = false;
namespace OHOS {
pid_t IPCSkeleton::GetCallingUid()
{
    if (g_returnUpdateService) {
        return g_testUpdateServiceUid;
    }
    return g_testCallingUid;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    uint32_t callingTokenID = 100;
    return callingTokenID;
}
}

namespace OHOS::AppExecFwk {
ErrCode BundleMgrClient::CreateBundleDataDirWithEl(int32_t userId, DataDirEl dirEl)
{
    return 0;
}
} // namespace OHOS::AppExecFwk

namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;
class StorageManagerProviderTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp();
    void TearDown();

    StorageManagerProvider *storageManagerProviderTest_;
};

class MockBundleMgr : public AppExecFwk::IBundleMgr {
public:
    bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        bundleName = "com.example.fake";
        return true;
    }
    sptr<IRemoteObject> AsObject() override { return nullptr; }
};

BundleMgrConnector::BundleMgrConnector() {}
BundleMgrConnector::~BundleMgrConnector() {}
sptr<AppExecFwk::IBundleMgr> g_testBundleMgrProxy = nullptr;
#ifdef STORAGE_MANAGER_UNIT_TEST
sptr<AppExecFwk::IBundleMgr> BundleMgrConnector::GetBundleMgrProxy()
{
    return g_testBundleMgrProxy;
}
#endif

void StorageManagerProviderTest::SetUp(void)
{
    g_returnUpdateService = false;
    g_accessTokenType = 1;
    storageManagerProviderTest_ = new StorageManagerProvider(STORAGE_MANAGER_MANAGER_ID);
}

void StorageManagerProviderTest::TearDown(void)
{
    if (storageManagerProviderTest_ != nullptr) {
        delete storageManagerProviderTest_;
        storageManagerProviderTest_ = nullptr;
    }
}

void StringVecToRawData(const std::vector<std::string> &stringVec, StorageFileRawData &rawData)
{
    std::stringstream ss;
    uint32_t stringCount = stringVec.size();
    ss.write(reinterpret_cast<const char*>(&stringCount), sizeof(stringCount));

    for (uint32_t i = 0; i < stringCount; ++i) {
        uint32_t strLen = stringVec[i].length();
        ss.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        ss.write(stringVec[i].c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}

class ScopedTestUid {
public:
    explicit ScopedTestUid(pid_t newUid) : oldUid(g_testCallingUid) { g_testCallingUid = newUid; }
    ~ScopedTestUid() { g_testCallingUid = oldUid; }
private:
    pid_t oldUid;
};

/**
 * @tc.name: StorageManagerProviderTest_PrepareAddUser_002
 * @tc.desc: Verify the PrepareAddUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareAddUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 123;
    uint32_t flags = 1;

    auto ret = storageManagerProviderTest_->PrepareAddUser(userId, flags);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->PrepareAddUser(userId, flags);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_RemoveUser_002
 * @tc.desc: Verify the RemoveUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RemoveUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 123;
    uint32_t flags = 1;

    auto ret = storageManagerProviderTest_->RemoveUser(userId, flags);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->RemoveUser(userId, flags);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareStartUser_002
 * @tc.desc: Verify the PrepareStartUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareStartUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 123;

    auto ret = storageManagerProviderTest_->PrepareStartUser(userId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->PrepareStartUser(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_StopUser_002
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_StopUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 123;

    auto ret = storageManagerProviderTest_->StopUser(userId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->StopUser(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CompleteAddUser_002
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CompleteAddUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CompleteAddUser_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 123;

    auto ret = storageManagerProviderTest_->CompleteAddUser(userId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->CompleteAddUser(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CompleteAddUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetFreeSizeOfVolume_002
 * @tc.desc: Verify the GetFreeSizeOfVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFreeSizeOfVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeUuid = "test-volume-uuid";
    int64_t freeSize = 0;
    auto ret = storageManagerProviderTest_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetTotalSizeOfVolume_002
 * @tc.desc: Verify the GetTotalSizeOfVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetTotalSizeOfVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    const std::string volumeUuid = "test_volume_uuid";
    int64_t totalSize = 0;
    auto ret = storageManagerProviderTest_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetBundleStats_002
 * @tc.desc: Verify the GetBundleStats function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetBundleStats_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string pkgName = "com.example.test";
    BundleStats bundleStats;
    int32_t appIndex = 0;
    uint32_t statFlag = 0x01;
    auto ret = storageManagerProviderTest_->GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetSystemSize_002
 * @tc.desc: Verify the GetSystemSize function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetSystemSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int64_t systemSize = 0;
    auto ret = storageManagerProviderTest_->GetSystemSize(systemSize);
    EXPECT_EQ(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicyk_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicyk_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 100;
    std::string dirPath = "/test";
    uint32_t type = 2;
    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(userId, dirPath, type);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->SetDirEncryptionPolicy(userId, dirPath, type);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicyk_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStats_002
 * @tc.desc: Verify the GetUserStorageStats function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStats_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    StorageStats storageStats;
    auto ret = storageManagerProviderTest_->GetUserStorageStats(storageStats);
    EXPECT_EQ(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStatsIpc_002
 * @tc.desc: Verify the GetUserStorageStatsIpc function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStatsIpc_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsIpc_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1012;
    StorageStats storageStats;
    auto ret = storageManagerProviderTest_->GetUserStorageStats(userId, storageStats);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsIpc_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeCreated_002
 * @tc.desc: Verify the NotifyVolumeCreated function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeCreated_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    VolumeCore volumeCore;
    auto ret = storageManagerProviderTest_->NotifyVolumeCreated(volumeCore);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeMounted_002
 * @tc.desc: Verify the NotifyVolumeMounted function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeMounted_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeId = "testVolumeId";
    std::string fsType = "ext4";
    std::string fsUuid = "testFsUuid";
    std::string path = "/mnt/testVolume";
    std::string description = "Test Volume";
    auto ret = storageManagerProviderTest_->NotifyVolumeMounted(
        VolumeInfoStr{volumeId, fsType, fsUuid, path, description, false});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeMounted_003
 * @tc.desc: Verify the NotifyVolumeMounted function when isDamaged is true.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeMounted_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_003 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeId = "testVolumeId";
    std::string fsType = "ext4";
    std::string fsUuid = "testFsUuid";
    std::string path = "/mnt/testVolume";
    std::string description = "Test Volume";
    auto ret = storageManagerProviderTest_->NotifyVolumeMounted(
        VolumeInfoStr{volumeId, fsType, fsUuid, path, description, true});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeStateChanged_002
 * @tc.desc: Verify the NotifyVolumeStateChanged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeStateChanged_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeId = "testVolumeId";
    uint32_t state = MOUNTED;
    auto ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Mount_002
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Mount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeId = "testVolumeId";
    auto ret = storageManagerProviderTest_->Mount(volumeId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Unmount_002
 * @tc.desc: Verify the Unmount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Unmount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeId = "testVolumeId";
    auto ret = storageManagerProviderTest_->Unmount(volumeId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetAllVolumes_002
 * @tc.desc: Verify the GetAllVolumes function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetAllVolumes_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllVolumes_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::vector<VolumeExternal> volumes;
    auto ret = storageManagerProviderTest_->GetAllVolumes(volumes);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllVolumes_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyDiskCreated_002
 * @tc.desc: Verify the NotifyDiskCreated function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyDiskCreated_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskCreated_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    Disk disk;
    auto ret = storageManagerProviderTest_->NotifyDiskCreated(disk);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskCreated_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyDiskDestroyed_002
 * @tc.desc: Verify the NotifyDiskDestroyed function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyDiskDestroyed_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskDestroyed_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string diskId = "testDiskId";
    auto ret = storageManagerProviderTest_->NotifyDiskDestroyed(diskId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyDiskDestroyed_002 end";
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
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string diskId = "testDiskId";
    int32_t type = 1;
    auto ret = storageManagerProviderTest_->Partition(diskId, type);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Partition_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetAllDisks_002
 * @tc.desc: Verify the GetAllDisks function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetAllDisks_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllDisks_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::vector<Disk> disks;
    auto ret = storageManagerProviderTest_->GetAllDisks(disks);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllDisks_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetVolumeByUuid_002
 * @tc.desc: Verify the GetVolumeByUuid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetVolumeByUuid_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeByUuid_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string fsUuid = "testUuid";
    VolumeExternal volume;
    auto ret = storageManagerProviderTest_->GetVolumeByUuid(fsUuid, volume);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeByUuid_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetVolumeById_002
 * @tc.desc: Verify the GetVolumeById function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetVolumeById_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeById_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeId = "testVolumeId";
    VolumeExternal volume;
    auto ret = storageManagerProviderTest_->GetVolumeById(volumeId, volume);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetVolumeById_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetVolumeDescription_002
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetVolumeDescription_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetVolumeDescription_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string fsUuid = "testUuid";
    std::string description = "Test Volume Description";
    auto ret = storageManagerProviderTest_->SetVolumeDescription(fsUuid, description);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetVolumeDescription_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Format_002
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Format_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Format_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volumeId = "testVolumeId";
    std::string fsType = "ext4";
    auto ret = storageManagerProviderTest_->Format(volumeId, fsType);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Format_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetDiskById_002
 * @tc.desc: Verify the GetDiskById function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetDiskById_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetDiskById_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string diskId = "testDiskId";
    Disk disk;
    auto ret = storageManagerProviderTest_->GetDiskById(diskId, disk);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetDiskById_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_QueryUsbIsInUse_002
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_QueryUsbIsInUse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string diskPath = "/dev/sda1";
    bool isInUse = false;
    auto ret = storageManagerProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_EQ(ret, E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_EraseAllUserEncryptedKeys_002
 * @tc.desc: Verify the EraseAllUserEncryptedKeys when callingUid not UPDATE_SERVICE_UID.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_EraseAllUserEncryptedKeys_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto ret = storageManagerProviderTest_->EraseAllUserEncryptedKeys();
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_EraseAllUserEncryptedKeys_003
 * @tc.desc: Verify the EraseAllUserEncryptedKeys when service is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_EraseAllUserEncryptedKeys_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_003 start";
    g_returnUpdateService = true;
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto ret = storageManagerProviderTest_->EraseAllUserEncryptedKeys();
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateUserAuth_002
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateUserAuth_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserAuth_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    uint64_t secureUid = 123456789;
    std::vector<uint8_t> token = {1, 2, 3, 4};
    std::vector<uint8_t> oldSecret = {5, 6, 7, 8};
    std::vector<uint8_t> newSecret = {9, 10, 11, 12};
    auto ret = storageManagerProviderTest_->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    uint32_t result = storageManagerProviderTest_->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserAuth_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_002
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::vector<uint8_t> authToken = {1, 2, 3, 4};
    std::vector<uint8_t> newSecret = {5, 6, 7, 8};
    uint64_t secureUid = 123456789;
    uint32_t userId = 1012;
    std::vector<std::vector<uint8_t>> plainText = {{9, 10}, {11, 12}};
    auto ret =
        storageManagerProviderTest_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ActiveUserKey_002
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ActiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ActiveUserKey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    std::vector<uint8_t> token = {1, 2, 3, 4};
    std::vector<uint8_t> secret = {5, 6, 7, 8};
    auto ret = storageManagerProviderTest_->ActiveUserKey(userId, token, secret);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->ActiveUserKey(userId, token, secret);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ActiveUserKey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_InactiveUserKey_002
 * @tc.desc: Verify the InactiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_InactiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_InactiveUserKey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    auto ret = storageManagerProviderTest_->InactiveUserKey(userId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->InactiveUserKey(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_InactiveUserKey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetFileEncryptStatus_002
 * @tc.desc: Verify the GetFileEncryptStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFileEncryptStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFileEncryptStatus_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    bool isEncrypted = false;
    bool needCheckDirMount = true;
    auto ret = storageManagerProviderTest_->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFileEncryptStatus_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserNeedActiveStatus_002
 * @tc.desc: Verify the GetUserNeedActiveStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserNeedActiveStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserNeedActiveStatus_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    bool needActive = false;
    auto ret = storageManagerProviderTest_->GetUserNeedActiveStatus(userId, needActive);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->GetUserNeedActiveStatus(userId, needActive);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserNeedActiveStatus_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UnlockUserScreen_002
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UnlockUserScreen_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnlockUserScreen_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    std::vector<uint8_t> token = {0x01, 0x02, 0x03};
    std::vector<uint8_t> secret = {0x04, 0x05, 0x06};
    auto ret = storageManagerProviderTest_->UnlockUserScreen(userId, token, secret);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->UnlockUserScreen(userId, token, secret);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnlockUserScreen_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_LockUserScreen_002
 * @tc.desc: Verify the LockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_LockUserScreen_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_LockUserScreen_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = nullptr;
    uint32_t userId = 1001;
    auto ret = storageManagerProviderTest_->LockUserScreen(userId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_LockUserScreen_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_LockUserScreen_003
 * @tc.desc: Verify the LockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_LockUserScreen_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_LockUserScreen_003 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    uint32_t userId = 1001;
    auto ret = storageManagerProviderTest_->LockUserScreen(userId);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_LockUserScreen_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetLockScreenStatus_002
 * @tc.desc: Verify the GetLockScreenStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetLockScreenStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetLockScreenStatus_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    bool needActive = false;
    auto ret = storageManagerProviderTest_->GetLockScreenStatus(userId, needActive);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->GetLockScreenStatus(userId, needActive);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetLockScreenStatus_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GenerateAppkey_002
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GenerateAppkey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GenerateAppkey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t hashId = 12345;
    uint32_t userId = 1012;
    std::string keyId;
    bool needReSet = false;
    auto ret = storageManagerProviderTest_->GenerateAppkey(hashId, userId, keyId, needReSet);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->GenerateAppkey(hashId, userId, keyId, needReSet);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GenerateAppkey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_DeleteAppkey_002
 * @tc.desc: Verify the DeleteAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_DeleteAppkey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteAppkey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string keyId = "testKeyId";
    auto ret = storageManagerProviderTest_->DeleteAppkey(keyId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteAppkey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CreateRecoverKey_002
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CreateRecoverKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateRecoverKey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    uint32_t userType = 1;
    std::vector<uint8_t> token = {0x01, 0x02, 0x03};
    std::vector<uint8_t> secret = {0xAA, 0xBB, 0xCC};
    auto ret = storageManagerProviderTest_->CreateRecoverKey(userId, userType, token, secret);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->CreateRecoverKey(userId, userType, token, secret);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateRecoverKey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetRecoverKey_002
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetRecoverKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetRecoverKey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::vector<uint8_t> key = {0x01, 0x02, 0x03};
    auto ret = storageManagerProviderTest_->SetRecoverKey(key);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetRecoverKey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateKeyContext_002
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateKeyContext_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateKeyContext_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1012;
    bool needRemoveTmpKey = true;
    auto ret = storageManagerProviderTest_->UpdateKeyContext(userId, needRemoveTmpKey);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->UpdateKeyContext(userId, needRemoveTmpKey);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateKeyContext_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetBundleQuota_002
 * @tc.desc: Verify the SetBundleQuota function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetBundleQuota_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetBundleQuota_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string bundleName = "com.example.app";
    int32_t uid = 1002;
    std::string bundleDataDirPath = "/data/app/example";
    int32_t limitSizeMb = 1024;
    auto ret = storageManagerProviderTest_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetBundleQuota_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyMtpMounted_002
 * @tc.desc: Verify the NotifyMtpMounted function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyMtpMounted_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyMtpMounted_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string id = "mtpId";
    std::string path = "/mnt/mtp";
    std::string desc = "MTP Device";
    std::string uuid = "1234-5678";
    auto ret = storageManagerProviderTest_->NotifyMtpMounted(id, path, desc, uuid);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyMtpMounted_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyMtpUnmounted_002
 * @tc.desc: Verify the NotifyMtpUnmounted function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyMtpUnmounted_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyMtpUnmounted_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string id = "mtpId";
    bool isBadRemove = false;
    auto ret = storageManagerProviderTest_->NotifyMtpUnmounted(id, isBadRemove);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyMtpUnmounted_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountFileMgrFuse_002
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountFileMgrFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1002;
    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/002";
    int32_t fuseFd = -1;
    auto ret = storageManagerProviderTest_->MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    EXPECT_EQ(fuseFd, -1);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountFileMgrFuse_003
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountFileMgrFuse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_003 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    int32_t userId = 1002;

    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/003";
    int32_t fuseFd = -1;
    auto ret = storageManagerProviderTest_->MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    EXPECT_EQ(fuseFd, -1);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountFileMgrFuse_002
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountFileMgrFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1002;
    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/002";
    auto ret = storageManagerProviderTest_->UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountFileMgrFuse_003
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountFileMgrFuse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_003 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    int32_t userId = 1002;
    std::string path = "/mnt/data/" + std::to_string(userId) + "/userExternal/002";
    auto ret = storageManagerProviderTest_->UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_IsFileOccupied_002
 * @tc.desc: Verify the IsFileOccupied function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_IsFileOccupied_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFileOccupied_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string path = "/test/file";
    std::vector<std::string> inputList = {"file1", "file2"};
    std::vector<std::string> outputList;
    bool isOccupy = false;
    auto ret = storageManagerProviderTest_->IsFileOccupied(path, inputList, outputList, isOccupy);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFileOccupied_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ResetSecretWithRecoveryKey_002
 * @tc.desc: Verify the ResetSecretWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ResetSecretWithRecoveryKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ResetSecretWithRecoveryKey_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 1002;
    uint32_t rkType = 1;
    std::vector<uint8_t> key = {0x01, 0x02, 0x03};
    auto ret = storageManagerProviderTest_->ResetSecretWithRecoveryKey(userId, rkType, key);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    userId = TOP_USER_ID + 1;
    ret = storageManagerProviderTest_->ResetSecretWithRecoveryKey(userId, rkType, key);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ResetSecretWithRecoveryKey_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CreateShareFile_002
 * @tc.desc: Verify the CreateShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CreateShareFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateShareFile_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string uriStr = "file1";
    std::vector<std::string> uriStrVec = {uriStr};
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 12345;
    uint32_t flag = 1;
    std::vector<int32_t> funcResult;
    auto ret = storageManagerProviderTest_->CreateShareFile(fileRawData, tokenId, flag, funcResult);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateShareFile_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_DeleteShareFile_002
 * @tc.desc: Verify the DeleteShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_DeleteShareFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteShareFile_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string uriStr = "file1";
    std::vector<std::string> uriStrVec = {uriStr};
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 12345;
    auto ret = storageManagerProviderTest_->DeleteShareFile(tokenId, fileRawData);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteShareFile_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStatsByType_002
 * @tc.desc: Verify the GetUserStorageStatsByType function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStatsByType_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsByType_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1089);
    int32_t userId = 1002;
    StorageStats storageStats;
    std::string type = "exampleType";
    auto ret = storageManagerProviderTest_->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStatsByType_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountDfsDocs_002
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountDfsDocs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDfsDocs_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1009);
    int32_t userId = 1002;
    std::string relativePath = "/relative/path";
    std::string networkId = "network123";
    std::string deviceId = "device123";
    auto ret = storageManagerProviderTest_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDfsDocs_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountDfsDocs_002
 * @tc.desc: Verify the UMountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountDfsDocs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDfsDocs_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1009);
    int32_t userId = 1002;
    std::string relativePath = "/relative/path";
    std::string networkId = "network123";
    std::string deviceId = "device123";
    auto ret = storageManagerProviderTest_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDfsDocs_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountMediaFuse_002
 * @tc.desc: Verify the MountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountMediaFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountMediaFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = nullptr;
    int32_t userId = 1001;
    int32_t devFd = -1;
    auto ret = storageManagerProviderTest_->MountMediaFuse(userId, devFd);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountMediaFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountMediaFuse_003
 * @tc.desc: Verify the MountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountMediaFuse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountMediaFuse_003 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    int32_t userId = 1001;
    int32_t devFd = -1;
    auto ret = storageManagerProviderTest_->MountMediaFuse(userId, devFd);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountMediaFuse_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountMediaFuse_002
 * @tc.desc: Verify the UMountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountMediaFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountMediaFuse_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = nullptr;
    int32_t userId = 1001;
    auto ret = storageManagerProviderTest_->UMountMediaFuse(userId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountMediaFuse_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountMediaFuse_003
 * @tc.desc: Verify the UMountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountMediaFuse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountMediaFuse_003 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    int32_t userId = 1001;
    auto ret = storageManagerProviderTest_->UMountMediaFuse(userId);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountMediaFuse_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountDisShareFile_002
 * @tc.desc: Verify the MountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountDisShareFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDisShareFile_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1009);
    int32_t userId = -1;
    std::map<std::string, std::string> shareFiles = {{{"/data/sharefile1", "/data/sharefile2"}}};
    auto ret = storageManagerProviderTest_->MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    userId = 100;
    shareFiles = {{{"../", "../"}}};
    ret = storageManagerProviderTest_->MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    shareFiles = {{{"/data/sharefile1", "/data/sharefile2"}}};
    ret = storageManagerProviderTest_->MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDisShareFile_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountDisShareFile_002
 * @tc.desc: Verify the UMountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountDisShareFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDisShareFile_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1009);
    int32_t userId = -1;
    std::string networkId = "sharefile1";
    auto ret = storageManagerProviderTest_->UMountDisShareFile(userId, networkId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    userId = 100;
    networkId = "../";
    ret = storageManagerProviderTest_->UMountDisShareFile(userId, networkId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    networkId = "sharefile1";
    ret = storageManagerProviderTest_->UMountDisShareFile(userId, networkId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDisShareFile_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_001
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1009);
    std::string volId = "vol-8-1";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-1";
    std::string path = "/";
    std::string description = "My Disk";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);

    int32_t fsType = 1;
    std::string diskId = "disk-1-6";
    VolumeCore vc(volId, fsType, diskId);
    storageManagerProviderTest_->NotifyVolumeCreated(vc);
    ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);

    int64_t sizeBytes = 1024;
    std::string vendor = "vendor-1";
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, path, vendor, 1);
    storageManagerProviderTest_->NotifyDiskCreated(disk);
    ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_001 end";
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
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1009);
    std::string volId = "vol-8-1";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-1";
    std::string path = "/";
    std::string description = "My Disk";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);

    int32_t fsType = 1;
    std::string diskId = "disk-1-6";
    VolumeCore vc(volId, fsType, diskId);
    storageManagerProviderTest_->NotifyVolumeCreated(vc);
    ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);

    int64_t sizeBytes = 1024;
    std::string vendor = "vendor-1";
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, path, vendor, 1);
    storageManagerProviderTest_->NotifyDiskCreated(disk);
    ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_TryToFix_001
 * @tc.desc: Verify the TryToFix function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_TryToFix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_TryToFix_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(1009);
    std::string volId = "vol-8-1";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-1";
    std::string path = "/";
    std::string description = "My Disk";

    auto ret = storageManagerProviderTest_->TryToFix(volId);
    EXPECT_EQ(ret, E_OK);

    int32_t fsType = 1;
    std::string diskId = "disk-1-6";
    VolumeCore vc(volId, fsType, diskId);
    storageManagerProviderTest_->NotifyVolumeCreated(vc);
    ret = storageManagerProviderTest_->TryToFix(volId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_TryToFix_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_RegisterUeceActivationCallback_001
 * @tc.desc: Verify the RegisterUeceActivationCallback function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RegisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RegisterUeceActivationCallback_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);

    sptr<IUeceActivationCallback> ueceCallback(new (std::nothrow) UeceActivationCallbackMock());
    EXPECT_NE(ueceCallback, nullptr);
    EXPECT_EQ(storageManagerProviderTest_->RegisterUeceActivationCallback(ueceCallback), E_SERVICE_IS_NULLPTR);
    storageManagerProviderTest_->UnregisterUeceActivationCallback();
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RegisterUeceActivationCallback_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UnregisterUeceActivationCallback_001
 * @tc.desc: Verify the UnregisterUeceActivationCallback function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UnregisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_TryToFix_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);

    EXPECT_EQ(storageManagerProviderTest_->UnregisterUeceActivationCallback(), E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnregisterUeceActivationCallback_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CreateUserDir_001
 * @tc.desc: Verify the CreateUserDir function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CreateUserDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateUserDir_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);

    g_testCallingUid = 0;
    EXPECT_EQ(storageManagerProviderTest_->CreateUserDir("", 0, 0, 0), E_PERMISSION_DENIED);

    g_testCallingUid = 7558;
    EXPECT_EQ(storageManagerProviderTest_->CreateUserDir("", 0, 0, 0), E_SERVICE_IS_NULLPTR);

    EXPECT_EQ(storageManagerProviderTest_->CreateUserDir("/test/../", 0, 0, 0), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateUserDir_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_DeleteUserDir_001
 * @tc.desc: Verify the DeleteUserDir function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_DeleteUserDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteUserDir_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);

    g_testCallingUid = 0;
    EXPECT_EQ(storageManagerProviderTest_->DeleteUserDir(""), E_PERMISSION_DENIED);

    g_testCallingUid = 7558;
    EXPECT_EQ(storageManagerProviderTest_->DeleteUserDir(""), E_SERVICE_IS_NULLPTR);

    EXPECT_EQ(storageManagerProviderTest_->DeleteUserDir("/test/../"), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteUserDir_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateUserPublicDirPolicy_002
 * @tc.desc: Verify the UpdateUserPublicDirPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR20250722463628
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateUserPublicDirPolicy_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserPublicDirPolicy_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    ScopedTestUid uidGuard(7014);
    uint32_t userId = 100;
    auto ret = storageManagerProviderTest_->UpdateUserPublicDirPolicy(userId);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserPublicDirPolicy_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetExtBundleStats_001
 * @tc.desc: Verify the SetExtBundleStats function.
 * @tc.type: FUNC
 * @tc.require: AR20250722463628
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetExtBundleStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetExtBundleStats_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = TOP_USER_ID + 1;
    ExtBundleStats extBundleStats;
    extBundleStats.businessName_ = "businessName";
    extBundleStats.businessSize_ = 0;
    g_testCallingUid = 7558;
    auto ret = storageManagerProviderTest_->SetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    userId = INT32_MAX;
    ret = storageManagerProviderTest_->SetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    userId = 1;
    extBundleStats.businessSize_ = INT64_MAX;
    ret = storageManagerProviderTest_->SetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    extBundleStats.businessSize_ = 1;
    extBundleStats.businessName_ = "";
    ret = storageManagerProviderTest_->SetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    extBundleStats.businessName_ = "test";
    ret = storageManagerProviderTest_->SetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_OK);
    g_accessTokenType = 0;
    ret = storageManagerProviderTest_->SetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_SYS_APP_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetExtBundleStats_001 end";
}


/**
 * @tc.name: StorageManagerProviderTest_GetExtBundleStats_001
 * @tc.desc: Verify the GetExtBundleStats function.
 * @tc.type: FUNC
 * @tc.require: AR20250722463628
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetExtBundleStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetExtBundleStats_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = TOP_USER_ID + 1;
    ExtBundleStats extBundleStats;
    extBundleStats.businessName_ = "businessName";
    extBundleStats.businessSize_ = 0;
    g_testCallingUid = 7558;
    auto ret = storageManagerProviderTest_->GetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    userId = INT32_MAX;
    ret = storageManagerProviderTest_->GetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    userId = 1;
    extBundleStats.businessName_ = "";
    ret = storageManagerProviderTest_->GetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    extBundleStats.businessName_ = "test";
    ret = storageManagerProviderTest_->GetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_OK);
    g_accessTokenType = 0;
    ret = storageManagerProviderTest_->GetExtBundleStats(userId, extBundleStats);
    EXPECT_EQ(ret, E_SYS_APP_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetExtBundleStats_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetAllExtBundleStats_001
 * @tc.desc: Verify the GetAllExtBundleStats function.
 * @tc.type: FUNC
 * @tc.require: AR20250722463628
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetAllExtBundleStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllExtBundleStats_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = TOP_USER_ID + 1;
    g_testCallingUid = 7558;
    std::vector<ExtBundleStats> bundleStats;
    auto ret = storageManagerProviderTest_->GetAllExtBundleStats(userId, bundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    userId = INT32_MAX;
    ret = storageManagerProviderTest_->GetAllExtBundleStats(userId, bundleStats);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    userId = 1;
    ret = storageManagerProviderTest_->GetAllExtBundleStats(userId, bundleStats);
    EXPECT_EQ(ret, E_OK);
    g_accessTokenType = 0;
    ret = storageManagerProviderTest_->GetAllExtBundleStats(userId, bundleStats);
    EXPECT_EQ(ret, E_SYS_APP_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetAllExtBundleStats_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ListUserdataDirInfo_002
 * @tc.desc: Verify the ListUserdataDirInfo function.
 * @tc.type: FUNC
 * @tc.require: AR20251022750568
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ListUserdataDirInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ListUserdataDirInfo_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::vector<UserdataDirInfo> scanDirs;
    auto ret = storageManagerProviderTest_->ListUserdataDirInfo(scanDirs);
    EXPECT_NE(ret, E_OK);
    EXPECT_EQ(scanDirs.size(), 0);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ListUserdataDirInfo_002 end";
}


/**
 * @tc.name: StorageManagerProviderTest_NotifyCreateBundleDataDirWithEl_001
 * @tc.desc: Verify the NotifyCreateBundleDataDirWithEl function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyCreateBundleDataDirWithEl_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyCreateBundleDataDirWithEl_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t userId = 100;
    uint8_t elx = 1;

    g_testCallingUid = 1;
    EXPECT_EQ(storageManagerProviderTest_->NotifyCreateBundleDataDirWithEl(userId, elx), E_PERMISSION_DENIED);

    g_testCallingUid = 0;
    EXPECT_EQ(storageManagerProviderTest_->NotifyCreateBundleDataDirWithEl(userId, elx), E_ERR);

    elx = 2;
    EXPECT_EQ(storageManagerProviderTest_->NotifyCreateBundleDataDirWithEl(userId, elx), E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyCreateBundleDataDirWithEl_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_QueryActiveOsAccountIds_002
 * @tc.desc: Verify the QueryActiveOsAccountIds function.
 * @tc.type: FUNC
 * @tc.require: AR20251022750568
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_QueryActiveOsAccountIds_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryActiveOsAccountIds_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::vector<int32_t> ids;
    g_testCallingUid = 1;
    EXPECT_EQ(storageManagerProviderTest_->QueryActiveOsAccountIds(ids), E_PERMISSION_DENIED);
    g_testCallingUid = 0;
    EXPECT_NE(storageManagerProviderTest_->QueryActiveOsAccountIds(ids), E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryActiveOsAccountIds_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_IsOsAccountExists_002
 * @tc.desc: Verify the IsOsAccountExists function.
 * @tc.type: FUNC
 * @tc.require: AR20251022750568
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_IsOsAccountExists_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsOsAccountExists_002 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    unsigned int userId = 0;
    bool isOsAccountExists = true;
    g_testCallingUid = 1;
    EXPECT_EQ(storageManagerProviderTest_->IsOsAccountExists(userId, isOsAccountExists), E_PERMISSION_DENIED);
    g_testCallingUid = 0;
    EXPECT_NE(storageManagerProviderTest_->IsOsAccountExists(userId, isOsAccountExists), E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsOsAccountExists_002 end";
}
}
}