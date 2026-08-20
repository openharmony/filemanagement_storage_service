/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <singleton.h>

#include "istorage_space_manager.h"
#include "storage_space_manager_client.h"
#include "storage_space_manager_errno.h"
#include "system_ability_mock.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

class StorageSpaceManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        g_mockSamgrReturnNull = true;
        DelayedSingleton<StorageSpaceManagerClient>::DestroyInstance();
    }
    void TearDown()
    {
        DelayedSingleton<StorageSpaceManagerClient>::DestroyInstance();
    }
};

/* ---------- ResetProxy ---------- */

HWTEST_F(StorageSpaceManagerClientTest, ResetProxy_CleanState, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    EXPECT_EQ(client->ResetProxy(), E_OK);
}

HWTEST_F(StorageSpaceManagerClientTest, ResetProxy_MultipleCalls, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    EXPECT_EQ(client->ResetProxy(), E_OK);
    EXPECT_EQ(client->ResetProxy(), E_OK);
    EXPECT_EQ(client->ResetProxy(), E_OK);
}

/* ---------- Connect (error path: samgr == nullptr) ---------- */

HWTEST_F(StorageSpaceManagerClientTest, Connect_SamgrNull_ReturnsEsaIsNullptr, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);

    int64_t dummy = 0;
    // All delegate methods should return E_SA_IS_NULLPTR because Connect fails
    EXPECT_EQ(client->GetTotalSize(dummy), E_SA_IS_NULLPTR);
    EXPECT_EQ(client->GetSystemSize(dummy), E_SA_IS_NULLPTR);
    EXPECT_EQ(client->GetFreeSize(dummy), E_SA_IS_NULLPTR);
    EXPECT_EQ(client->GetTotalInodes(dummy), E_SA_IS_NULLPTR);
    EXPECT_EQ(client->GetFreeInodes(dummy), E_SA_IS_NULLPTR);
    EXPECT_EQ(client->CleanBundleCache(100), E_SA_IS_NULLPTR);
}

/* ---------- Each delegate method returns Connect error ---------- */

HWTEST_F(StorageSpaceManagerClientTest, GetTotalSize_ConnectFails, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    int64_t size = 0;
    EXPECT_EQ(client->GetTotalSize(size), E_SA_IS_NULLPTR);
}

HWTEST_F(StorageSpaceManagerClientTest, GetSystemSize_ConnectFails, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    int64_t size = 0;
    EXPECT_EQ(client->GetSystemSize(size), E_SA_IS_NULLPTR);
}

HWTEST_F(StorageSpaceManagerClientTest, GetFreeSize_ConnectFails, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    int64_t size = 0;
    EXPECT_EQ(client->GetFreeSize(size), E_SA_IS_NULLPTR);
}

HWTEST_F(StorageSpaceManagerClientTest, GetTotalInodes_ConnectFails, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    int64_t inodes = 0;
    EXPECT_EQ(client->GetTotalInodes(inodes), E_SA_IS_NULLPTR);
}

HWTEST_F(StorageSpaceManagerClientTest, GetFreeInodes_ConnectFails, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    int64_t inodes = 0;
    EXPECT_EQ(client->GetFreeInodes(inodes), E_SA_IS_NULLPTR);
}

HWTEST_F(StorageSpaceManagerClientTest, CleanBundleCache_ConnectFails, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    EXPECT_EQ(client->CleanBundleCache(100), E_SA_IS_NULLPTR);
}

/* ---------- GetDataShareService ---------- */

