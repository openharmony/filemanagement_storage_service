/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "mock/uece_activation_callback_mock.h"
namespace OHOS {
namespace StorageManager {
using namespace std;
using namespace testing::ext;
using namespace StorageDaemon;
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
HWTEST_F(StorageManagerClientTest, Client_manager_service_PrepareAddUser_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_PrepareAddUser_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

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
 * @tc.number: SUB_STORAGE_Client_manager_service_EraseAllUserEncryptedKeys_0000
 * @tc.name: Client_manager_service_EraseAllUserEncryptedKeys_0000
 * @tc.desc: Test function of EraseAllUserEncryptedKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_EraseAllUserEncryptedKeys_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_EraseAllUserEncryptedKeys_0000";
    ASSERT_TRUE(storageManagerClient_ != nullptr);

    int32_t ret = storageManagerClient_->EraseAllUserEncryptedKeys();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Client_manager_service_EraseAllUserEncryptedKeys_0000 end";
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

    ret = storageManagerClient_->UpdateUserAuth(userId, 0, {}, {}, {});
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
    int32_t ret = storageManagerClient_->ActiveUserKey(userId, {}, {});
    EXPECT_TRUE(ret == E_OK);
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
    int32_t ret = storageManagerClient_->ActiveUserKey(userId, {}, {});
    EXPECT_TRUE(ret == E_OK);

    ret = storageManagerClient_->InactiveUserKey(userId);
    EXPECT_TRUE(ret == E_OK);
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
    int32_t ret = storageManagerClient_->UpdateUserAuth(userId, 0, {}, {}, {});
    EXPECT_TRUE(ret == E_OK) << "UpdateUserAuth error";

    ret = storageManagerClient_->UpdateKeyContext(userId);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Client_manager_service_UpdateKeyContext_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_UpdateUseAuthWithRecoveryKey_0000
 * @tc.name: Client_manager_service_UpdateUseAuthWithRecoveryKey_0000
 * @tc.desc: Test function of UpdateUseAuthWithRecoveryKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_UpdateUseAuthWithRecoveryKey_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_UpdateUseAuthWithRecoveryKey_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 108;
    uint64_t secureUid = 0;
    std::vector<std::vector<uint8_t>> plainText;
    int32_t ret = storageManagerClient_->UpdateUseAuthWithRecoveryKey({}, {}, secureUid, userId, plainText);
    EXPECT_TRUE(ret == E_OK) << "UpdateUseAuthWithRecoveryKey error";

    GTEST_LOG_(INFO) << "Client_manager_service_UpdateUseAuthWithRecoveryKey_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_LockUserScreen_0000
 * @tc.name: Client_manager_service_LockUserScreen_0000
 * @tc.desc: Test function of LockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_LockUserScreen_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_LockUserScreen_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 100;
    int32_t ret = storageManagerClient_->LockUserScreen(userId);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Client_manager_service_LockUserScreen_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_UnlockUserScreen_0000
 * @tc.name: Client_manager_service_UnlockUserScreen_0000
 * @tc.desc: Test function of UnlockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_UnlockUserScreen_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_UnlockUserScreen_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 104;
    int32_t ret = storageManagerClient_->UnlockUserScreen(userId, {}, {});
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Client_manager_service_UnlockUserScreen_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_MountDfsDocs_001
 * @tc.name: Client_manager_service_MountDfsDocs_001
 * @tc.desc: Test function of MountDfsDocs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_MountDfsDocs_001";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 104;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    int32_t ret = storageManagerClient_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Client_manager_service_MountDfsDocs_001 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_UMountDfsDocs_001
 * @tc.name: Client_manager_service_UMountDfsDocs_001
 * @tc.desc: Test function of UMountDfsDocs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_UMountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_UMountDfsDocs_001";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 104;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    int32_t ret = storageManagerClient_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Client_manager_service_UMountDfsDocs_001 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_GetLockScreenStatus_0000
 * @tc.name: Client_manager_service_GetLockScreenStatus_0000
 * @tc.desc: Test function of GetLockScreenStatus interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_GetLockScreenStatus_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_GetLockScreenStatus_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    uint32_t userId = 104;
    bool lockScreenStatus = false;
    int32_t ret = storageManagerClient_->GetLockScreenStatus(userId, lockScreenStatus);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Client_manager_service_GetLockScreenStatus_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Client_manager_service_GetFileEncryptStatus_0000
 * @tc.name: Client_manager_service_GetFileEncryptStatus_0000
 * @tc.desc: Test function of GetLockScreenStatus interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_GetFileEncryptStatus_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest-begin Client_manager_service_GetFileEncryptStatus_0000";

    ASSERT_TRUE(storageManagerClient_ != nullptr);
    bool isEncrypted = true;
    uint32_t userId = 104;
    int32_t ret = storageManagerClient_->GetFileEncryptStatus(userId, isEncrypted);
    EXPECT_TRUE(ret == 0);
    GTEST_LOG_(INFO) << "Client_manager_service_GetFileEncryptStatus_0000 end";
}

/**
 * @tc.number: Client_manager_service_RegisterUeceActivationCallback_001
 * @tc.name: Client_manager_service_RegisterUeceActivationCallback_001
 * @tc.desc: Test function of RegisterUeceActivationCallback interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_RegisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Client_manager_service_RegisterUeceActivationCallback_001 begin";

    ASSERT_TRUE(storageManagerClient_ != nullptr);
    sptr<IUeceActivationCallback> ueceCallback(new (std::nothrow) UeceActivationCallbackMock());
    EXPECT_NE(ueceCallback, nullptr);
    int32_t ret = storageManagerClient_->RegisterUeceActivationCallback(ueceCallback);
    EXPECT_TRUE(ret == 0);
    storageManagerClient_->UnregisterUeceActivationCallback();
    GTEST_LOG_(INFO) << "Client_manager_service_RegisterUeceActivationCallback_001 end";
}

/**
 * @tc.number: Client_manager_service_UnregisterUeceActivationCallback_001
 * @tc.name: Client_manager_service_UnregisterUeceActivationCallback_001
 * @tc.desc: Test function of UnregisterUeceActivationCallback interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(StorageManagerClientTest, Client_manager_service_UnregisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Client_manager_service_UnregisterUeceActivationCallback_001 begin";

    ASSERT_TRUE(storageManagerClient_ != nullptr);
    int32_t ret = storageManagerClient_->UnregisterUeceActivationCallback();
    EXPECT_TRUE(ret == 0);
    GTEST_LOG_(INFO) << "Client_manager_service_UnregisterUeceActivationCallback_001 end";
}
}
}
