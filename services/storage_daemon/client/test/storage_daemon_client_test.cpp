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

#include <gtest/gtest.h>

#include "directory_ex.h"

#include "storage_daemon_client.h"
#include "istorage_daemon.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "userdata_dir_info.h"
#include "help_utils.h"
namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class StorageDaemonClientTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    StorageDaemonClient* storageDaemonClient_;
};

void StorageDaemonClientTest::SetUp()
{
    storageDaemonClient_ = new StorageDaemonClient();
}

void StorageDaemonClientTest::TearDown(void)
{
    StorageTest::StorageTestUtils::ClearTestResource();
    if (storageDaemonClient_ != nullptr) {
        delete storageDaemonClient_;
        storageDaemonClient_ = nullptr;
    }
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001
 * @tc.desc: Verify the PrepareUserDirs function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID1;
    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->PrepareUserDirs(userid, flags);
    EXPECT_TRUE(ret == E_OK);

    ret = storageDaemonClient_->DestroyUserDirs(userid, flags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_SetDirEncryptionPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_SetDirEncryptionPolicy_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);
    uint32_t userId = 100;
    std::string path = "1.txt";
    uint32_t type = 2;
    int32_t ret = storageDaemonClient_->SetDirEncryptionPolicy(userId, path, type);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_SetDirEncryptionPolicy_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_StartUser_001
 * @tc.desc: check the StartUser function when args are normal
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StartUser_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID2;
    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    storageDaemonClient_->DestroyUserDirs(userid, flags);
    int32_t ret = storageDaemonClient_->PrepareUserDirs(userid, flags);
    EXPECT_TRUE(ret == E_OK) << "PrepareUserDirs error";
    ret = storageDaemonClient_->StartUser(userid);
    EXPECT_TRUE(ret == E_OK) << "StartUser error";

    storageDaemonClient_->StopUser(userid);
    storageDaemonClient_->DestroyUserDirs(userid, flags);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StartUser_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001
 * @tc.desc: Verify the PrepareUserSpace function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID3;
    std::string volId = "vol-1-1";
    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->PrepareUserSpace(userid, volId, flags);
    EXPECT_TRUE(ret == E_OK);

    storageDaemonClient_->DestroyUserSpace(userid, volId, flags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_MountDfsDocs_001
 * @tc.desc: Verify the MountDfsDocs function when args are normal.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_MountDfsDocs_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userId = StorageTest::USER_ID3;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    std::string path = "/mnt/data/" + std::to_string(userId) + "/hmdfs/";
    OHOS::ForceRemoveDirectory(path);
    int32_t ret = storageDaemonClient_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_OK);

    OHOS::ForceCreateDirectory(path);
    ret = storageDaemonClient_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_OK);
    OHOS::ForceRemoveDirectory(path);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_MountDfsDocs_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_UMountDfsDocs_001
 * @tc.desc: Verify the UMountDfsDocs function when args are normal.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UMountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UMountDfsDocs_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userId = StorageTest::USER_ID3;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    int32_t ret = storageDaemonClient_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UMountDfsDocs_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_ActiveUserKey_001
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_ActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_ActiveUserKey_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID4;
    int32_t ret = storageDaemonClient_->ActiveUserKey(userid, {}, {});
    EXPECT_TRUE(ret == E_OK);

    ret = storageDaemonClient_->InactiveUserKey(userid);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_ActiveUserKey_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID5;
    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->PrepareUserDirs(userid, flags);
    ret = storageDaemonClient_->StartUser(userid);
    EXPECT_TRUE(ret == E_OK) << "StartUser error";

    ret = storageDaemonClient_->UpdateUserAuth(userid, 0, {}, {}, {});
    EXPECT_TRUE(ret == E_OK) << "UpdateUserAuth error";

    storageDaemonClient_->StopUser(userid);
    storageDaemonClient_->DestroyUserDirs(userid, flags);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID1;
    int32_t ret = storageDaemonClient_->UpdateUserAuth(userid, 0, {}, {}, {});
    EXPECT_TRUE(ret == E_OK) << "UpdateUserAuth error";

    ret = storageDaemonClient_->UpdateKeyContext(userid);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_UnlockUserScreen_001
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UnlockUserScreen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UnlockUserScreen_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID1;

    int32_t ret = storageDaemonClient_->LockUserScreen(userid);
    ASSERT_TRUE(ret == E_OK);

    ret = storageDaemonClient_->UnlockUserScreen(userid, {}, {});
    ASSERT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UnlockUserScreen_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_GetLockScreenStatus_001
 * @tc.desc: Verify the GetLockScreenStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_GetLockScreenStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GetLockScreenStatus_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID1;
    bool lockScreenStatus = false;
    int32_t ret = storageDaemonClient_->GetLockScreenStatus(userid, lockScreenStatus);
    ASSERT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GetLockScreenStatus_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_GenerateAppkey_001
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_GenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GenerateAppkey_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = StorageTest::USER_ID1;
    uint32_t appUid = 0;
    std::string keyId = "keyId";

    int32_t ret = storageDaemonClient_->GenerateAppkey(userid, appUid, keyId);
    EXPECT_EQ(ret, E_OK);

    ret = storageDaemonClient_->DeleteAppkey(userid, keyId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GenerateAppkey_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_FscryptEnable_001
 * @tc.desc: Verify the FscryptEnable function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_FscryptEnable_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_FscryptEnable_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    std::string fscryptOptions = "test";
    int32_t ret = storageDaemonClient_->FscryptEnable(fscryptOptions);
    EXPECT_EQ(ret, -EINVAL);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_FscryptEnable_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_UpdateUseAuthWithRecoveryKey_001
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UpdateUseAuthWithRecoveryKey_001,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUseAuthWithRecoveryKey_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    uint64_t secureUid = 1;
    uint32_t userId = 100;
    std::vector<std::vector<uint8_t>> plainText;
    int32_t ret = storageDaemonClient_->UpdateUseAuthWithRecoveryKey({}, {}, secureUid, userId, plainText);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUseAuthWithRecoveryKey_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_GetFileEncryptStatus_001
 * @tc.desc: Verify the GetFileEncryptStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_GetFileEncryptStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GetFileEncryptStatus_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    uint32_t userId = 100;
    bool isEncrypted = true;
    bool needCheckDirMount = true;
    int32_t ret = storageDaemonClient_->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GetFileEncryptStatus_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_IsFileOccupied_001
 * @tc.desc: Verify the IsFileOccupied function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_IsFileOccupied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_IsFileOccupied_001 start";
    ASSERT_TRUE(storageDaemonClient_ != nullptr);
    std::string path;
    std::vector<std::string> input;
    std::vector<std::string> output;
    bool isOccupy;
    int32_t ret = storageDaemonClient_->IsFileOccupied(path, input, output, isOccupy);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_IsFileOccupied_001 end";
}

HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_ListUserdataDirInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_ListUserdataDirInfo_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);
    std::vector<OHOS::StorageManager::UserdataDirInfo> scanDirs;
    int32_t ret = storageDaemonClient_->ListUserdataDirInfo(scanDirs);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_ListUserdataDirInfo_001 end";
}
}
}