/**
 * @tc.number: SUB_STORAGE_Client_GetDataShareService_0001
 * @tc.name: GetDataShareService_BasicFunction
 * @tc.desc: Test GetDataShareService basic function returns E_FAIL
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerClientTest, GetDataShareService_BasicFunction, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);

    std::string uri = "datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record";
    sptr<IRemoteObject> remoteObject;
    
    int32_t ret = client->GetDataShareService(uri, remoteObject);
    
    EXPECT_EQ(ret, E_FAIL);
    EXPECT_EQ(remoteObject, nullptr);
}

HWTEST_F(StorageSpaceManagerClientTest, LoadSystemAbilityFail_Basic, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->LoadSystemAbilityFail();
    EXPECT_EQ(client->storageSpaceManager_, nullptr);
    EXPECT_TRUE(client->loadFinished_);
}

HWTEST_F(StorageSpaceManagerClientTest, LoadSystemAbilitySuccess_NullRemote, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    sptr<IRemoteObject> nullRemote = nullptr;
    client->LoadSystemAbilitySuccess(nullRemote);
    EXPECT_TRUE(client->loadFinished_);
    EXPECT_EQ(client->storageSpaceManager_, nullptr);
}

HWTEST_F(StorageSpaceManagerClientTest, LoadSystemAbilityFail_ThenResetProxy, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->LoadSystemAbilityFail();
    EXPECT_TRUE(client->loadFinished_);
    EXPECT_EQ(client->ResetProxy(), E_OK);
}

HWTEST_F(StorageSpaceManagerClientTest, LoadSystemAbilitySuccess_MultipleCalls, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    sptr<IRemoteObject> nullRemote = nullptr;
    client->LoadSystemAbilitySuccess(nullRemote);
    EXPECT_TRUE(client->loadFinished_);
    client->loadFinished_ = false;
    client->LoadSystemAbilitySuccess(nullRemote);
    EXPECT_TRUE(client->loadFinished_);
}

HWTEST_F(StorageSpaceManagerClientTest, LoadSystemAbilityFail_MultipleCalls, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->LoadSystemAbilityFail();
    EXPECT_TRUE(client->loadFinished_);
    client->loadFinished_ = false;
    client->LoadSystemAbilityFail();
    EXPECT_TRUE(client->loadFinished_);
}

HWTEST_F(StorageSpaceManagerClientTest, OnAddSystemAbility_ClearsState, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->loadFinished_ = true;
    client->OnAddSystemAbility();
    EXPECT_EQ(client->storageSpaceManager_, nullptr);
    EXPECT_EQ(client->deathRecipient_, nullptr);
    EXPECT_FALSE(client->loadFinished_);
}

HWTEST_F(StorageSpaceManagerClientTest, SsmDeathRecipient_OnRemoteDied, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    StorageSpaceManagerClient::SsmDeathRecipient recipient;
    wptr<IRemoteObject> remote(nullptr);
    recipient.OnRemoteDied(remote);
    EXPECT_EQ(client->storageSpaceManager_, nullptr);
}

HWTEST_F(StorageSpaceManagerClientTest, SystemAbilityStatusListener_OnRemoveSystemAbility, TestSize.Level1)
{
    StorageSpaceManagerClient::SystemAbilityStatusListener listener;
    listener.OnRemoveSystemAbility(8650, "");
    SUCCEED();
}

HWTEST_F(StorageSpaceManagerClientTest, SystemAbilityStatusListener_OnAddSystemAbility, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->loadFinished_ = true;
    StorageSpaceManagerClient::SystemAbilityStatusListener listener;
    listener.OnAddSystemAbility(8650, "");
    EXPECT_EQ(client->storageSpaceManager_, nullptr);
    EXPECT_FALSE(client->loadFinished_);
}

HWTEST_F(StorageSpaceManagerClientTest, SubscribeSsmSA_SamgrNull, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->SubscribeSsmSA();
    EXPECT_EQ(client->statusListener_, nullptr);
}

HWTEST_F(StorageSpaceManagerClientTest, GetProxy_SamgrNull, TestSize.Level1)
{
    g_mockSamgrReturnNull = true;
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->ResetProxy();
    auto proxy = client->GetProxy();
    EXPECT_EQ(proxy, nullptr);
}

HWTEST_F(StorageSpaceManagerClientTest, ResetProxy_WithDeathRecipient, TestSize.Level1)
{
    auto *client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    ASSERT_NE(client, nullptr);
    client->deathRecipient_ = new (std::nothrow) StorageSpaceManagerClient::SsmDeathRecipient();
    EXPECT_EQ(client->ResetProxy(), E_OK);
    EXPECT_EQ(client->storageSpaceManager_, nullptr);
    EXPECT_EQ(client->deathRecipient_, nullptr);
}

} // namespace StorageSpaceManager
} // namespace OHOS
