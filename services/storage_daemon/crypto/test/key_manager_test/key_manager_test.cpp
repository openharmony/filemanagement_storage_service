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
#include "file_ex.h"
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v2_mock.h"
#include "fscrypt_key_v2.h"
#include "key_control_mock.h"
#include "storage_service_constants.h"
#include "storage_service_errno.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "mock/uece_activation_callback_mock.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace {
constexpr const char *UECE_PATH = "/dev/fbex_uece";
}

namespace OHOS::StorageDaemon {
class KeyManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FscryptControlMoc> fscryptControlMock_ = nullptr;
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
    static inline shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
    static inline shared_ptr<FscryptKeyV2Moc> fscryptKeyMock_ = nullptr;
};

void KeyManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void KeyManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void KeyManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
    fscryptControlMock_ = make_shared<FscryptControlMoc>();
    FscryptControlMoc::fscryptControlMoc = fscryptControlMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
    baseKeyMock_ = make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
    fscryptKeyMock_ = make_shared<FscryptKeyV2Moc>();
    FscryptKeyV2Moc::fscryptKeyV2Moc = fscryptKeyMock_;
}

void KeyManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    FscryptControlMoc::fscryptControlMoc = nullptr;
    fscryptControlMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
    FscryptKeyV2Moc::fscryptKeyV2Moc = nullptr;
    fscryptKeyMock_ = nullptr;
}

