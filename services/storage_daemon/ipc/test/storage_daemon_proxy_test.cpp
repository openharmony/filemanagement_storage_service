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

#include <gtest/gtest.h>

#include "ipc/istorage_daemon.h"
#include "ipc/storage_daemon_ipc_interface_code.h"
#include "ipc/storage_daemon_proxy.h"
#include "storage_daemon_service_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

namespace {
    constexpr int32_t USER_ID1 = 100;
}

class StorageDaemonProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown() {};

    std::shared_ptr<StorageDaemonProxy> proxy_ = nullptr;
    sptr<StorageDaemonServiceMock> mock_ = nullptr;
};

void StorageDaemonProxyTest::SetUp()
{
    mock_ = new StorageDaemonServiceMock();
    proxy_ = std::make_shared<StorageDaemonProxy>(mock_);
}

/**
 * @tc.name: StorageDaemonProxyTest_Shutdown_001
 * @tc.desc: Verify the Shutdown function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_Shutdown_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Shutdown_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->Shutdown();
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::SHUTDOWN) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Shutdown_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_PrepareUserDirs_001
 * @tc.desc: Verify the PrepareUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_PrepareUserDirs_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->PrepareUserDirs(USER_ID1, IStorageDaemon::CRYPTO_FLAG_EL1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::PREPARE_USER_DIRS) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_DestroyUserDirs_001
 * @tc.desc: Verify the DestroyUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_DestroyUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DestroyUserDirs_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->DestroyUserDirs(USER_ID1, IStorageDaemon::CRYPTO_FLAG_EL1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::DESTROY_USER_DIRS) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DestroyUserDirs_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_StartUser_001
 * @tc.desc: Verify the StartUser function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StartUser_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->StartUser(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::START_USER) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StartUser_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_StopUser_001
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_StopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StopUser_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->StopUser(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::STOP_USER) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StopUser_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_Mount_001
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_Mount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Mount_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string volId = "vol-0-1";
    uint32_t flag = 1;
    int32_t ret = proxy_->Mount(volId, flag);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::MOUNT) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Mount_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_UMount_001
 * @tc.desc: Verify the UMount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_UMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UMount_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string volId = "vol-0-2";
    int32_t ret = proxy_->UMount(volId);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::UMOUNT) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UMount_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_Check_001
 * @tc.desc: Verify the Check function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_Check_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Check_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string volId = "vol-0-3";
    int32_t ret = proxy_->Check(volId);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::CHECK) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Check_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_Format_001
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_Format_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Format_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string volId = "vol-0-4";
    string fsType = "exfat";
    int32_t ret = proxy_->Format(volId, fsType);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::FORMAT) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Format_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_Partition_001
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_Partition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Partition_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string diskId = "disk-0-1";
    int32_t type = 0;
    int32_t ret = proxy_->Partition(diskId, type);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::PARTITION) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Partition_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_SetVolumeDescription_001
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_SetVolumeDescription_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_SetVolumeDescription_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string volId = "vol-0-1";
    string description = "description-1";
    int32_t ret = proxy_->SetVolumeDescription(volId, description);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::SET_VOL_DESC) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_SetVolumeDescription_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_InitGlobalKey_001
 * @tc.desc: Verify the InitGlobalKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_InitGlobalKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalKey_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->InitGlobalKey();
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_KEY) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalKey_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_InitGlobalUserKeys_001
 * @tc.desc: Verify the InitGlobalUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_InitGlobalUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalUserKeys_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->InitGlobalUserKeys();
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_USER_KEYS) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalUserKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_GenerateUserKeys_001
 * @tc.desc: Verify the GenerateUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_GenerateUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_GenerateUserKeys_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->GenerateUserKeys(USER_ID1, IStorageDaemon::CRYPTO_FLAG_EL1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::CREATE_USER_KEYS) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_GenerateUserKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_DeleteUserKeys_001
 * @tc.desc: Verify the DeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_DeleteUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DeleteUserKeys_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->DeleteUserKeys(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::DELETE_USER_KEYS) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DeleteUserKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_UpdateUserAuth_001
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_UpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UpdateUserAuth_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->UpdateUserAuth(USER_ID1, 0, {}, {}, {});
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UpdateUserAuth_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_ActiveUserKey_001
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_ActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_ActiveUserKey_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->ActiveUserKey(USER_ID1, {}, {});
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::ACTIVE_USER_KEY) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_ActiveUserKey_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_InactiveUserKey_001
 * @tc.desc: Verify the InactiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_InactiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InactiveUserKey_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->InactiveUserKey(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::INACTIVE_USER_KEY) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InactiveUserKey_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_LockUserScreen_001
 * @tc.desc: Verify the LockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_LockUserScreen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_LockUserScreen_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->LockUserScreen(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::LOCK_USER_SCREEN) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_LockUserScreen_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_UnlockUserScreen_001
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_UnlockUserScreen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UnlockUserScreen_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->UnlockUserScreen(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::UNLOCK_USER_SCREEN) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UnlockUserScreen_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_GenerateAppkey_001
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_GenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_GenerateAppkey_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    uint32_t appUid = 0;
    std::string keyId;
    int32_t ret = proxy_->GenerateAppkey(USER_ID1, appUid, keyId);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::GENERATE_APP_KEY) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_GenerateAppkey_001 end";
}


/**
 * @tc.name: StorageDaemonProxyTest_GenerateAppkey_001
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_DeleteAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DeleteAppkey_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    std::string keyId;
    int32_t ret = proxy_->DeleteAppkey(USER_ID1, keyId);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::DELETE_APP_KEY) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DeleteAppkey_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_MountDfsDocs_001
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_MountDfsDocs_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    int32_t ret = proxy_->MountDfsDocs(USER_ID1, relativePath, networkId, deviceId);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::MOUNT_DFS_DOCS) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_MountDfsDocs_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_UpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UpdateKeyContext_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->UpdateKeyContext(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::UPDATE_KEY_CONTEXT) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UpdateKeyContext_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_SetBundleQuota_001
 * @tc.desc: Verify the SetBundleQuota function.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_SetBundleQuota_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_SetBundleQuota_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string bundleName = "com.ohos.bundleName-0-1";
    string bundleDataDirPath = "/data/app/el2/100/base/" + bundleName;
    int32_t uid = 20000000;
    int32_t limitSizeMb = 1000;
    int32_t ret = proxy_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::SET_BUNDLE_QUOTA) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_SetBundleQuota_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_UpdateMemoryPara_001
 * @tc.desc: Verify the UpdateMemoryPara function.
 * @tc.type: FUNC
 * @tc.require: I8ZBB3
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_UpdateMemoryPara_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UpdateMemoryParaa_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t size = 1000;
    int32_t oldSize = 500;
    int32_t ret = proxy_->UpdateMemoryPara(size, oldSize);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(static_cast<int32_t>(StorageDaemonInterfaceCode::UPDATE_MEM_PARA) == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_UpdateMemoryPara_001 end";
}

} // STORAGE_DAEMON
} // OHOS