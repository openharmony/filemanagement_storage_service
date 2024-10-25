/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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
#include "el5_filekey_manager_kit_mock.h"
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
    static inline shared_ptr<El5FilekeyManagerKitMoc> el5FilekeyManagerKitMoc_ = nullptr;
};

void KeyManagerSupTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
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
    el5FilekeyManagerKitMoc_ = make_shared<El5FilekeyManagerKitMoc>();
    El5FilekeyManagerKitMoc::el5FilekeyManagerKitMoc = el5FilekeyManagerKitMoc_;
}

void KeyManagerSupTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
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
    El5FilekeyManagerKitMoc::el5FilekeyManagerKitMoc = nullptr;
    el5FilekeyManagerKitMoc_ = nullptr;
}

void KeyManagerSupTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
}

void KeyManagerSupTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
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
    EXPECT_EQ(KeyManager::GetInstance()->GetFileEncryptStatus(userId, isEncrypted), E_OK);
    EXPECT_EQ(isEncrypted, true);

    string basePath = "/data/app/el2/" + to_string(userId);
    string path = basePath + "/base";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));
    EXPECT_EQ(KeyManager::GetInstance()->GetFileEncryptStatus(userId, isEncrypted), E_OK);
    EXPECT_EQ(isEncrypted, false);

    EXPECT_CALL(*mountManagerMoc_, CheckMountFileByUser(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->GetFileEncryptStatus(userId, isEncrypted, true), E_OK);
    EXPECT_EQ(isEncrypted, false);

    EXPECT_CALL(*mountManagerMoc_, CheckMountFileByUser(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->GetFileEncryptStatus(userId, isEncrypted, true), E_OK);
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
        EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), -ENOTSUP);

        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->userEl4Key_.erase(user);

    string basePath = "/data/app/el2/" + to_string(user);
    string path = basePath + "/base";
    OHOS::ForceRemoveDirectory(basePath);
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), -ENOENT);

    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL4_KEY);
    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), -EFAULT);
    
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), 0);

    KeyManager::GetInstance()->userEl4Key_.erase(user);
    KeyManager::GetInstance()->userEl4Key_[user] = nullptr;
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), -ENOENT);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(basePath));
    ASSERT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
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

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->userEl4Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->DeleteAppkey(user, keyId), -ENOENT);

    string basePath = "/data/app/el2/" + to_string(user);
    string path = basePath + "/base";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL4_KEY);
    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DeleteAppkey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->DeleteAppkey(user, keyId), -EFAULT);

    EXPECT_CALL(*fscryptKeyMock_, DeleteAppkey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->DeleteAppkey(user, keyId), 0);

    KeyManager::GetInstance()->userEl4Key_.erase(user);
    KeyManager::GetInstance()->userEl4Key_[user] = nullptr;
    EXPECT_EQ(KeyManager::GetInstance()->DeleteAppkey(user, keyId), -ENOENT);
    KeyManager::GetInstance()->userEl4Key_.erase(user);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(basePath));
    ASSERT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
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
    EXPECT_EQ(KeyManager::GetInstance()->UpgradeKeys(dirInfo), 0);
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
    KeyManager::GetInstance()->userEl1Key_.erase(user);
    KeyManager::GetInstance()->userEl2Key_.erase(user);
    KeyManager::GetInstance()->userEl3Key_.erase(user);
    KeyManager::GetInstance()->userEl4Key_.erase(user);
    KeyManager::GetInstance()->userEl5Key_.erase(user);
    KeyManager::GetInstance()->SaveUserElKey(user, EL1_KEY, tmpKey);
    EXPECT_NE(KeyManager::GetInstance()->userEl1Key_[user], nullptr);

    KeyManager::GetInstance()->SaveUserElKey(user, EL2_KEY, tmpKey);
    EXPECT_NE(KeyManager::GetInstance()->userEl2Key_[user], nullptr);

    KeyManager::GetInstance()->SaveUserElKey(user, EL3_KEY, tmpKey);
    EXPECT_NE(KeyManager::GetInstance()->userEl3Key_[user], nullptr);

    KeyManager::GetInstance()->SaveUserElKey(user, EL4_KEY, tmpKey);
    EXPECT_NE(KeyManager::GetInstance()->userEl4Key_[user], nullptr);

    KeyManager::GetInstance()->SaveUserElKey(user, EL5_KEY, tmpKey);
    EXPECT_NE(KeyManager::GetInstance()->userEl5Key_[user], nullptr);

    int eL6Key = 6;
    KeyType type = static_cast<KeyType>(eL6Key);
    KeyManager::GetInstance()->SaveUserElKey(user, type, tmpKey);
    EXPECT_NE(KeyManager::GetInstance()->userEl5Key_[user], nullptr);
    KeyManager::GetInstance()->userEl1Key_.erase(user);
    KeyManager::GetInstance()->userEl2Key_.erase(user);
    KeyManager::GetInstance()->userEl3Key_.erase(user);
    KeyManager::GetInstance()->userEl4Key_.erase(user);
    KeyManager::GetInstance()->userEl5Key_.erase(user);
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
    std::map<unsigned int, std::shared_ptr<BaseKey>> userElKey;

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->DoDeleteUserCeEceSeceKeys(user, userDir, userElKey),
        -ENOMEM);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->DoDeleteUserCeEceSeceKeys(user, userDir, userElKey),
        -E_CLEAR_KEY_FAILED);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->DoDeleteUserCeEceSeceKeys(user, userDir, userElKey), 0);
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    userElKey.insert(make_pair(user, tmpKey));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->DoDeleteUserCeEceSeceKeys(user, userDir, userElKey),
        -E_CLEAR_KEY_FAILED);
    EXPECT_EQ(userElKey.find(user), userElKey.end());

    userElKey.insert(make_pair(user, tmpKey));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->DoDeleteUserCeEceSeceKeys(user, userDir, userElKey), 0);
    EXPECT_EQ(userElKey.find(user), userElKey.end());
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
        EXPECT_EQ(KeyManager::GetInstance()->DoDeleteUserKeys(user), -ENOMEM);

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
    EXPECT_EQ(KeyManager::GetInstance()->DoDeleteUserKeys(user), -ENOMEM);

    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_DoDeleteUserKeys_001 end";
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
    KeyManager::GetInstance()->userEl4Key_.erase(user);
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL4_KEY);
    OHOS::ForceRemoveDirectory(keyDir);
    EXPECT_EQ(KeyManager::GetInstance()->UnlockEceSece(user, token, secret), E_NON_EXIST);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockEceSece(user, token, secret), E_RESTORE_KEY_FAILED);

    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, UnlockUserScreen(_, _, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockEceSece(user, token, secret), E_UNLOCK_SCREEN_FAILED);

    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, UnlockUserScreen(_, _, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockEceSece(user, token, secret), E_OK);

    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, UnlockUserScreen(_, _, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockEceSece(user, token, secret), E_UNLOCK_SCREEN_FAILED);
    OHOS::ForceRemoveDirectory(keyDir);
    GTEST_LOG_(INFO) << "KeyManager_UnlockEceSece_001 end";
}

