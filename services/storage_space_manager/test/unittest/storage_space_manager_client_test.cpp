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

} // namespace StorageSpaceManager
} // namespace OHOS
