/*
 * Copyright (C) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "key_manager.h"

#include <fcntl.h>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "base_key_mock.h"
#include "directory_ex.h"
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v2_mock.h"
#include "key_control_mock.h"
#include "fscrypt_key_v2.h"
#include "mount_manager_mock.h"
#include "storage_service_errno.h"
#include "utils/file_utils.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace {
constexpr const char *UECE_PATH = "/dev/fbex_uece";
}
 
namespace OHOS::StorageDaemon {
class KeyManagerSupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<MountManagerMoc> mountManagerMoc_ = nullptr;
    static inline shared_ptr<FscryptKeyV2Moc> fscryptKeyMock_ = nullptr;
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
    static inline shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
    static inline shared_ptr<FscryptControlMoc> fscryptControlMock_ = nullptr;
};

void KeyManagerSupTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void KeyManagerSupTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void KeyManagerSupTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
    mountManagerMoc_ = make_shared<MountManagerMoc>();
    MountManagerMoc::mountManagerMoc = mountManagerMoc_;
    fscryptKeyMock_ = make_shared<FscryptKeyV2Moc>();
    FscryptKeyV2Moc::fscryptKeyV2Moc = fscryptKeyMock_;
    fscryptControlMock_ = make_shared<FscryptControlMoc>();
    FscryptControlMoc::fscryptControlMoc = fscryptControlMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
    baseKeyMock_ = make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
}

void KeyManagerSupTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    MountManagerMoc::mountManagerMoc = nullptr;
    mountManagerMoc_ = nullptr;
    FscryptKeyV2Moc::fscryptKeyV2Moc = nullptr;
    fscryptKeyMock_ = nullptr;
    FscryptControlMoc::fscryptControlMoc = nullptr;
    fscryptControlMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
}

/**
 * @tc.name: KeyManager_GetFileEncryptStatus_000
 * @tc.desc: Verify the GetFileEncryptStatus function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_GetFileEncryptStatus_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GetFileEncryptStatus_000 Start";
    unsigned int userId = 1000;
    bool isEncrypted = true;
    EXPECT_EQ(KeyManager::GetInstance().GetFileEncryptStatus(userId, isEncrypted), E_OK);
    EXPECT_EQ(isEncrypted, true);

    string basePath = "/data/app/el2/" + to_string(userId);
    string path = basePath + "/base";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));
    EXPECT_EQ(KeyManager::GetInstance().GetFileEncryptStatus(userId, isEncrypted), E_OK);
    EXPECT_EQ(isEncrypted, false);

    EXPECT_CALL(*mountManagerMoc_, CheckMountFileByUser(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GetFileEncryptStatus(userId, isEncrypted, true), E_OK);
    EXPECT_EQ(isEncrypted, false);

    EXPECT_CALL(*mountManagerMoc_, CheckMountFileByUser(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().GetFileEncryptStatus(userId, isEncrypted, true), E_OK);
    EXPECT_EQ(isEncrypted, true);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(basePath));
    GTEST_LOG_(INFO) << "KeyManager_GetFileEncryptStatus_000 end";
}

/**
 * @tc.name: KeyManager_GenerateAppkey_001
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_GenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAppkey_001 Start";
    unsigned int user = 800;
    string keyId;

    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        EXPECT_EQ(KeyManager::GetInstance().GenerateAppkey(user, 100, keyId), -ENOTSUP);

        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAppkey(user, 100, keyId), E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(E_NOT_SUPPORT));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAppkey(user, 100, keyId), E_EL5_GENERATE_APP_KEY_ERR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAppkey(user, 100, keyId), 0);

    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_GenerateAppkey_001 end";
}

/**
 * @tc.name: KeyManager_GenerateAppkey_002
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_GenerateAppkey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAppkey_002 Start";
    unsigned int user = 300;
    string keyId;

    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }
    
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAppkey(user, 100, keyId), E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(E_NOT_SUPPORT));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAppkey(user, 100, keyId), E_EL5_GENERATE_APP_KEY_ERR);
    
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAppkey(user, 100, keyId), E_OK);
    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_GenerateAppkey_001 end";
}

/**
 * @tc.name: KeyManager_DeleteAppkey_001
 * @tc.desc: Verify the DeleteAppkey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_DeleteAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_DeleteAppkey_001 Start";
    unsigned int user = 800;
    string keyId;
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().DeleteAppkey(user, keyId), E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DeleteAppkey(_)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().DeleteAppkey(user, keyId), E_EL5_DELETE_APP_KEY_ERR);
    
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DeleteAppkey(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().DeleteAppkey(user, keyId), 0);
    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_DeleteAppkey_001 end";
}

/**
 * @tc.name: KeyManager_UpgradeKeys_001
 * @tc.desc: Verify the UpgradeKeys function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UpgradeKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpgradeKeys_001 Start";
    std::vector<FileList> dirInfo;
    FileList file1{800, "data/test"};
    FileList file2{801, "data/test"};
    dirInfo.push_back(file1);
    dirInfo.push_back(file2);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*baseKeyMock_, UpgradeKeys()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().UpgradeKeys(dirInfo), 0);
    GTEST_LOG_(INFO) << "KeyManager_UpgradeKeys_001 end";
}

/**
 * @tc.name: KeyManager_SaveUserElKey_001
 * @tc.desc: Verify the SaveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_SaveUserElKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_001 Start";
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    unsigned int user = 800;
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, tmpKey);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL1_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, tmpKey);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, tmpKey);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL3_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, tmpKey);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL4_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL5_KEY, tmpKey);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL5_KEY));

    int eL6Key = 6;
    KeyType type = static_cast<KeyType>(eL6Key);
    KeyManager::GetInstance().SaveUserElKey(user, type, tmpKey);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, type));
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_001 end";
}

/**
 * @tc.name: KeyManager_DoDeleteUserCeEceSeceKeys_001
 * @tc.desc: Verify the DoDeleteUserCeEceSeceKeys function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_DoDeleteUserCeEceSeceKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_DoDeleteUserCeEceSeceKeys_001 Start";
    unsigned int user = 800;
    std::string userDir = "/data/test/";
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserCeEceSeceKeys(user, userDir, EL1_KEY),
        E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserCeEceSeceKeys(user, userDir, EL1_KEY),
        E_CLEAR_KEY_FAILED);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserCeEceSeceKeys(user, userDir, EL1_KEY), 0);
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));

    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, tmpKey);
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserCeEceSeceKeys(user, userDir, EL1_KEY),
        E_CLEAR_KEY_FAILED);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL1_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, tmpKey);
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserCeEceSeceKeys(user, userDir, EL1_KEY), 0);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL1_KEY));
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    GTEST_LOG_(INFO) << "KeyManager_DoDeleteUserCeEceSeceKeys_001 end";
}

/**
 * @tc.name: KeyManager_DoDeleteUserKeys_001
 * @tc.desc: Verify the DoDeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_DoDeleteUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_DoDeleteUserKeys_001 Start";
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    unsigned int user = 800;
    std::string userDir = "/data/test/";
    std::map<unsigned int, std::shared_ptr<BaseKey>> userElKey;
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
            .WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID));
        EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
            .WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID));
        EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserKeys(user), E_PARAMS_NULLPTR_ERR);

        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserKeys(user), E_PARAMS_NULLPTR_ERR);

    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_DoDeleteUserKeys_001 end";
}

/**
 * @tc.name: KeyManager_DoDeleteUserKeys_002
 * @tc.desc: Verify the DoDeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_DoDeleteUserKeys_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_DoDeleteUserKeys_002 Start";
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    unsigned int user = 800;
    std::string userDir = "/data/test/";
    std::map<unsigned int, std::shared_ptr<BaseKey>> userElKey;
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserKeys(user), E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().DoDeleteUserKeys(user), 0);

    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_DoDeleteUserKeys_002 end";
}

/**
 * @tc.name: KeyManager_UnlockEceSece_001
 * @tc.desc: Verify the UnlockEceSece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UnlockEceSece_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UnlockEceSece_001 Start";
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    unsigned int user = 800;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL4_KEY);
    OHOS::ForceRemoveDirectory(keyDir);
    EXPECT_EQ(KeyManager::GetInstance().UnlockEceSece(user, token, secret), E_NON_EXIST);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UnlockEceSece(user, token, secret), E_RESTORE_KEY_FAILED);

    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, UnlockUserScreen(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UnlockEceSece(user, token, secret), E_UNLOCK_SCREEN_FAILED);

    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, UnlockUserScreen(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().UnlockEceSece(user, token, secret), E_OK);

    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, UnlockUserScreen(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UnlockEceSece(user, token, secret), E_UNLOCK_SCREEN_FAILED);
    OHOS::ForceRemoveDirectory(keyDir);
    GTEST_LOG_(INFO) << "KeyManager_UnlockEceSece_001 end";
}

/**
 * @tc.name: KeyManager_UpdateUseAuthWithRecoveryKey_001
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UpdateUseAuthWithRecoveryKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_001 Start";
    unsigned int userId = 888;
    std::vector<uint8_t> authToken;
    std::vector<uint8_t> newSecret;
    uint64_t secureUid = 1;
    std::vector<std::vector<uint8_t>> plainText;
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    OHOS::ForceRemoveDirectory(el2Path);
    EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
        authToken, newSecret, secureUid, userId, plainText), E_KEY_TYPE_INVALID);

    OHOS::ForceCreateDirectory(el2Path);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
        authToken, newSecret, secureUid, userId, plainText), E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
        authToken, newSecret, secureUid, userId, plainText), E_PARAMS_INVALID);
    
    std::vector<std::vector<uint8_t>> plainText2(3);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(-1));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(-1));
    #endif
    EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
        authToken, newSecret, secureUid, userId, plainText2), E_ELX_KEY_STORE_ERROR);
    OHOS::ForceRemoveDirectory(el2Path);
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_001 end";
}

/**
 * @tc.name: KeyManager_UpdateUseAuthWithRecoveryKey_002
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UpdateUseAuthWithRecoveryKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_002 Start";
    unsigned int userId = 888;
    std::vector<uint8_t> authToken;
    std::vector<uint8_t> newSecret;
    uint64_t secureUid = 1;
    std::vector<std::vector<uint8_t>> plainText(3);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);

    OHOS::ForceCreateDirectory(el2Path);
    OHOS::ForceCreateDirectory(el3Path);
    OHOS::ForceCreateDirectory(el4Path);
    
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
            .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
        EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
            .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
        #ifdef USER_CRYPTO_MIGRATE_KEY
        EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
        #else
        EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
        #endif

        EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
            authToken, newSecret, secureUid, userId, plainText), E_OK);

        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }
    
    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    OHOS::ForceRemoveDirectory(el2Path);
    OHOS::ForceRemoveDirectory(el3Path);
    OHOS::ForceRemoveDirectory(el4Path);
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_002 end";
}

/**
 * @tc.name: KeyManager_UpdateUseAuthWithRecoveryKey_003
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UpdateUseAuthWithRecoveryKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_003 Start";
    unsigned int userId = 888;
    std::vector<uint8_t> authToken;
    std::vector<uint8_t> newSecret;
    uint64_t secureUid = 1;
    std::vector<std::vector<uint8_t>> plainText(3);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);

    OHOS::ForceCreateDirectory(el2Path);
    OHOS::ForceCreateDirectory(el3Path);
    OHOS::ForceCreateDirectory(el4Path);
    
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
        authToken, newSecret, secureUid, userId, plainText), E_EL5_ENCRYPT_CLASS_ERROR);
    
    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    OHOS::ForceRemoveDirectory(el2Path);
    OHOS::ForceRemoveDirectory(el3Path);
    OHOS::ForceRemoveDirectory(el4Path);
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_003 end";
}

/**
 * @tc.name: KeyManager_UpdateUseAuthWithRecoveryKey_004
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UpdateUseAuthWithRecoveryKey_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_004 Start";
    unsigned int userId = 888;
    std::vector<uint8_t> authToken;
    std::vector<uint8_t> newSecret;
    uint64_t secureUid = 1;
    std::vector<std::vector<uint8_t>> plainText(3);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);

    OHOS::ForceCreateDirectory(el2Path);
    OHOS::ForceCreateDirectory(el3Path);
    OHOS::ForceCreateDirectory(el4Path);
    
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, LockUece(_)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
        authToken, newSecret, secureUid, userId, plainText), E_OK);

    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    OHOS::ForceRemoveDirectory(el2Path);
    OHOS::ForceRemoveDirectory(el3Path);
    OHOS::ForceRemoveDirectory(el4Path);
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_004 end";
}

/**
 * @tc.name: KeyManager_UpdateUseAuthWithRecoveryKey_005
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UpdateUseAuthWithRecoveryKey_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_005 Start";
    unsigned int userId = 888;
    std::vector<uint8_t> authToken;
    std::vector<uint8_t> newSecret;
    uint64_t secureUid = 1;
    std::vector<std::vector<uint8_t>> plainText(3);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);

    OHOS::ForceCreateDirectory(el2Path);
    OHOS::ForceCreateDirectory(el3Path);
    OHOS::ForceCreateDirectory(el4Path);
    
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, LockUece(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(
        authToken, newSecret, secureUid, userId, plainText), E_OK);

    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    OHOS::ForceRemoveDirectory(el2Path);
    OHOS::ForceRemoveDirectory(el3Path);
    OHOS::ForceRemoveDirectory(el4Path);
    GTEST_LOG_(INFO) << "KeyManager_UpdateUseAuthWithRecoveryKey_005 end";
}

/**
 * @tc.name: KeyManager_HashElxActived_001
 * @tc.desc: Verify the HashElxActived function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_HashElxActived_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_HashElxActived_001 Start";
    unsigned int user = 800;
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    EXPECT_FALSE(KeyManager::GetInstance().HashElxActived(user, EL1_KEY));

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, tmpKey);
    EXPECT_CALL(*baseKeyMock_, KeyDescIsEmpty()).WillOnce(Return(false));
    EXPECT_TRUE(KeyManager::GetInstance().HashElxActived(user, EL1_KEY));
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);

    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    EXPECT_FALSE(KeyManager::GetInstance().HashElxActived(user, EL2_KEY));
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, tmpKey);
    EXPECT_CALL(*baseKeyMock_, KeyDescIsEmpty()).WillOnce(Return(false));
    EXPECT_TRUE(KeyManager::GetInstance().HashElxActived(user, EL2_KEY));
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    GTEST_LOG_(INFO) << "KeyManager_HashElxActived_001 end";
}

/**
 * @tc.name: KeyManager_HashElxActived_002
 * @tc.desc: Verify the HashElxActived function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_HashElxActived_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_HashElxActived_002 Start";
    unsigned int user = 800;
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    EXPECT_FALSE(KeyManager::GetInstance().HashElxActived(user, EL3_KEY));

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, tmpKey);
    EXPECT_CALL(*baseKeyMock_, KeyDescIsEmpty()).WillOnce(Return(false));
    EXPECT_TRUE(KeyManager::GetInstance().HashElxActived(user, EL3_KEY));
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);

    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    EXPECT_FALSE(KeyManager::GetInstance().HashElxActived(user, EL4_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, tmpKey);
    EXPECT_CALL(*baseKeyMock_, KeyDescIsEmpty()).WillOnce(Return(false));
    EXPECT_TRUE(KeyManager::GetInstance().HashElxActived(user, EL4_KEY));
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    GTEST_LOG_(INFO) << "KeyManager_HashElxActived_002 end";
}

/**
 * @tc.name: KeyManager_HashElxActived_003
 * @tc.desc: Verify the HashElxActived function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_HashElxActived_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_HashElxActived_003 Start";
    unsigned int user = 800;
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    EXPECT_FALSE(KeyManager::GetInstance().HashElxActived(user, EL5_KEY));

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(user, EL5_KEY, tmpKey);
    EXPECT_CALL(*baseKeyMock_, KeyDescIsEmpty()).WillOnce(Return(false));
    EXPECT_TRUE(KeyManager::GetInstance().HashElxActived(user, EL5_KEY));
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    
    int eL6Key = 6;
    KeyType type = static_cast<KeyType>(eL6Key);
    EXPECT_FALSE(KeyManager::GetInstance().HashElxActived(user, type));
    GTEST_LOG_(INFO) << "KeyManager_HashElxActived_003 end";
}
}
