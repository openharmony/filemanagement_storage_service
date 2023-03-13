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

#include <cstdio>
#include <gtest/gtest.h>

#include "client/storage_manager_client.h"
#include "get_self_permissions.h"
#include "ipc/storage_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
using namespace std;
using namespace testing::ext;
class StorageManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    StorageManagerClient* storageManagerClient_;
};

void StorageManagerClientTest::SetUp()
{
    storageManagerClient_ = new StorageManagerClient();
}

void StorageManagerClientTest::TearDown(void)
{
    if (storageManagerClient_ != nullptr) {
        delete storageManagerClient_;
        storageManagerClient_ = nullptr;
    }
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_service_PrepareAddUser_0000
 * @tc.name: Client_manager_service_PrepareAddUser_0000
 * @tc.desc: Test function of PrepareAddUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_PrepareAddUser_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_PrepareAddUser_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    vector<string> perms;
    perms.push_back("ohos.permission.STORAGE_MANAGER");
    uint64_t tokenId = 0;
    PermissionUtilsTest::SetAccessTokenPermission("StorageManagerClientTest", perms, tokenId);
    ASSERT_TRUE(tokenId != 0);

    uint32_t userId = 121;
    int32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->PrepareAddUser(userId, flag);
    GTEST_LOG_(INFO) << ret;
    EXPECT_TRUE(ret == E_OK);

    storageManagerClient_->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "Client_manager_service_PrepareAddUser_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_service_RemoveUser_0000
 * @tc.name: Client_manager_service_RemoveUser_0000
 * @tc.desc: Test function of RemoveUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_RemoveUser_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_RemoveUser_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 102;
    uint32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->PrepareAddUser(userId, flag);
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->RemoveUser(userId, flag);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Client_manager_service_RemoveUser_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_service_GenerateUserKeys_0000
 * @tc.name: Client_manager_service_GenerateUserKeys_0000
 * @tc.desc: Test function of GenerateUserKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_GenerateUserKeys_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_GenerateUserKeys_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 103;
    uint32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->GenerateUserKeys(userId, flag);
    EXPECT_TRUE(ret == E_OK);

    storageManagerClient_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "Client_manager_service_GenerateUserKeys_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_service_DeleteUserKeys_0000
 * @tc.name: Client_manager_service_DeleteUserKeys_0000
 * @tc.desc: Test function of DeleteUserKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_DeleteUserKeys_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_DeleteUserKeys_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 104;
    uint32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->GenerateUserKeys(userId, flag);
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->DeleteUserKeys(userId);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Client_manager_service_DeleteUserKeys_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_service_UpdateUserAuth_0000
 * @tc.name: Client_manager_service_UpdateUserAuth_0000
 * @tc.desc: Test function of UpdateUserAuth interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_UpdateUserAuth_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_UpdateUserAuth_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 105;
    uint32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->PrepareAddUser(userId, flag);
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->UpdateUserAuth(userId, {}, {}, {});
    EXPECT_TRUE(ret == E_OK) << "UpdateUserAuth error";

    storageManagerClient_->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "Client_manager_service_UpdateUserAuth_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_serviceActiveUserKey_0000
 * @tc.name: Client_manager_service_ActiveUserKey_0000
 * @tc.desc: Test function of ActiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_ActiveUserKey_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_ActiveUserKey_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 106;
    uint32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->GenerateUserKeys(userId, flag);
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->ActiveUserKey(userId, {}, {});
    EXPECT_TRUE(ret == E_OK);

    storageManagerClient_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "Client_manager_service_ActiveUserKey_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_InactiveUserKey_0000
 * @tc.name: Client_manager_service_InactiveUserKey_0000
 * @tc.desc: Test function of InactiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_InactiveUserKey_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_InactiveUserKey_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 107;
    uint32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->GenerateUserKeys(userId, flag);
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->ActiveUserKey(userId, {}, {});
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->InactiveUserKey(userId);
    EXPECT_TRUE(ret == E_OK);

    storageManagerClient_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "Client_manager_service_InactiveUserKey_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_UpdateKeyContext_0000
 * @tc.name: Client_manager_service_UpdateKeyContext_0000
 * @tc.desc: Test function of UpdateKeyContext interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_UpdateKeyContext_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_UpdateKeyContext_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 108;
    uint32_t flag = CRYPTO_FLAG_EL2;
    int32_t ret = storageManagerClient_->GenerateUserKeys(userId, flag);
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->UpdateUserAuth(userId, {}, {}, {});
    EXPECT_TRUE(ret == E_OK) << "UpdateUserAuth error";

    ret = storageManagerClient_->UpdateKeyContext(userId);
    EXPECT_TRUE(ret == E_OK);

    storageManagerClient_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "Client_manager_service_UpdateKeyContext_0000 end";
}
}
}
