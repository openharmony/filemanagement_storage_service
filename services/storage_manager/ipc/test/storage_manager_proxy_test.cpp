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

#include "ipc/storage_manager_proxy.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class StorageManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

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
 * @tc.desc: Test function of PrepareAddUser interface for Logic ERROR which Repeated add.
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
    EXPECT_NE(result, 0);
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
 * @tc.desc: Test function of RemoveUser interface for Logic ERROR which remove userId not exist.
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
    EXPECT_NE(result, 0);
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
 * @tc.desc: Test function of PrepareStartUser interface for Logic ERROR which Repeated start.
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
    EXPECT_NE(result, 0);
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
} // namespace