/**
 * @tc.name: KeyManager_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SetDirEncryptionPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetDirEncryptionPolicy_001 Start";
    std::string dirPath = "/data/service/test";
    uint32_t userId = 100;
    StorageService::EncryptionLevel type = StorageService::EL2_USER_KEY;

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, type), E_NOT_SUPPORT);
    unsigned int user = 100;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");

    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, elKey);
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    type = StorageService::EL1_USER_KEY;
    EXPECT_EQ(KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, type), -ENOENT);

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(-1));
    type = StorageService::EL2_USER_KEY;
    EXPECT_EQ(KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, type), -1);

    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(-1));
    type = StorageService::EL3_USER_KEY;
    EXPECT_EQ(KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, type), -1);

    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    type = StorageService::EL3_USER_KEY;
    EXPECT_EQ(KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, type), -2);


    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(-1));
    type = StorageService::EL4_USER_KEY;
    EXPECT_EQ(KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, type), -1);

    GTEST_LOG_(INFO) << "KeyManager_SetDirEncryptionPolicy_001 end";
}

/**
 * @tc.name: KeyManager_GetBaseKey_001
 * @tc.desc: Verify the GetBaseKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GetBaseKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GetBaseKey_001 Start";
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GetBaseKey("/data/test"), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance().GetBaseKey("/data/test"), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V1));
    EXPECT_NE(KeyManager::GetInstance().GetBaseKey("/data/test"), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V1));
    EXPECT_NE(KeyManager::GetInstance().GetBaseKey("/data/test"), nullptr);
    GTEST_LOG_(INFO) << "KeyManager_GetBaseKey_001 end";
}

/**
 * @tc.name: KeyManager_GenerateAndInstallDeviceKey_001
 * @tc.desc: Verify the GenerateAndInstallDeviceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GenerateAndInstallDeviceKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallDeviceKey_001 Start";
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallDeviceKey("/data/test"), E_GLOBAL_KEY_NULLPTR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallDeviceKey("/data/test"), E_GLOBAL_KEY_INIT_ERROR);
    EXPECT_EQ(KeyManager::GetInstance().globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(-1));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(-1));
    #endif
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallDeviceKey("/data/test"), E_GLOBAL_KEY_STORE_ERROR);
    EXPECT_EQ(KeyManager::GetInstance().globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallDeviceKey("/data/test"), E_GLOBAL_KEY_ACTIVE_ERROR);
    EXPECT_EQ(KeyManager::GetInstance().globalEl1Key_, nullptr);
    GTEST_LOG_(INFO) << "KeyManager_GetBaseKey_001 end";
}

/**
 * @tc.name: KeyManager_GenerateAndInstallDeviceKey_002
 * @tc.desc: Verify the GenerateAndInstallDeviceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GenerateAndInstallDeviceKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallDeviceKey_002 Start";
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif

    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallDeviceKey("/data/test"), 0);
    EXPECT_EQ(KeyManager::GetInstance().hasGlobalDeviceKey_, true);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallDeviceKey("/data/test"), E_OK);

    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallDeviceKey_002 end";
}

/**
 * @tc.name: KeyManager_RestoreDeviceKey_001
 * @tc.desc: Verify the RestoreDeviceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_RestoreDeviceKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_RestoreDeviceKey_001 Start";
    KeyManager::GetInstance().globalEl1Key_ = nullptr;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().RestoreDeviceKey("/data/test"), E_GLOBAL_KEY_NULLPTR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().RestoreDeviceKey("/data/test"), E_GLOBAL_KEY_INIT_ERROR);
    EXPECT_EQ(KeyManager::GetInstance().globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().RestoreDeviceKey("/data/test"), E_GLOBAL_KEY_STORE_ERROR);
    EXPECT_EQ(KeyManager::GetInstance().globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance().RestoreDeviceKey("/data/test"), E_GLOBAL_KEY_ACTIVE_ERROR);
    EXPECT_EQ(KeyManager::GetInstance().globalEl1Key_, nullptr);
    GTEST_LOG_(INFO) << "KeyManager_RestoreDeviceKey_001 end";
}

/**
 * @tc.name: KeyManager_RestoreDeviceKey_002
 * @tc.desc: Verify the RestoreDeviceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_RestoreDeviceKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_RestoreDeviceKey_002 Start";
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().RestoreDeviceKey("/data/test"), 0);
    EXPECT_NE(KeyManager::GetInstance().globalEl1Key_, nullptr);

    EXPECT_EQ(KeyManager::GetInstance().RestoreDeviceKey("/data/test"), 0);
    EXPECT_NE(KeyManager::GetInstance().globalEl1Key_, nullptr);
    KeyManager::GetInstance().globalEl1Key_ = nullptr;
    GTEST_LOG_(INFO) << "KeyManager_RestoreDeviceKey_002 end";
}

/**
 * @tc.name: KeyManager_ActiveCeSceSeceUserKey_001
 * @tc.desc: Verify the ActiveCeSceSeceUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_ActiveCeSceSeceUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveCeSceSeceUserKey_001 Start";
    unsigned int user = 888;
    std::vector<uint8_t> token = {};
    std::vector<uint8_t> secret = {};
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), 0);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, tmpKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, KeyDescIsEmpty()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL2_KEY, token, secret), -ENOENT);
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, tmpKey);
    int eL6Key = 6;
    KeyType type = static_cast<KeyType>(eL6Key);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, type, token, secret), E_KEY_TYPE_INVALID);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), -ENOENT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), -ENOENT);

    std::string keyDir = std::string(USER_EL1_DIR) + "/" + std::to_string(user);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), -EOPNOTSUPP);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), E_ELX_KEY_ACTIVE_ERROR);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    GTEST_LOG_(INFO) << "KeyManager_ActiveCeSceSeceUserKey_001 end";
}

/**
 * @tc.name: KeyManager_ActiveCeSceSeceUserKey_002
 * @tc.desc: Verify the ActiveCeSceSeceUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_ActiveCeSceSeceUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveCeSceSeceUserKey_002 Start";
    unsigned int user = 888;
    std::vector<uint8_t> token = {};
    std::vector<uint8_t> secret = {};

    std::string keyUeceDir = std::string(UECE_DIR) + "/" + std::to_string(user);
    std::string keyDir = std::string(USER_EL5_DIR) + "/" + std::to_string(user);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyUeceDir));
    std::ofstream file(keyDir + "/test.txt");
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), E_ELX_KEY_ACTIVE_ERROR);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL5_KEY));

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(E_OK));
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), 0);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL5_KEY));

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, KeyDescIsEmpty()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), E_ACTIVE_REPEATED);
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    OHOS::RemoveFile(keyDir + "/test.txt");
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyUeceDir));
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    GTEST_LOG_(INFO) << "KeyManager_ActiveCeSceSeceUserKey_002 end";
}

/**
 * @tc.name: KeyManager_CheckAndDeleteEmptyEl5Directory
 * @tc.desc: Verify the CheckAndDeleteEmptyEl5Directory function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_CheckAndDeleteEmptyEl5Directory, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_CheckAndDeleteEmptyEl5Directory Start";
    unsigned int user = 1;
    std::string keyDir = std::string(USER_EL1_DIR) + "/" + std::to_string(user);
    std::string keyUeceDir = std::string(UECE_DIR) + "/" + std::to_string(user);

    EXPECT_EQ(KeyManager::GetInstance().CheckAndDeleteEmptyEl5Directory(keyDir, user), -ENOENT);

    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyUeceDir));
    EXPECT_EQ(KeyManager::GetInstance().CheckAndDeleteEmptyEl5Directory(keyDir, user), -ENOENT);
    OHOS::ForceRemoveDirectory(keyDir);
    OHOS::ForceRemoveDirectory(keyUeceDir);

    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyUeceDir));
    std::ofstream file(keyDir + "/test.txt");
    EXPECT_EQ(KeyManager::GetInstance().CheckAndDeleteEmptyEl5Directory(keyDir, user), 0);
    OHOS::RemoveFile(keyDir + "/test.txt");
    OHOS::ForceRemoveDirectory(keyDir);
    OHOS::ForceRemoveDirectory(keyUeceDir);
    GTEST_LOG_(INFO) << "KeyManager_CheckAndDeleteEmptyEl5Directory end";
}

/**
 * @tc.name: KeyManager_ActiveElXUserKey
 * @tc.desc: Verify the ActiveElXUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_ActiveElXUserKey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey Start";
    unsigned int user = 1;
    const std::vector<uint8_t> token = {};
    const std::vector<uint8_t> secret = {};
    std::shared_ptr<BaseKey> elKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_ELX_KEY_INIT_ERROR);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_RESTORE_KEY_FAILED);
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey end";
}

/**
 * @tc.name: KeyManager_ActiveElXUserKey_002
 * @tc.desc: Verify the ActiveElXUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_ActiveElXUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey_002 Start";
    unsigned int user = 888;
    const std::vector<uint8_t> token = {};
    const std::vector<uint8_t> secret = {};
    std::shared_ptr<BaseKey> elKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));

    std::string latestPath = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY) +
        PATH_LATEST;
    OHOS::ForceRemoveDirectory(latestPath);
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(-1));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(-1));
    #endif
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_ELX_KEY_STORE_ERROR);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_ELX_KEY_ACTIVE_ERROR);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_OK);
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey_002 end";
}

/**
 * @tc.name: KeyManager_ActiveElXUserKey_003
 * @tc.desc: Verify the ActiveElXUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_ActiveElXUserKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey_003 Start";
    unsigned int user = 888;
    const std::vector<uint8_t> token = {};
    const std::vector<uint8_t> secret = {};
    std::shared_ptr<BaseKey> elKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));

    std::string userPath = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY);
    std::string latestPath = userPath + PATH_LATEST;
    OHOS::ForceRemoveDirectory(latestPath);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(latestPath));
    std::string fileUpdate = latestPath + SUFFIX_NEED_UPDATE;
    auto fd = open(fileUpdate.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);
    close(fd);
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_OK);

    std::string fileRestore = latestPath + SUFFIX_NEED_RESTORE;
    fd = open(fileRestore.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);
    close(fd);
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_OK);

    ASSERT_EQ(remove(fileRestore.c_str()), 0);
    ASSERT_EQ(remove(fileUpdate.c_str()), 0);
    OHOS::ForceRemoveDirectory(userPath);
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey_003 end";
}

/**
 * @tc.name: KeyManager_UnlockUserScreen
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_UnlockUserScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserScreen Start";
    unsigned int user = 1;
    const std::vector<uint8_t> token = {};
    const std::vector<uint8_t> secret = {};
    EXPECT_EQ(KeyManager::GetInstance().UnlockUserScreen(user, token, secret), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().UnlockUserScreen(100, token, secret), 0);
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserScreen end";
}

/**
 * @tc.name: KeyManager_UnlockUserScreen_NotCancelDelayTask
 * @tc.desc: Verify the UnlockUserScreen_NotCancelDelayTask function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_UnlockUserScreen_NotCancelDelayTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserScreen_NotCancelDelayTask Start";
    unsigned int user = 1;
    const std::vector<uint8_t> token = {};
    const std::vector<uint8_t> secret = {};
    KeyManager::GetInstance().userLockScreenTask_[user] = nullptr;
    EXPECT_EQ(KeyManager::GetInstance().UnlockUserScreen(user, token, secret), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().UnlockUserScreen(100, token, secret), 0);
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserScreen end";
}

/**
 * @tc.name: KeyManager_SetDirectoryElPolicy
 * @tc.desc: Verify the SetDirectoryElPolicy function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SetDirectoryElPolicy, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy Start";
    unsigned int user = 1;
    KeyType type = EL1_KEY;
    std::vector<FileList> vec = {};

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), -ENOENT);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, tmpKey);
    vec.push_back({1, "/test"});
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(-EINVAL));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), E_LOAD_AND_SET_POLICY_ERR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), 0);

    int eL6Key = 6;
    type = static_cast<KeyType>(eL6Key);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), 0);

    type = EL3_KEY;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), -ENOENT);

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, tmpKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, tmpKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(-EINVAL));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), E_LOAD_AND_SET_ECE_POLICY_ERR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, type, vec), 0);
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy end";
}

/**
 * @tc.name: KeyManager_IsUeceSupport
 * @tc.desc: Verify the IsUeceSupport function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_IsUeceSupport, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_IsUeceSupport Start";
    EXPECT_TRUE(OHOS::RemoveFile(UECE_PATH));
    EXPECT_FALSE(KeyManager::GetInstance().IsUeceSupport());

    EXPECT_TRUE(OHOS::ForceCreateDirectory(UECE_PATH));
    EXPECT_FALSE(KeyManager::GetInstance().IsUeceSupport());
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(UECE_PATH));

    std::ofstream file(UECE_PATH);
    EXPECT_TRUE(KeyManager::GetInstance().IsUeceSupport());
    EXPECT_TRUE(OHOS::RemoveFile(UECE_PATH));
    GTEST_LOG_(INFO) << "KeyManager_IsUeceSupport end";
}

/**
 * @tc.name: KeyManager_RestoreUserKey_000
 * @tc.desc: Verify the RestoreUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_RestoreUserKey_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_000 Start";
    uint32_t userId = 1;
    KeyType type = EL1_KEY;

    #ifdef USER_CRYPTO_MIGRATE_KEY
    int eL6Key = 6;
    type = static_cast<KeyType>(eL6Key);
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(userId, type), E_PARAMS_INVALID);

    type = EL1_KEY;
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(userId, type), -ENOENT);

    std::string keyDir = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(userId, type), 0);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    #endif
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_000 end";
}

/**
 * @tc.name: KeyManager_ClearAppCloneUserNeedRestore_000
 * @tc.desc: Verify the ClearAppCloneUserNeedRestore function.
 * @tc.type: FUNC
 * @tc.require: IBCTXL
 */