#ifdef EL5_FILEKEY_MANAGER
/**
 * @tc.name: KeyManager_GenerateAndLoadAppKeyInfo_001
 * @tc.desc: Verify the GenerateAndLoadAppKeyInfo function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_GenerateAndLoadAppKeyInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndLoadAppKeyInfo_001 Start";
    uint32_t userId = 800;
    std::vector<std::pair<int, std::string>> keyInfo;
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndLoadAppKeyInfo(userId, keyInfo), E_OK);

    keyInfo.push_back(make_pair(1, "test"));
    keyInfo.push_back(make_pair(2, "test2"));
    keyInfo.push_back(make_pair(3, "test3"));
    KeyManager::GetInstance()->userEl5Key_.erase(userId);
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndLoadAppKeyInfo(userId, keyInfo), -ENOENT);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance()->userEl5Key_[userId] = tmpKey;
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(false)).WillOnce(Return(true))
        .WillOnce(DoAll(SetArgReferee<2>("test3"), Return(true)));
    EXPECT_CALL(*el5FilekeyManagerKitMoc_, ChangeUserAppkeysLoadInfo(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndLoadAppKeyInfo(userId, keyInfo), -EFAULT);

    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(false)).WillOnce(Return(true))
        .WillOnce(DoAll(SetArgReferee<2>("test3"), Return(true)));
    EXPECT_CALL(*el5FilekeyManagerKitMoc_, ChangeUserAppkeysLoadInfo(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndLoadAppKeyInfo(userId, keyInfo), E_OK);
    KeyManager::GetInstance()->userEl5Key_.erase(userId);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndLoadAppKeyInfo_001 end";
}
#endif

/**
 * @tc.name: KeyManager_UnlockUserAppKeys_001
 * @tc.desc: Verify the UnlockUserAppKeys function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UnlockUserAppKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserAppKeys_001 Start";
    bool existUece = true;
    bool needGetAllAppKey = true;
    uint32_t userId = 800;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), E_OK);

        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }

    std::vector<std::pair<int, std::string>> keyInfo;
    keyInfo.push_back(make_pair(1, "test"));
    keyInfo.push_back(make_pair(2, "test2"));
    keyInfo.push_back(make_pair(3, "test3"));

    #ifdef EL5_FILEKEY_MANAGER
    EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAllAppKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), -EFAULT);

    EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAllAppKey(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(keyInfo), Return(0)));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), -ENOENT);

    EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAllAppKey(_, _)).WillOnce(Return(0));
    #endif
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), E_OK);
    KeyManager::GetInstance()->userEl5Key_.erase(userId);
    needGetAllAppKey = false;
    #ifdef EL5_FILEKEY_MANAGER
    EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAppKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), -EFAULT);

    EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAppKey(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(keyInfo), Return(0)));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), -ENOENT);

    EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAppKey(_, _)).WillOnce(Return(0));
    #endif
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), E_OK);
    KeyManager::GetInstance()->userEl5Key_.erase(userId);
    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_UnlockUece_001 end";
}

/**
 * @tc.name: KeyManager_UnlockUece_001
 * @tc.desc: Verify the UnlockUece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerSupTest, KeyManager_UnlockUece_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UnlockUece_001 Start";
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    unsigned int user = 800;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL5_KEY);
    OHOS::ForceCreateDirectory(keyDir);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUece(user, token, secret), E_UNLOCK_APP_KEY2_FAILED);

    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(true));
    if (access(UECE_PATH, F_OK) == 0) {
        #ifdef EL5_FILEKEY_MANAGER
        std::vector<std::pair<int, std::string>> keyInfo;
        keyInfo.push_back(make_pair(1, "test"));
        keyInfo.push_back(make_pair(2, "test2"));
        keyInfo.push_back(make_pair(3, "test3"));

        EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAppKey(_, _)).WillOnce(Return(-1));
        EXPECT_EQ(KeyManager::GetInstance()->UnlockUece(user, token, secret), E_UNLOCK_APP_KEY2_FAILED);
        EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(true));
        EXPECT_CALL(*el5FilekeyManagerKitMoc_, GetUserAppKey(_, _)).WillOnce(Return(0));
        #endif
    }

    EXPECT_EQ(KeyManager::GetInstance()->UnlockUece(user, token, secret), E_OK);
    GTEST_LOG_(INFO) << "KeyManager_UnlockUece_001 end";
}
}

