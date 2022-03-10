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

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "volume/volume_manager_service.h"
#include "ipc/storage_manager_proxy.h"
#include "storage_manager_service_mock.h"

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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(samgr != nullptr) << "Storage_manager_proxy_PrepareAddUser_0000 fail to get GetSystemAbilityManager";
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_TRUE(remote != nullptr) << "GetSystemAbility failed";
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    ASSERT_TRUE(proxy != nullptr) << "fail to get proxy";
    int32_t result = proxy->PrepareAddUser(userId);
    EXPECT_EQ(result, 0);
    proxy->RemoveUser(userId);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(samgr != nullptr) << "Storage_manager_proxy_PrepareAddUser_0001 fail to get GetSystemAbilityManager";
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_TRUE(remote != nullptr) << "GetSystemAbility failed";
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    ASSERT_TRUE(proxy != nullptr) << "fail to get proxy";
    int32_t result = proxy->PrepareAddUser(userId);
    EXPECT_NE(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    int32_t result = proxy->PrepareAddUser(userId);
    EXPECT_NE(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    int32_t result = proxy->PrepareAddUser(userId);
    EXPECT_EQ(result, 0);
    proxy->RemoveUser(userId);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    int32_t result = proxy->RemoveUser(userId);
    EXPECT_EQ(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    int32_t result = proxy->RemoveUser(userId);
    EXPECT_EQ(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    int32_t result = proxy->RemoveUser(userId);
    EXPECT_NE(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    int32_t result = proxy->PrepareStartUser(userId);
    EXPECT_EQ(result, 0);
    proxy->StopUser(userId);
    proxy->RemoveUser(userId);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareStartUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_PrepareStartUser_0001
 * @tc.name: Storage_manager_proxy_PrepareStartUser_0001
 * @tc.desc: Test function of PrepareStartUser interface for SUCCESS which Repeated start.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_PrepareStartUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_PrepareStartUser_0001";
    int32_t userId = 106;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    proxy->PrepareStartUser(userId);
    int32_t result = proxy->PrepareStartUser(userId);
    EXPECT_EQ(result, 0);
    proxy->StopUser(userId);
    proxy->RemoveUser(userId);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_PrepareStartUser_0001";
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
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    int32_t result = proxy->PrepareStartUser(userId);
    EXPECT_NE(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    int32_t result = proxy->PrepareStartUser(userId);
    EXPECT_NE(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    proxy->PrepareStartUser(userId);
    int32_t result = proxy->StopUser(userId);
    EXPECT_EQ(result, 0);
    proxy->RemoveUser(userId);
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
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    int32_t result = proxy->StopUser(userId);
    EXPECT_NE(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    int32_t result = proxy->StopUser(userId);
    EXPECT_NE(result, 0);
    proxy->RemoveUser(userId);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->PrepareAddUser(userId);
    proxy->PrepareStartUser(userId);
    int32_t result = proxy->StopUser(userId);
    EXPECT_NE(result, 0);
    proxy->RemoveUser(userId);
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
    std::string volumeUuid = "111";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    int64_t result = proxy->GetFreeSizeOfVolume(volumeUuid);
    EXPECT_NE(result, 0);
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
    std::string volumeUuid = "112";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    int64_t result = proxy->GetTotalSizeOfVolume(volumeUuid);
    EXPECT_NE(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    std::vector<int64_t> result = proxy->GetBundleStats(pkgName);
    GTEST_LOG_(INFO) << result[0];
    GTEST_LOG_(INFO) << result[1];
    GTEST_LOG_(INFO) << result[2];
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetBundleStats_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_proxy_GetBundleStats_0001
 * @tc.name: Storage_manager_proxy_GetBundleStats_0001
 * @tc.desc: Test function of GetBundleStats interface for Parameters ERROR which pkgName is not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK101
 */
HWTEST_F(StorageManagerProxyTest, Storage_manager_proxy_GetBundleStats_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-begin Storage_manager_proxy_GetBundleStats_0001";
    std::string pkgName = "ohos.acts.storage.zzzz";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    std::vector<int64_t> result = proxy->GetBundleStats(pkgName);
    GTEST_LOG_(INFO) << result[0];
    GTEST_LOG_(INFO) << result[1];
    GTEST_LOG_(INFO) << result[2];
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetBundleStats_0001";
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
    VolumeCore vc;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->NotifyVolumeCreated(vc);
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
    std::string volumeId = "118";
    int32_t fsType = 1;
    std::string fsUuid = "119";
    std::string path = "/";
    std::string description = "130";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->NotifyVolumeMounted(volumeId, fsType, fsUuid, path, description);
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
    std::string volumeId = "120";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->NotifyVolumeDestroyed(volumeId);
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
    std::string volumeId = "121";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    int32_t result = proxy_->Mount(volumeId);
    EXPECT_EQ(result, 0);
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
    std::string volumeId = "122";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    int32_t result = proxy_->Unmount(volumeId);
    EXPECT_EQ(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    std::vector<VolumeExternal> result = proxy->GetAllVolumes();
    EXPECT_NE(result.size(), 0);
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
    std::string diskId = "124";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "124";
    int32_t flag = 1;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->NotifyDiskCreated(disk);
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
    std::string diskId = "123";
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    proxy->NotifyDiskDestroyed(diskId);
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
    std::string volumeId = "121";
    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageManagerServiceMock::InvokeSendRequest));

    std::string diskId = "124";
    int32_t type = 1;
    int32_t result = proxy_->Partition(diskId, type);
    EXPECT_EQ(result, 0);
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
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    auto proxy = iface_cast<StorageManager::IStorageManager>(remote);
    std::vector<Disk> result = proxy->GetAllDisks();
    EXPECT_NE(result.size(), 0);
    GTEST_LOG_(INFO) << "StorageManagerProxyTest-end Storage_manager_proxy_GetAllDisks_0000";
}
} // namespace