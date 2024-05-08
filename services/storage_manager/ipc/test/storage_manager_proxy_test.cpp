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
    int32_t userId = 101;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(samgr != nullptr) << "Storage_manager_proxy_PrepareAddUser_0000 fail to get GetSystemAbilityManager";
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_TRUE(remote != nullptr) << "GetSystemAbility failed";
    auto proxy = iface_cast<IStorageManager>(remote);
    ASSERT_TRUE(proxy != nullptr) << "fail to get proxy";
    int32_t result = proxy->PrepareAddUser(userId, flag);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    proxy->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareAddUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareAddUser_0001
 * @tc.name: Storage_manager_proxy_PrepareAddUser_0001
 * @tc.desc: Test function of PrepareAddUser interface for Parameters ERROR which userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareAddUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareAddUser_0001";
    int32_t userId = -1;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(samgr != nullptr) << "Storage_manager_proxy_PrepareAddUser_0001 fail to get GetSystemAbilityManager";
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_TRUE(remote != nullptr) << "GetSystemAbility failed";
    auto proxy = iface_cast<IStorageManager>(remote);
    ASSERT_TRUE(proxy != nullptr) << "fail to get proxy";
    int32_t result = proxy->PrepareAddUser(userId, flag);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareAddUser_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareAddUser_0002
 * @tc.name: Storage_manager_proxy_PrepareAddUser_0002
 * @tc.desc: Test function of PrepareAddUser interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareAddUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareAddUser_0002";
    int32_t userId = 10000;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int32_t result = proxy->PrepareAddUser(userId, flag);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareAddUser_0002";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareAddUser_0003
 * @tc.name: Storage_manager_proxy_PrepareAddUser_0003
 * @tc.desc: Test function of PrepareAddUser interface for SUCCESS which Repeated add.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareAddUser_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareAddUser_0003";
    int32_t userId = 102;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    int32_t result = proxy->PrepareAddUser(userId, flag);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    proxy->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareAddUser_0003";
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
    int32_t userId = 103;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    int32_t result = proxy->RemoveUser(userId, flag);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_RemoveUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_RemoveUser_0001
 * @tc.name: Storage_manager_proxy_RemoveUser_0001
 * @tc.desc: Test function of RemoveUser interface for SUCCESS which remove userId not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_RemoveUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_RemoveUser_0001";
    int32_t userId = 104;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int32_t result = proxy->RemoveUser(userId, flag);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_RemoveUser_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_RemoveUser_0002
 * @tc.name: Storage_manager_proxy_RemoveUser_0002
 * @tc.desc: Test function of RemoveUser interface for Logic ERROR which userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_RemoveUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_RemoveUser_0002";
    int32_t userId = -2;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    int32_t result = proxy->RemoveUser(userId, flag);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_RemoveUser_0002";
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
    int32_t userId = 105;
    uint32_t flag = CRYPTO_FLAG_EL2;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    int32_t result = proxy->PrepareStartUser(userId);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    proxy->StopUser(userId);
    proxy->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareStartUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareStartUser_0002
 * @tc.name: Storage_manager_proxy_PrepareStartUser_0002
 * @tc.desc: Test function of PrepareStartUser interface for Logic ERROR which start userId not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareStartUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareStartUser_0001";
    int32_t userId = 107;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int32_t result = proxy->PrepareStartUser(userId);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareStartUser_0002";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareStartUser_0003
 * @tc.name: Storage_manager_proxy_PrepareStartUser_0003
 * @tc.desc: Test function of PrepareStartUser interface for Logic ERROR which  userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareStartUser_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareStartUser_0001";
    int32_t userId = -3;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    int32_t result = proxy->PrepareStartUser(userId);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareStartUser_0003";
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
    int32_t userId = 108;
    uint32_t flag = CRYPTO_FLAG_EL2;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    proxy->PrepareStartUser(userId);
    int32_t result = proxy->StopUser(userId);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    proxy->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_StopUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_StopUser_0001
 * @tc.name: Storage_manager_proxy_StopUser_0001
 * @tc.desc: Test function of StopUser interface for Logic ERROR which stop userId not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_StopUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_StopUser_0001";
    int32_t userId = 109;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int32_t result = proxy->StopUser(userId);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_StopUser_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_StopUser_0002
 * @tc.name: Storage_manager_proxy_StopUser_0002
 * @tc.desc: Test function of StopUser interface for Logic ERROR which stop userId not start.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_StopUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_StopUser_0002";
    int32_t userId = 110;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    int32_t result = proxy->StopUser(userId);
    EXPECT_NE(result, E_OK);
    proxy->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_StopUser_0002";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_StopUser_0003
 * @tc.name: Storage_manager_proxy_StopUser_0003
 * @tc.desc: Test function of StopUser interface for Logic ERROR which userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_StopUser_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_StopUser_0003";
    int32_t userId = -4;
    uint32_t flag = CRYPTO_FLAG_EL1;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    proxy->PrepareAddUser(userId, flag);
    proxy->PrepareStartUser(userId);
    int32_t result = proxy->StopUser(userId);
    EXPECT_NE(result, E_OK);
    proxy->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_StopUser_0003";
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << result;
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << result;
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
    std::string pkgName = "ohos.acts.storage.volume";
    BundleStats bundleStats;
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetBundleStats(pkgName, bundleStats);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetBundleStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyVolumeCreated_0000
 * @tc.name: Storage_manager_proxy_NotifyVolumeCreated_0001
 * @tc.desc: Test function of NotifyVolumeCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyVolumeCreated_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyVolumeCreated_0000";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string volumeId = "vol-1-16";
    int32_t fsType = 1;
    std::string diskId = "disk-1-17";
    VolumeCore vc(volumeId, fsType, diskId);
    int64_t result = proxy_->NotifyVolumeCreated(vc);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << result;
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyVolumeCreated_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyVolumeMounted_0000
 * @tc.name: Storage_manager_proxy_NotifyVolumeMounted_0001
 * @tc.desc: Test function of NotifyVolumeMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyVolumeMounted_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyVolumeMounted_0000";
    std::string volumeId = "vol-1-18";
    int32_t fsType = 1;
    std::string fsUuid = "uuid-3";
    std::string path = "/";
    std::string description = "description-1";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int64_t result = proxy->NotifyVolumeMounted(volumeId, fsType, fsUuid, path, description);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << result;
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyVolumeMounted_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyVolumeDestroyed_0000
 * @tc.name: Storage_manager_proxy_NotifyVolumeDestroyed_0001
 * @tc.desc: Test function of NotifyVolumeDestroyed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyVolumeDestroyed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyVolumeDestroyed_0000";
    std::string volumeId = "vol-1-20";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int64_t result = proxy->NotifyVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << result;
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    int32_t result = proxy_->Mount(volumeId);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    int32_t result = proxy_->Unmount(volumeId);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::vector<VolumeExternal> vecOfVol;
    int32_t result = proxy_->GetAllVolumes(vecOfVol);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetAllVolumes_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyDiskCreated_0000
 * @tc.name: Storage_manager_proxy_NotifyDiskCreated_0001
 * @tc.desc: Test function of NotifyDiskCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyDiskCreated_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyDiskCreated_0000";
    std::string diskId = "disk-1-23";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1; // disk type
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int32_t result = proxy->NotifyDiskCreated(disk);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_NotifyDiskCreated_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_NotifyDiskDestroyed_0000
 * @tc.name: Storage_manager_proxy_NotifyDiskDestroyed_0001
 * @tc.desc: Test function of NotifyDiskDestroyed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_NotifyDiskDestroyed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_NotifyDiskDestroyed_0000";
    std::string diskId = "disk-1-24";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    int32_t result = proxy->NotifyDiskDestroyed(diskId);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string diskId = "disk-1-25";
    int32_t type = 1;
    int32_t result = proxy_->Partition(diskId, type);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::vector<Disk> vecOfDisk;
    int32_t result = proxy_->GetAllDisks(vecOfDisk);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetSystemSize(systemSize);
    EXPECT_GE(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetTotalSize(totalSize);
    EXPECT_GE(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetFreeSize(FreeSize);
    EXPECT_GE(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetUserStorageStats(storageStats);
    EXPECT_GE(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetUserStorageStats(userId, storageStats);
    EXPECT_GE(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string fsUuid = "uuid-4";
    VolumeExternal ve;
    int64_t result = proxy_->GetVolumeByUuid(fsUuid, ve);
    EXPECT_EQ(result, E_OK);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    std::string volumeId = "vol-1-27";
    int32_t fsType = 1;
    std::string fsUuid = "uuid-5";
    std::string diskId = "disk-1-27";
    VolumeCore vc(volumeId, fsType, diskId);
    proxy->NotifyVolumeCreated(vc);
    VolumeExternal ve;
    int64_t result = proxy->GetVolumeById(volumeId, ve);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
    proxy->NotifyVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string fsUuid = "uuid-6";
    string description = "description-1";
    int64_t result = proxy_->SetVolumeDescription(fsUuid, description);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string volumeId = "vol-1-29";
    string fsTypes = "1";
    int64_t result = proxy_->Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_OK);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<IStorageManager>(remote);
    std::string diskId = "disk-1-30";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1; // disk type
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    proxy->NotifyDiskCreated(disk);
    int64_t result = proxy->GetDiskById(diskId, disk);
    EXPECT_EQ(result, E_PERMISSION_DENIED);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string uri = "file://com.demo.a/storage/share/files/test.txt";
    uint32_t tokenId = 100;
    uint32_t flag = 0;
    vector<string> uriList(1, uri);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_DESCRIPTOR_ERR));
    std::string uri = "file://com.demo.a/storage/share/files/test.txt";
    uint32_t tokenId = 100;
    uint32_t flag = 0;
    vector<string> uriList(1, uri);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string uri = "file://com.demo.a/storage/share/files/test.txt";
    uint32_t tokenId = 100;
    std::vector<std::string> sharePathList;
    sharePathList.push_back(uri);
    int64_t result = proxy_->DeleteShareFile(tokenId, sharePathList);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_DeleteShareFile_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_DeleteShareFile_0100
 * @tc.name: Storage_manager_proxy_DeleteShareFile_0100
 * @tc.desc: Test function of DeleteShareFile interface for SendRequest failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI7U9Z9
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_DeleteShareFile_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_DeleteShareFile_0100";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(E_WRITE_DESCRIPTOR_ERR));
    std::string uri = "file://com.demo.a/storage/share/files/test.txt";
    uint32_t tokenId = 100;
    std::vector<std::string> sharePathList;
    sharePathList.push_back(uri);
    int64_t result = proxy_->DeleteShareFile(tokenId, sharePathList);
    EXPECT_EQ(result, E_WRITE_DESCRIPTOR_ERR);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_DeleteShareFile_0100";
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 112;
    uint32_t flags = 2; // UserKeys type
    uint32_t result = proxy_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 113;
    uint32_t result = proxy_->DeleteUserKeys(userId);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 114;
    uint32_t result = proxy_->UpdateUserAuth(userId, 0, {}, {}, {});
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 115;
    uint32_t result = proxy_->ActiveUserKey(userId, {}, {});
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 116;
    uint32_t result = proxy_->InactiveUserKey(userId);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 116;
    uint32_t result = proxy_->LockUserScreen(userId);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 120;
    uint32_t result = proxy_->UnlockUserScreen(userId);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    GTEST_LOG_(INFO) << proxy_;
    uint32_t appUid = 0;
    std::string keyId;
    uint32_t result = proxy_->GenerateAppkey(appUid, keyId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "Storage_manager_proxy_GenerateAppkey_0000 end";
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    const std::string keyId;
    uint32_t result = proxy_->DeleteAppkey(keyId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_DeleteAppkey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_MountDfsDocs_001
 * @tc.name: Storage_manager_proxy_MountDfsDocs_001
 * @tc.desc: Test function of MountDfsDocs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_MountDfsDocs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_MountDfsDocs_001";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 120;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    uint32_t result = proxy_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_MountDfsDocs_001";
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 117;
    uint32_t result = proxy_->UpdateKeyContext(userId);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    BundleStats bundleStats;
    int32_t result = proxy_->GetCurrentBundleStats(bundleStats);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    std::string bundleName = "com.ohos.bundleName-0-1";
    std::string bundleDataDirPath = "/data/app/el2/100/base/" + bundleName;
    int32_t uid = 20000000;
    int32_t limitSizeMb = 1000;
    int32_t result = proxy_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t result = proxy_->GetUserStorageStatsByType(userId, storageStats, type);
    EXPECT_GE(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    int32_t size = 1000;
    int32_t oldSize =500;
    int32_t result = proxy_->UpdateMemoryPara(size, oldSize);
    EXPECT_EQ(result, E_OK);
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
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));
    uint32_t userId = 100;
    std::vector<std::string> bundleNames;
    std::vector<int64_t> incrementalBackTimes;
    std::vector<int64_t> pkgFileSizes;
    int32_t result = proxy_->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetBundleStatsForIncrease_0000";
}
} // namespace