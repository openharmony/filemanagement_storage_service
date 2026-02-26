/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "storage_daemon_test.h"

namespace OHOS {
namespace StorageDaemon {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace StorageService;

#ifdef USER_CRYPTO_MIGRATE_KEY
/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Nato_001
 * @tc.desc: Verify the ActiveUserKey4Nato when token or secret is not empty.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Nato_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    // token not empty and secret is empty
    token_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Nato(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);
    RefreshConfigDir();

    // token is empty and secret not empty
    token_.clear();
    secret_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Nato(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);
    RefreshConfigDir();

    // token not empty and secret not empty
    token_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Nato(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Nato_002
 * @tc.desc: Verify the ActiveUserKey4Nato when EL2 failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Nato_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->ActiveUserKey4Nato(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Nato_003
 * @tc.desc: Verify the ActiveUserKey4Nato when EL3 failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Nato_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->ActiveUserKey4Nato(userId_, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Nato_004
 * @tc.desc: Verify the ActiveUserKey4Nato when EL4 failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Nato_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->ActiveUserKey4Nato(userId_, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Nato_005
 * @tc.desc: Verify the ActiveUserKey4Nato when all operations succeed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Nato_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->ActiveUserKey4Nato(userId_, token_, secret_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Update_001
 * @tc.desc: Verify the ActiveUserKey4Update when token and secret is empty.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Update_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_EQ(storageDaemon_->ActiveUserKey4Update(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Update_002
 * @tc.desc: Verify the ActiveUserKey4Update when PrepareUserDirsAndUpdateUserAuth failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Update_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL2_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL2_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ERR));

    token_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Update(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Update_003
 * @tc.desc: Verify the ActiveUserKey4Update when SaveStringToFile failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Update_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL2_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL2_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    g_saveStringToFile = false;

    token_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Update(userId_, token_, secret_), E_SYS_KERNEL_ERR);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Update_004
 * @tc.desc: Verify the ActiveUserKey4Update when ActiveUserKeyAndPrepareElX failed with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Update_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL2_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL2_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_ERR)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    g_saveStringToFile = true;
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_OK));

    secret_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Update(userId_, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Update_005
 * @tc.desc: Verify the ActiveUserKey4Update when ActiveUserKeyAndPrepareElX failed with E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Update_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL2_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL2_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_ACTIVE_REPEATED)).WillOnce(Return(E_ACTIVE_REPEATED)).WillOnce(Return(E_ACTIVE_REPEATED));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    g_saveStringToFile = true;
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_OK));

    secret_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Update(userId_, token_, secret_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Update_006
 * @tc.desc: Verify the ActiveUserKey4Update when NotifyUeceActivation failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Update_006, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL2_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL2_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    g_saveStringToFile = true;
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_ERR));

    secret_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Update(userId_, token_, secret_), E_UNLOCK_APP_KEY2_FAILED);
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Update_007
 * @tc.desc: Verify the ActiveUserKey4Update when all operations succeed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Update_007, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL2_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL2_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    g_saveStringToFile = true;
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_OK));

    secret_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Update(userId_, token_, secret_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_ClearNatoRestoreKey_001
 * @tc.desc: Verify the ClearNatoRestoreKey.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ClearNatoRestoreKey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    storageDaemon_->ClearNatoRestoreKey(userId_, EL2_KEY, false);
    RefreshConfigDir();

    storageDaemon_->ClearNatoRestoreKey(userId_, EL2_KEY, true);
    RefreshConfigDir();

    storageDaemon_->ClearNatoRestoreKey(userId_, EL3_KEY, true);
    RefreshConfigDir();

    storageDaemon_->ClearNatoRestoreKey(userId_, EL4_KEY, true);
}
#endif

