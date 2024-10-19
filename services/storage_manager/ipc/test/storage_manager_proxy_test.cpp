/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "storage_manager_proxy.h"
#include "ipc/storage_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "storage_manager_service_mock.h"
#include "system_ability_definition.h"
#include "volume/volume_manager_service.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class StorageManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp();
    void TearDown() {};
    std::shared_ptr<StorageManagerProxy> proxy_ = nullptr;
    sptr<StorageManagerServiceMock> mock_ = nullptr;
};

void StorageManagerProxyTest::SetUp()
{
    mock_ = new StorageManagerServiceMock();
    proxy_ = std::make_shared<StorageManagerProxy>(mock_);
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareAddUser_0000
 * @tc.name: Storage_manager_proxy_PrepareAddUser_0000
 * @tc.desc: Test function of PrepareAddUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareAddUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareAddUser_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    int32_t userId = 101;
    uint32_t flag = CRYPTO_FLAG_EL1;
    int32_t result = proxy_->PrepareAddUser(userId, flag);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->PrepareAddUser(userId, flag);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareAddUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_RemoveUser_0000
 * @tc.name: Storage_manager_proxy_RemoveUser_0000
 * @tc.desc: Test function of RemoveUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_RemoveUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_RemoveUser_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    int32_t userId = 103;
    uint32_t flag = CRYPTO_FLAG_EL1;
    int32_t result = proxy_->RemoveUser(userId, flag);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->RemoveUser(userId, flag);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_RemoveUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareStartUser_0000
 * @tc.name: Storage_manager_proxy_PrepareStartUser_0000
 * @tc.desc: Test function of PrepareStartUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareStartUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareStartUser_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    int32_t userId = 105;
    int32_t result = proxy_->PrepareStartUser(userId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->PrepareStartUser(userId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareStartUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_StopUser_0000
 * @tc.name: Storage_manager_proxy_StopUser_0000
 * @tc.desc: Test function of StopUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_StopUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_StopUser_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    int32_t userId = 108;
    int32_t result = proxy_->StopUser(userId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->StopUser(userId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_StopUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_CompleteAddUser_0000
 * @tc.name: Storage_manager_proxy_CompleteAddUser_0000
 * @tc.desc: Test function of CompleteAddUser interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_CompleteAddUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_CompleteAddUser_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    int32_t userId = 109;
    int32_t result = proxy_->CompleteAddUser(userId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->CompleteAddUser(userId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_CompleteAddUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetFreeSizeOfVolume_0000
 * @tc.name: Storage_manager_proxy_GetFreeSizeOfVolume_0000
 * @tc.desc: Test function of GetFreeSizeOfVolume interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK100
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetFreeSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetFreeSizeOfVolume_0000";
    std::string volumeUuid = "uuid-1";
    int64_t freeSize;
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetFreeSizeOfVolume_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetTotalSizeOfVolume_0000
 * @tc.name: Storage_manager_proxy_GetTotalSizeOfVolume_0000
 * @tc.desc: Test function of GetTotalSizeOfVolume interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK100
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetTotalSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetTotalSizeOfVolume_0000";
    std::string volumeUuid = "uuid-2";
    int64_t totalSize;
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetTotalSizeOfVolume_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetBundleStats_0000
 * @tc.name: Storage_manager_proxy_GetBundleStats_0000
 * @tc.desc: Test function of GetBundleStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK101
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetBundleStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetBundleStats_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string pkgName = "ohos.acts.storage.volume";
    BundleStats bundleStats;
    int32_t result = proxy_->GetBundleStats(pkgName, bundleStats, 0);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetBundleStats(pkgName, bundleStats, 0);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetBundleStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyVolumeCreated_0000
 * @tc.name: Storage_manager_proxy_NotifyVolumeCreated_0000
 * @tc.desc: Test function of NotifyVolumeCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyVolumeCreated_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyVolumeCreated_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string volumeId = "vol-1-16";
    int32_t fsType = 1;
    std::string diskId = "disk-1-17";
    VolumeCore vc(volumeId, fsType, diskId);
    int64_t result = proxy_->NotifyVolumeCreated(vc);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->NotifyVolumeCreated(vc);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyVolumeCreated_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyVolumeMounted_0000
 * @tc.name: Storage_manager_proxy_NotifyVolumeMounted_0000
 * @tc.desc: Test function of NotifyVolumeMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyVolumeMounted_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyVolumeMounted_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string volumeId = "vol-1-18";
    int32_t fsType = 1;
    std::string fsUuid = "uuid-3";
    std::string path = "/";
    std::string description = "description-1";
    int64_t result = proxy_->NotifyVolumeMounted(volumeId, fsType, fsUuid, path, description);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->NotifyVolumeMounted(volumeId, fsType, fsUuid, path, description);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyVolumeMounted_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyVolumeDestroyed_0000
 * @tc.name: Storage_manager_proxy_NotifyVolumeDestroyed_0000
 * @tc.desc: Test function of NotifyVolumeDestroyed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyVolumeDestroyed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyVolumeDestroyed_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string volumeId = "vol-1-20";
    int64_t result = proxy_->NotifyVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->NotifyVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyVolumeDestroyed_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_Mount_0000
 * @tc.name: Storage_manager_proxy_Mount_0000
 * @tc.desc: Test function of Mount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUOT
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_Mount_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_Mount_0000";
    std::string volumeId = "vol-1-21";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->Mount(volumeId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->Mount(volumeId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_Mount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_Unmount_0000
 * @tc.name: Storage_manager_proxy_Unmount_0000
 * @tc.desc: Test function of Unmount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUOT
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_Unmount_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_Unmount_0000";
    std::string volumeId = "vol-1-22";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->Unmount(volumeId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->Unmount(volumeId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_Unmount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetAllVolumes_0000
 * @tc.name: Storage_manager_proxy_GetAllVolumes_0000
 * @tc.desc: Test function of GetAllVolumes interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetAllVolumes_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetAllVolumes_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::vector<VolumeExternal> vecOfVol;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetAllVolumes(vecOfVol);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetAllVolumes(vecOfVol);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetAllVolumes_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyDiskCreated_0000
 * @tc.name: Storage_manager_proxy_NotifyDiskCreated_0000
 * @tc.desc: Test function of NotifyDiskCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyDiskCreated_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyDiskCreated_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string diskId = "disk-1-23";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1; // disk type
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    int32_t result = proxy_->NotifyDiskCreated(disk);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->NotifyDiskCreated(disk);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyDiskCreated_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyDiskDestroyed_0000
 * @tc.name: Storage_manager_proxy_NotifyDiskDestroyed_0000
 * @tc.desc: Test function of NotifyDiskDestroyed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyDiskDestroyed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyDiskDestroyed_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string diskId = "disk-1-24";
    int32_t result = proxy_->NotifyDiskDestroyed(diskId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->NotifyDiskDestroyed(diskId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyDiskDestroyed_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_Partition_0000
 * @tc.name: Storage_manager_proxy_Partition_0000
 * @tc.desc: Test function of Partition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUOT
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_Partition_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_Partition_0000";
    std::string volumeId = "vol-1-25";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string diskId = "disk-1-25";
    int32_t type = 1;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->Partition(diskId, type);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->Partition(diskId, type);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_Partition_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetAllDisks_0000
 * @tc.name: Storage_manager_proxy_GetAllDisks_0000
 * @tc.desc: Test function of GetAllDisks interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetAllDisks_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetAllDisks_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::vector<Disk> vecOfDisk;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetAllDisks(vecOfDisk);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetAllDisks(vecOfDisk);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetAllDisks_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetSystemSize_0000
 * @tc.name: Storage_manager_proxy_GetSystemSize_0000
 * @tc.desc: Test function of GetSystemSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetSystemSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetSystemSize_0000";
    int64_t systemSize;
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetSystemSize(systemSize);
    EXPECT_GE(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetSystemSize(systemSize);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetSystemSize_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetTotalSize_0000
 * @tc.name: Storage_manager_proxy_GetTotalSize_0000
 * @tc.desc: Test function of GetTotalSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetTotalSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetTotalSize_0000";
    int64_t totalSize;
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetTotalSize(totalSize);
    EXPECT_GE(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetTotalSize(totalSize);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetTotalSize_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetFreeSize_0000
 * @tc.name: Storage_manager_proxy_GetFreeSize_0000
 * @tc.desc: Test function of GetFreeSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0371
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetFreeSize_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetFreeSize_0000";
    int64_t FreeSize;
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetFreeSize(FreeSize);
    EXPECT_GE(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetFreeSize(FreeSize);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetFreeSize_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetUserStorageStats_0000
 * @tc.name: Storage_manager_proxy_GetUserStorageStats_0000
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0373
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetUserStorageStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetUserStorageStats_0000";
    StorageStats storageStats;
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetUserStorageStats(storageStats);
    EXPECT_GE(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetUserStorageStats(storageStats);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetUserStorageStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetUserStorageStats_0001
 * @tc.name: Storage_manager_proxy_GetUserStorageStats_0001
 * @tc.desc: Test function of GetUserStorageStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0373
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetUserStorageStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetUserStorageStats_0001";
    StorageStats storageStats;
    int32_t userId = 111;
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetUserStorageStats(userId, storageStats);
    EXPECT_GE(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetUserStorageStats(userId, storageStats);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetUserStorageStats_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetVolumeByUuid_0000
 * @tc.name: Storage_manager_proxy_GetVolumeByUuid_0000
 * @tc.desc: Test function of GetVolumeByUuid interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetVolumeByUuid_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetVolumeByUuid_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string fsUuid = "uuid-4";
    VolumeExternal ve;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int64_t result = proxy_->GetVolumeByUuid(fsUuid, ve);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetVolumeByUuid(fsUuid, ve);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetVolumeByUuid_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetVolumeById_0000
 * @tc.name: Storage_manager_proxy_GetVolumeById_0000
 * @tc.desc: Test function of GetVolumeById interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetVolumeById_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetVolumeById_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string volumeId = "vol-1-27";
    VolumeExternal ve;
    int64_t result = proxy_->GetVolumeById(volumeId, ve);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetVolumeById(volumeId, ve);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetVolumeById_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_SetVolumeDescription_0000
 * @tc.name: Storage_manager_proxy_SetVolumeDescription_0000
 * @tc.desc: Test function of SetVolumeDescription interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_SetVolumeDescription_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_SetVolumeDescription_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string fsUuid = "uuid-6";
    string description = "description-1";
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int64_t result = proxy_->SetVolumeDescription(fsUuid, description);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->SetVolumeDescription(fsUuid, description);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_SetVolumeDescription_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_Format_0000
 * @tc.name: Storage_manager_proxy_Format_0000
 * @tc.desc: Test function of Format interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_Format_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_Format_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string volumeId = "vol-1-29";
    string fsTypes = "1";
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int64_t result = proxy_->Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_Format_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetDiskById_0000
 * @tc.name: Storage_manager_proxy_GetDiskById_0000
 * @tc.desc: Test function of GetDiskById interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetDiskById_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetDiskById_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string diskId = "disk-1-30";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1; // disk type
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    int64_t result = proxy_->GetDiskById(diskId, disk);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetDiskById(diskId, disk);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetDiskById_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_CreateShareFile_0000
 * @tc.name: Storage_manager_proxy_CreateShareFile_0000
 * @tc.desc: Test function of CreateShareFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI7U9Z9
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_CreateShareFile_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_CreateShareFile_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string uri = "file://com.demo.a/storage/share/files/test.txt";
    uint32_t tokenId = 100;
    uint32_t flag = 0;
    vector<string> uriList(1, uri);
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    vector<int32_t> retList = proxy_->CreateShareFile(uriList, tokenId, flag);
    for (const auto &ret : retList) {
        EXPECT_EQ(ret, E_OK);
    }
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_CreateShareFile_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_CreateShareFile_0100
 * @tc.name: Storage_manager_proxy_CreateShareFile_0100
 * @tc.desc: Test function of CreateShareFile interface for SendRequest failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI7U9Z9
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_CreateShareFile_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_CreateShareFile_0100";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_DESCRIPTOR_ERR));
    std::string uri = "file://com.demo.a/storage/share/files/test.txt";
    uint32_t tokenId = 100;
    uint32_t flag = 0;
    vector<string> uriList(1, uri);
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    vector<int32_t> retList = proxy_->CreateShareFile(uriList, tokenId, flag);
    for (const auto &ret : retList) {
        EXPECT_EQ(ret, E_WRITE_DESCRIPTOR_ERR);
    }
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_CreateShareFile_0100";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_DeleteShareFile_0000
 * @tc.name: Storage_manager_proxy_DeleteShareFile_0000
 * @tc.desc: Test function of DeleteShareFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI7U9Z9
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_DeleteShareFile_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_DeleteShareFile_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string uri = "file://com.demo.a/storage/share/files/test.txt";
    uint32_t tokenId = 100;
    std::vector<std::string> sharePathList;
    sharePathList.push_back(uri);
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int64_t result = proxy_->DeleteShareFile(tokenId, sharePathList);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->DeleteShareFile(tokenId, sharePathList);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_DeleteShareFile_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GenerateUserKeys_0000
 * @tc.name: Storage_manager_proxy_GenerateUserKeys_0000
 * @tc.desc: Test function of GenerateUserKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GenerateUserKeys_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GenerateUserKeys_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 112;
    uint32_t flags = 2; // UserKeys type
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GenerateUserKeys_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_DeleteUserKeys_0000
 * @tc.name: Storage_manager_proxy_DeleteUserKeys_0000
 * @tc.desc: Test function of DeleteUserKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_DeleteUserKeys_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_DeleteUserKeys_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 113;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->DeleteUserKeys(userId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->DeleteUserKeys(userId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_DeleteUserKeys_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_UpdateUserAuth_0000
 * @tc.name: Storage_manager_proxy_UpdateUserAuth_0000
 * @tc.desc: Test function of UpdateUserAuth interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_UpdateUserAuth_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_UpdateUserAuth_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 114;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->UpdateUserAuth(userId, 0, {}, {}, {});
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->UpdateUserAuth(userId, 0, {}, {}, {});
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_UpdateUserAuth_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_ActiveUserKey_0000
 * @tc.name: Storage_manager_proxy_ActiveUserKey_0000
 * @tc.desc: Test function of ActiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_ActiveUserKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_ActiveUserKey_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 115;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->ActiveUserKey(userId, {}, {});
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->ActiveUserKey(userId, {}, {});
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_ActiveUserKey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_InactiveUserKey_0000
 * @tc.name: Storage_manager_proxy_InactiveUserKey_0000
 * @tc.desc: Test function of InactiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_InactiveUserKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_InactiveUserKey_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 116;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->InactiveUserKey(userId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->InactiveUserKey(userId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_InactiveUserKey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_LockUserScreen_0000
 * @tc.name: Storage_manager_proxy_LockUserScreen_0000
 * @tc.desc: Test function of LockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_LockUserScreen_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_LockUserScreen_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 116;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->LockUserScreen(userId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->LockUserScreen(userId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_LockUserScreen_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_UnlockUserScreen_0000
 * @tc.name: Storage_manager_proxy_UnlockUserScreen_0000
 * @tc.desc: Test function of UnlockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_UnlockUserScreen_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_UnlockUserScreen_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 120;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->UnlockUserScreen(userId, {}, {});
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->UnlockUserScreen(userId, {}, {});
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_UnlockUserScreen_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GenerateAppkey_0000
 * @tc.name: Storage_manager_proxy_GenerateAppkey_0000
 * @tc.desc: Test function of UnlockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GenerateAppkey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GenerateAppkey_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    uint32_t hashId = 0;
    uint32_t userId = 0;
    std::string keyId;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->GenerateAppkey(hashId, userId, keyId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GenerateAppkey(hashId, userId, keyId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "Storage_manager_proxy_GenerateAppkey_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetLockScreenStatus_0000
 * @tc.name: Storage_manager_proxy_GetLockScreenStatus_0000
 * @tc.desc: Test function of GetLockScreenStatus interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: I9TCA3
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetLockScreenStatus_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetLockScreenStatus_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 120;
    bool lockStatus;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->GetLockScreenStatus(userId, lockStatus);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetLockScreenStatus(userId, lockStatus);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetLockScreenStatus_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_DeleteAppkey_0000
 * @tc.name: Storage_manager_proxy_DeleteAppkey_0000
 * @tc.desc: Test function of UnlockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_DeleteAppkey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_DeleteAppkey_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    const std::string keyId;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->DeleteAppkey(keyId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->DeleteAppkey(keyId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_DeleteAppkey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_CreateRecoverKey_0000
 * @tc.name: Storage_manager_proxy_CreateRecoverKey_0000
 * @tc.desc: Test function of CreateRecoverKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_CreateRecoverKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_CreateRecoverKey_0000";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 120;
    uint32_t userType = 12;
    uint32_t result = proxy_->CreateRecoverKey(userId, userType, {}, {});
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->CreateRecoverKey(userId, userType, {}, {});
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_CreateRecoverKey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_SetRecoverKey_0000
 * @tc.name: Storage_manager_proxy_SetRecoverKey_0000
 * @tc.desc: Test function of SetRecoverKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_SetRecoverKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_SetRecoverKey_0000";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t result = proxy_->SetRecoverKey({});
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->SetRecoverKey({});
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_SetRecoverKey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_MountDfsDocs_0000
 * @tc.name: Storage_manager_proxy_MountDfsDocs_0000
 * @tc.desc: Test function of MountDfsDocs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_MountDfsDocs_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_MountDfsDocs_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 120;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_MountDfsDocs_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_UMountDfsDocs_0000
 * @tc.name: Storage_manager_proxy_UMountDfsDocs_0000
 * @tc.desc: Test function of UMountDfsDocs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_UMountDfsDocs_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_UMountDfsDocs_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 120;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_UMountDfsDocs_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_UpdateKeyContext_0000
 * @tc.name: Storage_manager_proxy_UpdateKeyContext_0000
 * @tc.desc: Test function of UpdateKeyContext interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_UpdateKeyContext_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_UpdateKeyContext_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 117;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    uint32_t result = proxy_->UpdateKeyContext(userId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->UpdateKeyContext(userId);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_UpdateKeyContext_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetCurrentBundleStats_0000
 * @tc.name: Storage_manager_proxy_GetCurrentBundleStats_0000
 * @tc.desc: Test function of GetCurrentBundleStats interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetCurrentBundleStats_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetCurrentBundleStats_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    BundleStats bundleStats;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetCurrentBundleStats(bundleStats);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetCurrentBundleStats(bundleStats);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetCurrentBundleStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_SetBundleQuota_0000
 * @tc.name: Storage_manager_proxy_SetBundleQuota_0000
 * @tc.desc: Test function of SetBundleQuota interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000HSKSO
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_SetBundleQuota_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_SetBundleQuota_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string bundleName = "com.ohos.bundleName-0-1";
    std::string bundleDataDirPath = "/data/app/el2/100/base/" + bundleName;
    int32_t uid = 20000000;
    int32_t limitSizeMb = 1000;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_SetBundleQuota_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetUserStorageStatsByType_0000
 * @tc.name: Storage_manager_proxy_GetUserStorageStatsByType_0000
 * @tc.desc: Test function of GetUserStorageStatsByType interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetUserStorageStatsByType_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetUserStorageStatsByType_0000";
    StorageStats storageStats;
    int32_t userId = 111;
    std::string type = "media";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_GE(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetUserStorageStatsByType_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_UpdateMemoryPara_0000
 * @tc.name: Storage_manager_proxy_UpdateMemoryPara_0000
 * @tc.desc: Test function of UpdateMemoryPara interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: I90X2X
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_UpdateMemoryPara_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_UpdateMemoryPara_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t size = 1000;
    int32_t oldSize =500;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->UpdateMemoryPara(size, oldSize);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->UpdateMemoryPara(size, oldSize);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_UpdateMemoryPara_0000";
}

/**
 * @tc.number: SUB_Storage_manager_proxy_GetBundleStatsForIncrease_0000
 * @tc.name: Storage_manager_proxy_GetBundleStatsForIncrease_0000
 * @tc.desc: Test function of GetBundleStatsForIncrease interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetBundleStatsForIncrease_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetBundleStatsForIncrease_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 100;
    std::vector<std::string> bundleNames;
    std::vector<int64_t> incrementalBackTimes;
    std::vector<int64_t> pkgFileSizes;
    std::vector<int64_t> incPkgFileSizes;
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int32_t result = proxy_->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes,
        incPkgFileSizes);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes,
        incPkgFileSizes);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetBundleStatsForIncrease_0000";
}

/**
 * @tc.number: Storage_manager_proxy_GetFileEncryptStatus_0000
 * @tc.name: Storage_manager_proxy_GetFileEncryptStatus_0000
 * @tc.desc: Test function of GetFileEncryptStatus interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetFileEncryptStatus_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetFileEncryptStatus_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 800;
    bool isEncrypted;
    int32_t result = proxy_->GetFileEncryptStatus(userId, isEncrypted);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->GetFileEncryptStatus(userId, isEncrypted);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetFileEncryptStatus_0000";
}

/**
 * @tc.number: Storage_manager_proxy_UpdateUseAuthWithRecoveryKey_0000
 * @tc.name: Storage_manager_proxy_UpdateUseAuthWithRecoveryKey_0000
 * @tc.desc: Test function of UpdateUseAuthWithRecoveryKey interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_UpdateUseAuthWithRecoveryKey_0000,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_UpdateUseAuthWithRecoveryKey_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::vector<uint8_t> authToken;
    std::vector<uint8_t> newSecret;
    uint64_t secureUid = 0;
    uint32_t userId = 800;
    std::vector<std::vector<uint8_t>> plainText;
    int32_t result = proxy_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_UpdateUseAuthWithRecoveryKey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyMtpMounted_0000
 * @tc.name: Storage_manager_proxy_NotifyMtpMounted_0000
 * @tc.desc: Test function of NotifyMtpMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyMtpMounted_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyMtpMounted_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";

    std::string id = "vol-1-18";
    std::string path = "/";
    std::string description = "description-1";
    std::string uuid = "uuid-1";
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int64_t result = proxy_->NotifyMtpMounted(id, path, description, uuid);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->NotifyMtpMounted(id, path, description, uuid);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyMtpMounted_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyMtpUnmounted_0000
 * @tc.name: Storage_manager_proxy_NotifyMtpUnmounted_0000
 * @tc.desc: Test function of NotifyMtpUnmounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyMtpUnmounted_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyMtpUnmounted_0000";
    ASSERT_TRUE(mock_ != nullptr) << "StorageManagerServiceMock failed";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    std::string id = "vol-1-18";
    std::string path = "/";
    ASSERT_TRUE(proxy_ != nullptr) << "StorageManagerProxy failed";
    int64_t result = proxy_->NotifyMtpUnmounted(id, path);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_PARCEL_ERR));
    result = proxy_->NotifyMtpUnmounted(id, path);
    EXPECT_EQ(result, E_WRITE_PARCEL_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyMtpUnmounted_0000";
}
} // namespace