#ifdef USER_CRYPTO_MIGRATE_KEY
HWTEST_F(KeyManagerTest, KeyManager_ClearAppCloneUserNeedRestore_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ClearAppCloneUserNeedRestore_000 Start";
    uint32_t userId = 228;
    const string USER_EL1_DIR = "data/test/user/el1";
    std::string keyEl1Dir = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyEl1Dir));
    std::ofstream el1_need_restore(keyEl1Dir + RESTORE_DIR);
    if (el1_need_restore.is_open()) {
        el1_need_restore << "1";
        el1_need_restore.close();
    }
    std::string elNeedRestorePath = keyEl1Dir + RESTORE_DIR;
    EXPECT_EQ(KeyManager::GetInstance().ClearAppCloneUserNeedRestore(userId, elNeedRestorePath), E_OK);
    GTEST_LOG_(INFO) << "KeyManager_ClearAppCloneUserNeedRestore_000 end";
}
#endif

/**
 * @tc.name: KeyManager_GenerateAndInstallUserKey_001
 * @tc.desc: Verify the GenerateAndInstallUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GenerateAndInstallUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallUserKey_001 Start";
    UserAuth auth;
    KeyManager::GetInstance().DeleteElKey(800, EL1_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY),
        E_GLOBAL_KEY_NULLPTR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY),
        E_ELX_KEY_INIT_ERROR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(-1));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(-1));
    #endif
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY),
        E_ELX_KEY_STORE_ERROR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY),
        E_ELX_KEY_ACTIVE_ERROR);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallUserKey_001 end";
}

/**
 * @tc.name: KeyManager_GenerateAndInstallUserKey_002
 * @tc.desc: Verify the GenerateAndInstallUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GenerateAndInstallUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallUserKey_002 Start";
    UserAuth auth;
    KeyManager::GetInstance().DeleteElKey(800, EL1_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL1_KEY), true);
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL1_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL1_KEY);

    KeyManager::GetInstance().DeleteElKey(800, EL2_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL2_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL2_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL2_KEY);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallUserKey_002 end";
}

/**
 * @tc.name: KeyManager_GenerateAndInstallUserKey_003
 * @tc.desc: Verify the GenerateAndInstallUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GenerateAndInstallUserKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallUserKey_003 Start";
    UserAuth auth;
    KeyManager::GetInstance().DeleteElKey(800, EL3_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL3_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL3_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL3_KEY);

    KeyManager::GetInstance().DeleteElKey(800, EL4_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().GenerateAndInstallUserKey(800, "/data/test", auth, EL4_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL4_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL4_KEY);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallUserKey_003 end";
}

/**
 * @tc.name: KeyManager_RestoreUserKey_001
 * @tc.desc: Verify the RestoreUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_RestoreUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_001 Start";
    UserAuth auth;
    KeyManager::GetInstance().DeleteElKey(800, EL1_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL1_KEY), E_GLOBAL_KEY_NULLPTR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL1_KEY), E_ELX_KEY_INIT_ERROR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL1_KEY), E_ELX_KEY_STORE_ERROR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL1_KEY), E_ELX_KEY_ACTIVE_ERROR);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallUserKey_001 end";
}

/**
 * @tc.name: KeyManager_RestoreUserKey_002
 * @tc.desc: Verify the RestoreUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_RestoreUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_002 Start";
    UserAuth auth;
    KeyManager::GetInstance().DeleteElKey(800, EL1_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL1_KEY), true);
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL1_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL1_KEY);

    KeyManager::GetInstance().DeleteElKey(800, EL2_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL2_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL2_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL2_KEY);

    KeyManager::GetInstance().DeleteElKey(800, EL3_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL3_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL3_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL3_KEY);
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_002 end";
}

/**
 * @tc.name: KeyManager_RestoreUserKey_003
 * @tc.desc: Verify the RestoreUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_RestoreUserKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_003 Start";
    UserAuth auth;
    KeyManager::GetInstance().DeleteElKey(800, EL4_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL4_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL4_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL4_KEY);

    KeyManager::GetInstance().DeleteElKey(800, EL5_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().RestoreUserKey(800, "/data/test", auth, EL5_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance().HasElkey(800, EL5_KEY), true);
    KeyManager::GetInstance().DeleteElKey(800, EL5_KEY);
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_003 end";
}

/**
 * @tc.name: KeyManager_GetKeyDirByUserAndType_001
 * @tc.desc: Verify the GetKeyDirByUserAndType function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GetKeyDirByUserAndType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GetKeyDirByUserAndType_001 Start";
    unsigned int user = 800;
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY),
        std::string(USER_EL1_DIR) + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL2_KEY),
        std::string(USER_EL2_DIR) + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL3_KEY),
        std::string(USER_EL3_DIR) + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL4_KEY),
        std::string(USER_EL4_DIR) + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL5_KEY),
        std::string(USER_EL5_DIR) + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByUserAndType(user, static_cast<KeyType>(0)), "");
    GTEST_LOG_(INFO) << "KeyManager_GetKeyDirByUserAndType_001 end";
}

/**
 * @tc.name: KeyManager_GetKeyDirByType_001
 * @tc.desc: Verify the GetKeyDirByType function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GetKeyDirByType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GetKeyDirByType_001 Start";
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByType(EL1_KEY), USER_EL1_DIR);
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByType(EL2_KEY), USER_EL2_DIR);
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByType(EL3_KEY), USER_EL3_DIR);
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByType(EL4_KEY), USER_EL4_DIR);
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByType(EL5_KEY), USER_EL5_DIR);
    EXPECT_EQ(KeyManager::GetInstance().GetKeyDirByType(static_cast<KeyType>(0)), "");
    GTEST_LOG_(INFO) << "KeyManager_GetKeyDirByType_001 end";
}

/**
 * @tc.name: KeyManager_SaveUserElKey_001
 * @tc.desc: Verify the SaveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SaveUserElKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_001 Start";
    unsigned int user = 800;
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY);
    OHOS::ForceRemoveDirectory(keyDir);
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL1_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL1_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL1_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL1_KEY, false), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_001 end";
}

/**
 * @tc.name: KeyManager_SaveUserElKey_002
 * @tc.desc: Verify the SaveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SaveUserElKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_002 Start";
    unsigned int user = 800;
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL2_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL2_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL2_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL2_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL2_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_002 end";
}

/**
 * @tc.name: KeyManager_SaveUserElKey_003
 * @tc.desc: Verify the SaveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SaveUserElKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_003 Start";
    unsigned int user = 800;
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL3_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL3_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL3_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL3_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL3_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_003 end";
}

/**
 * @tc.name: KeyManager_SaveUserElKey_004
 * @tc.desc: Verify the SaveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SaveUserElKey_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_004 Start";
    unsigned int user = 800;
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL4_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL4_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL4_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL4_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL4_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_004 end";
}

/**
 * @tc.name: KeyManager_SaveUserElKey_005
 * @tc.desc: Verify the SaveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SaveUserElKey_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_005 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL5_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL5_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, EL5_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL5_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(user, EL5_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_005 end";
}

/**
 * @tc.name: KeyManager_SaveUserElKey_006
 * @tc.desc: Verify the SaveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SaveUserElKey_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_006 Start";
    unsigned int user = 800;
    EXPECT_EQ(KeyManager::GetInstance().GetUserElKey(user, static_cast<KeyType>(0)), nullptr);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_006 end";
}

/**
 * @tc.name: KeyManager_GetUserDelayHandler_001
 * @tc.desc: Verify the GetUserDelayHandler function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GetUserDelayHandler_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GetUserDelayHandler_001 Start";
    unsigned int user = 800;
    std::shared_ptr<DelayHandler> userDelayHandler;
    EXPECT_EQ(KeyManager::GetInstance().GetUserDelayHandler(user, userDelayHandler), true);

    KeyManager::GetInstance().userLockScreenTask_[user] = nullptr;
    EXPECT_EQ(KeyManager::GetInstance().GetUserDelayHandler(user, userDelayHandler), false);
    KeyManager::GetInstance().userLockScreenTask_.erase(user);
    GTEST_LOG_(INFO) << "KeyManager_GetUserDelayHandler_001 end";
}

/**
 * @tc.name: KeyManager_GetLockScreenStatus_001
 * @tc.desc: Verify the GetLockScreenStatus function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GetLockScreenStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GetLockScreenStatus_001 Start";
    unsigned int user = 800;
    bool lockScreenStatus;
    EXPECT_EQ(KeyManager::GetInstance().GetLockScreenStatus(user, lockScreenStatus), false);

    KeyManager::GetInstance().saveLockScreenStatus[user] = true;
    shared_ptr<DelayHandler> delayHandler;
    EXPECT_EQ(KeyManager::GetInstance().GetUserDelayHandler(user, delayHandler), true);
    KeyManager::GetInstance().saveLockScreenStatus.erase(user);
    GTEST_LOG_(INFO) << "KeyManager_GetLockScreenStatus_001 end";
}

/**
 * @tc.name: KeyManager_InactiveUserElKey_001
 * @tc.desc: Verify the InactiveUserElKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_InactiveUserElKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_InactiveUserElKey_001 Start";
    unsigned int user = 800;
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    EXPECT_EQ(KeyManager::GetInstance().InactiveUserElKey(user, EL1_KEY), E_PARAMS_INVALID);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, elKey);

    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().InactiveUserElKey(user, EL1_KEY), E_ELX_KEY_INACTIVE_ERROR);

    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return(""));
    EXPECT_EQ(KeyManager::GetInstance().InactiveUserElKey(user, EL1_KEY), 0);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL1_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return(""));
    EXPECT_EQ(KeyManager::GetInstance().InactiveUserElKey(user, EL1_KEY), 0);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL1_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("el1"));
    EXPECT_EQ(KeyManager::GetInstance().InactiveUserElKey(user, EL1_KEY), 0);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL1_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("el2"));
    EXPECT_EQ(KeyManager::GetInstance().InactiveUserElKey(user, EL1_KEY), 0);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL1_KEY));
    GTEST_LOG_(INFO) << "KeyManager_InactiveUserElKey_001 end";
}

/**
 * @tc.name: KeyManager_InActiveUserKey_001
 * @tc.desc: Verify the InActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_InActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_InactiveUserElKey_001 Start";
    unsigned int user = 800;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), E_PARAMS_INVALID);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(-1));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), E_ELX_KEY_INACTIVE_ERROR);
    EXPECT_TRUE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return(""));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), E_PARAMS_INVALID);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("el1"));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), E_PARAMS_INVALID);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("el2"));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), E_PARAMS_INVALID);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));
    GTEST_LOG_(INFO) << "KeyManager_InactiveUserElKey_001 end";
}

/**
 * @tc.name: KeyManager_InActiveUserKey_002
 * @tc.desc: Verify the InActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_InActiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_InActiveUserKey_002 end";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("")).WillOnce(Return(""));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), E_PARAMS_INVALID);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL3_KEY));

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("")).WillOnce(Return("")).WillOnce(Return(""));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), E_PARAMS_INVALID);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL3_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL4_KEY));
    GTEST_LOG_(INFO) << "KeyManager_InActiveUserKey_002 end";
}

/**
 * @tc.name: KeyManager_InActiveUserKey_003
 * @tc.desc: Verify the InActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_InActiveUserKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_InActiveUserKey_003 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL5_KEY, elKey);
    KeyManager::GetInstance().userLockScreenTask_[user] = std::make_shared<DelayHandler>(user);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("")).WillOnce(Return("")).WillOnce(Return(""))
        .WillOnce(Return(""));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), 0);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL3_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL4_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL5_KEY));
    EXPECT_TRUE(KeyManager::GetInstance().userLockScreenTask_.find(user) ==
        KeyManager::GetInstance().userLockScreenTask_.end());

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL5_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetKeyDir()).WillOnce(Return("")).WillOnce(Return("")).WillOnce(Return(""))
        .WillOnce(Return(""));
    EXPECT_EQ(KeyManager::GetInstance().InActiveUserKey(user), 0);
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL2_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL3_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL4_KEY));
    EXPECT_FALSE(KeyManager::GetInstance().HasElkey(user, EL5_KEY));
    GTEST_LOG_(INFO) << "KeyManager_InActiveUserKey_003 end";
}

/**
 * @tc.name: KeyManager_getElxKeyPath_001
 * @tc.desc: Verify the getElxKeyPath function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_getElxKeyPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_getElxKeyPath_001 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    std::string elxKeyPath;
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, elKey);
    EXPECT_EQ(KeyManager::GetInstance().getElxKeyPath(user, EL3_KEY, elxKeyPath), 0);
    EXPECT_EQ(elxKeyPath, "/data/test");

    elxKeyPath.clear();
    EXPECT_EQ(KeyManager::GetInstance().getElxKeyPath(user, EL4_KEY, elxKeyPath), 0);
    EXPECT_EQ(elxKeyPath, "/data/test");

    elxKeyPath.clear();
    EXPECT_EQ(KeyManager::GetInstance().getElxKeyPath(user, EL2_KEY, elxKeyPath), 0);
    EXPECT_EQ(elxKeyPath, "/data/test");

    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);
    EXPECT_EQ(KeyManager::GetInstance().getElxKeyPath(user, EL3_KEY, elxKeyPath), -ENOENT);
    EXPECT_EQ(KeyManager::GetInstance().getElxKeyPath(user, EL4_KEY, elxKeyPath), -ENOENT);
    GTEST_LOG_(INFO) << "KeyManager_getElxKeyPath_001 end";
}

/**
 * @tc.name: KeyManager_SetDirectoryElPolicy_001
 * @tc.desc: Verify the SetDirectoryElPolicy function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SetDirectoryElPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_001 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    FileList file = { 100, "/test/path" };
    std::vector<FileList> vec;
    vec.push_back(file);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL1_KEY, vec), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL1_KEY, vec), -ENOENT);

    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL1_KEY, vec), E_LOAD_AND_SET_POLICY_ERR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL1_KEY, vec), 0);
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_001 end";
}

/**
 * @tc.name: KeyManager_SetDirectoryElPolicy_002
 * @tc.desc: Verify the SetDirectoryElPolicy function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SetDirectoryElPolicy_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_002 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    FileList file = { 100, "/test/path" };
    std::vector<FileList> vec;
    vec.push_back(file);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL2_KEY, vec), -ENOENT);

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL2_KEY, vec), 0);
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_002 end";
}

/**
 * @tc.name: KeyManager_SetDirectoryElPolicy_003
 * @tc.desc: Verify the SetDirectoryElPolicy function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SetDirectoryElPolicy_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_003 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    FileList file = { 100, "/test/path" };
    std::vector<FileList> vec;
    vec.push_back(file);
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL3_KEY, vec), -ENOENT);

    KeyManager::GetInstance().SaveUserElKey(user, EL3_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL3_KEY, vec), E_LOAD_AND_SET_ECE_POLICY_ERR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL3_KEY, vec), 0);
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    KeyManager::GetInstance().DeleteElKey(user, EL3_KEY);
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_003 end";
}

/**
 * @tc.name: KeyManager_SetDirectoryElPolicy_004
 * @tc.desc: Verify the SetDirectoryElPolicy function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_SetDirectoryElPolicy_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_004 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    FileList file = { 100, "/test/path" };
    std::vector<FileList> vec;
    vec.push_back(file);
    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, elKey);
    KeyManager::GetInstance().SaveUserElKey(user, EL4_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL4_KEY, vec), 0);
    KeyManager::GetInstance().DeleteElKey(user, EL4_KEY);

    KeyManager::GetInstance().SaveUserElKey(user, EL5_KEY, elKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, EL5_KEY, vec), 0);
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().SetDirectoryElPolicy(user, static_cast<KeyType>(0), vec), 0);
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy_004 end";
}

/**
 * @tc.name: KeyManager_Generate_And_Install_El5_Key_001
 * @tc.desc: Verify the KeyManager GenerateAndInstallEl5Key function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_And_Install_El5_Key_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallEl5Key_0100 start";
    uint32_t userId = 123;
    const string USER_EL5_DIR = "/data/test/user/el5";
    std::string token = "bad_token";
    std::string secret = "bad_secret";
    std::vector<uint8_t> badToken(token.begin(), token.end());
    std::vector<uint8_t> badSecret(secret.begin(), secret.end());
    UserAuth badUserAuth {
            .token = badToken,
            .secret = badSecret
    };
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    auto ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, USER_EL5_DIR, badUserAuth);
    EXPECT_EQ(ret, E_GLOBAL_KEY_NULLPTR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, USER_EL5_DIR, badUserAuth);
    EXPECT_EQ(ret, E_EL5_ADD_CLASS_ERROR);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallEl5Key_0100 end";
}

/**
 * @tc.name: KeyManager_Generate_And_Install_El5_Key_002
 * @tc.desc: Verify the KeyManager GenerateAndInstallEl5Key function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_And_Install_El5_Key_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallEl5Key_0200 start";
    uint32_t userId = 123;
    const string TEST_DIR = "/data/test";
    UserAuth badUserAuth;

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(E_OK)));
    auto ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);

    std::string token = "bad_token";
    std::string secret = "bad_secret";
    std::vector<uint8_t> badToken(token.begin(), token.end());
    std::vector<uint8_t> badSecret(secret.begin(), secret.end());
    badUserAuth.token = badToken;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(E_OK)));
    ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);

    badUserAuth.secret = badSecret;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(E_OK)));
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, E_EL5_ENCRYPT_CLASS_ERROR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(E_OK)));
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(E_OK));
    ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAndInstallEl5Key_0300 end";
}

/**
 * @tc.name: KeyManager_Generate_And_Install_El5_Key_003
 * @tc.desc: Verify the KeyManager GenerateAndInstallEl5Key function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_And_Install_El5_Key_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_Generate_And_Install_El5_Key_003 start";
    uint32_t userId = 123;
    const string TEST_DIR = "/data/test";
    std::string token = "bad_token";
    std::string secret = "bad_secret";
    std::vector<uint8_t> badToken(token.begin(), token.end());
    std::vector<uint8_t> badSecret(secret.begin(), secret.end());
    UserAuth badUserAuth;

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(false), SetArgReferee<1>(false), Return(E_OK)));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(-1));
    auto ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(false), SetArgReferee<1>(false), Return(E_OK)));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(E_OK));
    ret = KeyManager::GetInstance().GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "KeyManager_Generate_And_Install_El5_Key_003 end";
}

/**
 * @tc.name: KeyManager_IsNeedClearKeyFile_001
 * @tc.desc: Verify the KeyManager IsNeedClearKeyFile function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_IsNeedClearKeyFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_IsNeedClearKeyFile_0100 start";
    std::string TEST_DIR = "/data/123456";
    auto ret = KeyManager::GetInstance().IsNeedClearKeyFile(TEST_DIR);
    EXPECT_FALSE(ret);

    TEST_DIR = "/data/test";
    ret = KeyManager::GetInstance().IsNeedClearKeyFile(TEST_DIR);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "KeyManager_IsNeedClearKeyFile_0100 end";
}

/**
 * @tc.name: KeyManager_ProcUpgraeKey_001
 * @tc.desc: Verify the KeyManager ProcUpgradeKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_ProcUpgradeKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ProcUpgradeKey_0100 start";
    FileList file = { 100, "/test/path" };
    std::vector<FileList> vec;
    vec.push_back(file);
    KeyManager::GetInstance().ProcUpgradeKey(vec);
    auto ret = KeyManager::GetInstance().IsNeedClearKeyFile("/test/path/latest/need_restore");
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "KeyManager_ProcUpgradeKey_0100 end";
}

/**
 * @tc.name: KeyManager_ProcUpgraeKey_002
 * @tc.desc: Verify the KeyManager ProcUpgradeKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_ProcUpgradeKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ProcUpgradeKey_0200 start";
    FileList file = { 100, "/data/test" };
    std::vector<FileList> vec;
    vec.push_back(file);
    KeyManager::GetInstance().ProcUpgradeKey(vec);
    auto ret = KeyManager::GetInstance().IsNeedClearKeyFile("/data/test/latest/need_restore");
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "KeyManager_ProcUpgradeKey_0200 end";
}

/**
 * @tc.name: KeyManager_GenerateUserKeys_001
 * @tc.desc: Verify the KeyManager GenerateUserKeys function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_GenerateUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateUserKeys_0100 start";
    uint32_t userId = 124;
    uint32_t flags = 1;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    auto ret = KeyManager::GetInstance().GenerateUserKeys(userId, flags);
    EXPECT_EQ(ret, 0);

    flags = 0;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    ret = KeyManager::GetInstance().GenerateUserKeys(userId, flags);
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    ret = KeyManager::GetInstance().GenerateUserKeys(userId, flags);
    EXPECT_EQ(ret, E_GLOBAL_KEY_NULLPTR);
    GTEST_LOG_(INFO) << "KeyManager_GenerateUserKeys_0100 end";
}

/**
 * @tc.name: KeyManager_Generate_Elx_And_Install_User_key_001
 * @tc.desc: Verify the KeyManager GenerateElxAndInstallUserKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_Elx_And_Install_User_key_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateElxAndInstallUserKey_0100 start";
    uint32_t userId = 125;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    auto ret = KeyManager::GetInstance().GenerateElxAndInstallUserKey(userId);
    EXPECT_NE(ret, 0);
    GTEST_LOG_(INFO) << "KeyManager_GenerateElxAndInstallUserKey_0100 end";
}

/**
 * @tc.name: KeyManager_Generate_Elx_And_Install_User_key_101
 * @tc.desc: Verify the KeyManager GenerateElxAndInstallUserKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_Elx_And_Install_User_key_101, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateElxAndInstallUserKey_0101 start";
    uint32_t userId = 125;
    const std::string el1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    const std::string el2Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    const std::string el3Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    const std::string el4Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(el1Path, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    auto ret = KeyManager::GetInstance().GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(el1Path);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(el2Path, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    ret = KeyManager::GetInstance().GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(el2Path);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(el3Path, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    ret = KeyManager::GetInstance().GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(el3Path);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(el4Path, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    ret = KeyManager::GetInstance().GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(el4Path);
    GTEST_LOG_(INFO) << "KeyManager_GenerateElxAndInstallUserKey_0101 end";
}

/**
 * @tc.name: KeyManager_Generate_Elx_And_Install_User_key_102
 * @tc.desc: Verify the KeyManager GenerateElxAndInstallUserKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_Elx_And_Install_User_key_102, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateElxAndInstallUserKey_0102 start";
    uint32_t userId = 125;
    const std::string el5Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(el5Path, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    auto ret = KeyManager::GetInstance().GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);
    RmDirRecurse(el5Path);
    GTEST_LOG_(INFO) << "KeyManager_GenerateElxAndInstallUserKey_0102 end";
}

/**
 * @tc.name: KeyManager_Generate_User_Key_By_Type_002
 * @tc.desc: Verify the KeyManager GenerateUserKeyByType function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_User_Key_By_Type_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateUserKeyByType_0200 start";
    uint32_t userId = 127;
    KeyType keyType = EL1_KEY;
    std::string token = "bad_token";
    std::string secret = "bad_secret";
    std::vector<uint8_t> badToken(token.begin(), token.end());
    std::vector<uint8_t> badSecret(secret.begin(), secret.end());
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    auto ret = KeyManager::GetInstance().GenerateUserKeyByType(userId, keyType, badToken, badSecret);
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    std::string elPath = KeyManager::GetInstance().GetKeyDirByType(keyType);
    EXPECT_TRUE(MkDir(elPath, S_IRWXU));
    std::string elUserKeyPath = elPath + "/" + std::to_string(userId);
    EXPECT_FALSE(MkDir(elUserKeyPath, S_IRWXU));
    ret = KeyManager::GetInstance().GenerateUserKeyByType(userId, keyType, badToken, badSecret);
    EXPECT_EQ(ret, -EEXIST);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(elUserKeyPath));
    GTEST_LOG_(INFO) << "KeyManager_GenerateUserKeyByType_0200 end";
}

/**
 * @tc.name: KeyManager_UpdateCeEceSeceUserAuth_001
 * @tc.desc: Verify the KeyManager UpdateCeEceSeceUserAuth function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_UpdateCeEceSeceUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_0100 start";
    uint32_t userId = 128;
    UserTokenSecret userTokenSecret = {.token = {'t', 'o', 'k', 'e', 'n'}, .oldSecret = {},
            .newSecret = {'s', 'e', 'c', 'r', 'e', 't'}, .secureUid = 0};
    KeyType keyType = EL2_KEY;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    auto ret = KeyManager::GetInstance().UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType, false);
    #else
    auto ret = KeyManager::GetInstance().UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType);
    #endif
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    ret = KeyManager::GetInstance().UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType, false);
    #else
    ret = KeyManager::GetInstance().UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType);
    #endif
    EXPECT_NE(ret, 0);
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_0100 end";
}

/**
 * @tc.name: KeyManager_UpdateESecret_001
 * @tc.desc: Verify the KeyManager UpdateESecret function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_UpdateESecret_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateESecret_0100 start";
    uint32_t userId = 129;
    UserTokenSecret emptyTokenSecret;

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    auto ret = KeyManager::GetInstance().UpdateESecret(userId, emptyTokenSecret);
    EXPECT_NE(ret, 0);

    UserTokenSecret userTokenSecret = {.token = {'t', 'o', 'k', 'e', 'n'}, .oldSecret = {},
            .newSecret = {'s', 'e', 'c', 'r', 'e', 't'}, .secureUid = 0};
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    ret = KeyManager::GetInstance().UpdateESecret(userId, userTokenSecret);
    EXPECT_NE(ret, 0);

    UserTokenSecret newTokenSecret = {.token = {'t', 'o', 'k', 'e', 'n'}, .oldSecret = {'t', 'e', 's', 't'},
            .newSecret = {'s', 'e', 'c', 'r', 'e', 't'}, .secureUid = 0};
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    ret = KeyManager::GetInstance().UpdateESecret(userId, newTokenSecret);
    EXPECT_NE(ret, 0);
    GTEST_LOG_(INFO) << "KeyManager_UpdateESecret_0100 end";
}

#ifdef EL5_FILEKEY_MANAGER
/**
 * @tc.name: KeyManager_RegisterUeceActivationCallback_001
 * @tc.desc: Verify the KeyManager RegisterUeceActivationCallback function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_RegisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_RegisterUeceActivationCallback_001 start";
    auto &keyManager = KeyManager::GetInstance();

    sptr<IUeceActivationCallback> ueceCallback = nullptr;
    EXPECT_EQ(keyManager.RegisterUeceActivationCallback(ueceCallback), E_PARAMS_INVALID);

    keyManager.ueceCallback_ = nullptr;
    ueceCallback = sptr<IUeceActivationCallback>(new (std::nothrow) UeceActivationCallbackMock());
    EXPECT_EQ(keyManager.RegisterUeceActivationCallback(ueceCallback), E_OK);
    EXPECT_EQ(keyManager.RegisterUeceActivationCallback(ueceCallback), E_OK);

    GTEST_LOG_(INFO) << "KeyManager_RegisterUeceActivationCallback_001 end";
}

/**
 * @tc.name: KeyManager_UnregisterUeceActivationCallback_001
 * @tc.desc: Verify the KeyManager UnregisterUeceActivationCallback function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_UnregisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UnregisterUeceActivationCallback_001 start";
    auto &keyManager = KeyManager::GetInstance();
    keyManager.ueceCallback_ = sptr<IUeceActivationCallback>(new (std::nothrow) UeceActivationCallbackMock());
    EXPECT_EQ(keyManager.UnregisterUeceActivationCallback(), E_OK);
    EXPECT_EQ(keyManager.UnregisterUeceActivationCallback(), E_OK);

    GTEST_LOG_(INFO) << "KeyManager_UnregisterUeceActivationCallback_001 end";
}

/**
 * @tc.name: KeyManager_NotifyUeceActivation_001
 * @tc.desc: Verify the KeyManager NotifyUeceActivation function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_NotifyUeceActivation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_NotifyUeceActivation_001 start";
    auto &keyManager = KeyManager::GetInstance();

    uint32_t userId = 100;
    keyManager.ueceCallback_ = nullptr;
    EXPECT_EQ(keyManager.NotifyUeceActivation(userId, E_OK, true), E_OK);

    auto ueceCallback = sptr<UeceActivationCallbackMock>(new (std::nothrow) UeceActivationCallbackMock());
    keyManager.ueceCallback_ = ueceCallback;

    EXPECT_CALL(*ueceCallback, OnEl5Activation(_, _, _, _))
        .WillOnce(DoAll(Invoke([]() { sleep(1); }), Return(E_OK)));
    EXPECT_EQ(keyManager.NotifyUeceActivation(userId, E_OK, true), E_OK);

    EXPECT_CALL(*ueceCallback, OnEl5Activation(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(E_PARAMS_INVALID), Return(E_OK)));
    EXPECT_EQ(keyManager.NotifyUeceActivation(userId, E_PARAMS_INVALID, true), E_OK);

    EXPECT_CALL(*ueceCallback, OnEl5Activation(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(E_PARAMS_INVALID), Return(E_OK)));
    EXPECT_EQ(keyManager.NotifyUeceActivation(userId, E_OK, true), E_OK);

    EXPECT_CALL(*ueceCallback, OnEl5Activation(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(E_OK), Return(E_OK)));
    EXPECT_EQ(keyManager.NotifyUeceActivation(userId, E_OK, true), E_OK);

    GTEST_LOG_(INFO) << "KeyManager_NotifyUeceActivation_001 end";
}
#endif
}