/**
 * @tc.name: StorageDaemonTest_InactiveUserKey_001
 * @tc.desc: Verify the InactiveUserKey.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_InactiveUserKey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, InActiveUserKey(_)).WillOnce(Return(E_ERR))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerExtMock_, InActiveUserKey(_)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->InactiveUserKey(userId_), E_ERR);
    EXPECT_EQ(storageDaemon_->InactiveUserKey(userId_), E_OK);
#endif
    EXPECT_EQ(storageDaemon_->InactiveUserKey(userId_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_LockUserScreen_001
 * @tc.desc: Verify the LockUserScreen.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_LockUserScreen_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, LockUserScreen(_)).WillOnce(Return(E_ERR))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerExtMock_, InActiveUserKey(_)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->LockUserScreen(userId_), E_ERR);
    EXPECT_EQ(storageDaemon_->LockUserScreen(userId_), E_OK);
#endif
    EXPECT_EQ(storageDaemon_->LockUserScreen(userId_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_UnlockUserScreen_001
 * @tc.desc: Verify the UnlockUserScreen.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_UnlockUserScreen_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, UnlockUserScreen(_, _, _)).WillOnce(Return(E_ERR))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerExtMock_, ActiveUserKey(_, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->UnlockUserScreen(userId_, token_, secret_), E_ERR);
    EXPECT_EQ(storageDaemon_->UnlockUserScreen(userId_, token_, secret_), E_OK);
#endif
    EXPECT_EQ(storageDaemon_->UnlockUserScreen(userId_, token_, secret_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_GenerateAppkey_001
 * @tc.desc: Verify the GenerateAppkey.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GenerateAppkey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string keyId;

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateAppkey(_, _, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->GenerateAppkey(userId_, 0, keyId, false), E_ERR);
#endif
    EXPECT_EQ(storageDaemon_->GenerateAppkey(userId_, 0, keyId, false), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_DeleteAppkey_001
 * @tc.desc: Verify the DeleteAppkey.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_DeleteAppkey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string keyId;

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, DeleteAppkey(_, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->DeleteAppkey(userId_, keyId), E_ERR);
#endif
    EXPECT_EQ(storageDaemon_->DeleteAppkey(userId_, keyId), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_UpdateKeyContext_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, UpdateKeyContext(_, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->UpdateKeyContext(userId_, false), E_ERR);
#endif
    EXPECT_EQ(storageDaemon_->UpdateKeyContext(userId_, false), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_GetFileEncryptStatus_001
 * @tc.desc: Verify the GetFileEncryptStatus.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GetFileEncryptStatus_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    bool isEncrypted = false;

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GetFileEncryptStatus(_, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->GetFileEncryptStatus(userId_, isEncrypted, false), E_ERR);
#endif
    EXPECT_EQ(storageDaemon_->GetFileEncryptStatus(userId_, isEncrypted, false), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_InactiveUserPublicDirKey_001
 * @tc.desc: Verify the InactiveUserPublicDirKey.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_InactiveUserPublicDirKey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerExtMock_, InActiveUserKey(_)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->InactiveUserPublicDirKey(userId_), E_ERR);
#endif
    EXPECT_EQ(storageDaemon_->InactiveUserPublicDirKey(userId_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_UpdateUserPublicDirPolicy_001
 * @tc.desc: Verify the UpdateUserPublicDirPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_UpdateUserPublicDirPolicy_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerExtMock_, UpdateUserPublicDirPolicy(_)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->UpdateUserPublicDirPolicy(userId_), E_ERR);
#endif
    EXPECT_EQ(storageDaemon_->UpdateUserPublicDirPolicy(userId_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy when level not in EL1_SYS_KEY and EL4_USER_KEY.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_SetDirEncryptionPolicy_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string dirPath;
    uint32_t level = 10;

    EXPECT_EQ(storageDaemon_->SetDirEncryptionPolicy(userId_, dirPath, level), E_PARAMS_INVALID);
#endif
}

/**
 * @tc.name: StorageDaemonTest_SetDirEncryptionPolicy_002
 * @tc.desc: Verify the SetDirEncryptionPolicy when level and userId not allow.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_SetDirEncryptionPolicy_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string dirPath;

    EXPECT_EQ(storageDaemon_->SetDirEncryptionPolicy(userId_, dirPath, EL1_SYS_KEY), E_PARAMS_INVALID);
#endif
}

/**
 * @tc.name: StorageDaemonTest_SetDirEncryptionPolicy_003
 * @tc.desc: Verify the SetDirEncryptionPolicy when dirPath is not support.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_SetDirEncryptionPolicy_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string dirPath = "test";

    EXPECT_EQ(storageDaemon_->SetDirEncryptionPolicy(GLOBAL_USER_ID, dirPath, EL1_SYS_KEY), E_NON_ACCESS);
#endif
}

/**
 * @tc.name: StorageDaemonTest_SetDirEncryptionPolicy_004
 * @tc.desc: Verify the SetDirEncryptionPolicy when SetDirEncryptionPolicy failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_SetDirEncryptionPolicy_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string dirPath = std::string(ANCO_DIR) + std::to_string(userId_);

    std::string path = "/data/virt_service";
    std::error_code errCode;
    if (std::filesystem::exists(path, errCode)) {
        std::filesystem::rename(path, path + "_bak", errCode);
    }
    std::filesystem::create_directories(dirPath, errCode);
    EXPECT_CALL(*keyManagerMock_, SetDirEncryptionPolicy(_, _, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->SetDirEncryptionPolicy(userId_, dirPath, EL2_USER_KEY), E_ERR);

    std::filesystem::remove_all(path, errCode);
    if (std::filesystem::exists(path + "_bak", errCode)) {
        std::filesystem::rename(path + "_bak", path, errCode);
    }
#endif
}

/**
 * @tc.name: StorageDaemonTest_SetDirEncryptionPolicy_005
 * @tc.desc: Verify the SetDirEncryptionPolicy when all.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_SetDirEncryptionPolicy_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string dirPath = std::string(ANCO_DIR) + std::to_string(userId_);

    std::string path = "/data/virt_service";
    std::error_code errCode;
    if (std::filesystem::exists(path, errCode)) {
        std::filesystem::rename(path, path + "_bak");
    }
    std::filesystem::create_directories(dirPath, errCode);
    EXPECT_CALL(*keyManagerMock_, SetDirEncryptionPolicy(_, _, _)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->SetDirEncryptionPolicy(userId_, dirPath, EL2_USER_KEY), E_OK);

    std::filesystem::remove_all(path, errCode);
    if (std::filesystem::exists(path + "_bak", errCode)) {
        std::filesystem::rename(path + "_bak", path, errCode);
    }
#endif
}

/**
 * @tc.name: StorageDaemonTest_UintToKeyType_001
 * @tc.desc: Verify the UintToKeyType when all.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_UintToKeyType_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_EQ(storageDaemon_->UintToKeyType(static_cast<uint32_t>(EL1_SYS_KEY)), EL1_SYS_KEY);
    EXPECT_EQ(storageDaemon_->UintToKeyType(static_cast<uint32_t>(EL1_USER_KEY)), EL1_USER_KEY);
    EXPECT_EQ(storageDaemon_->UintToKeyType(static_cast<uint32_t>(EL2_USER_KEY)), EL2_USER_KEY);
    EXPECT_EQ(storageDaemon_->UintToKeyType(static_cast<uint32_t>(EL3_USER_KEY)), EL3_USER_KEY);
    EXPECT_EQ(storageDaemon_->UintToKeyType(static_cast<uint32_t>(EL4_USER_KEY)), EL4_USER_KEY);
    EXPECT_EQ(storageDaemon_->UintToKeyType(static_cast<uint32_t>(EL4_USER_KEY) + 1), EL1_SYS_KEY);
}

/**
 * @tc.name: StorageDaemonTest_IsDirPathSupport_001
 * @tc.desc: Verify the IsDirPathSupport when access failed.
 * @tc.type: FUNC
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_IsDirPathSupport_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    std::string dirPath = std::string(ANCO_DIR) + std::to_string(userId_);

    std::string path = "/data/virt_service";
    std::error_code errCode;
    if (std::filesystem::exists(path, errCode)) {
        std::filesystem::rename(path, path + "_bak");
    }

    // dirPath not exist
    EXPECT_EQ(storageDaemon_->IsDirPathSupport(dirPath), E_NON_ACCESS);

    // dirPath is not dir
    std::filesystem::create_directories(dirPath, errCode);
    std::ofstream filePath(dirPath + "/test");
    filePath.close();
    EXPECT_EQ(storageDaemon_->IsDirPathSupport(dirPath + "/test"), E_NOT_DIR_PATH);

    // dirPath is not begin with ANCO_DIR
    EXPECT_EQ(storageDaemon_->IsDirPathSupport(path), E_PARAMS_INVALID);

    std::filesystem::remove_all(path, errCode);
    if (std::filesystem::exists(path + "_bak", errCode)) {
        std::filesystem::rename(path + "_bak", path, errCode);
    }
}
} // Test
} // STORAGE_DAEMON
} // OHOS