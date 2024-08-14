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
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v2_mock.h"
#include "fscrypt_key_v2.h"
#include "key_control_mock.h"
#include "storage_service_errno.h"

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
    fscryptControlMock_ = make_shared<FscryptControlMoc>();
    FscryptControlMoc::fscryptControlMoc = fscryptControlMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
    baseKeyMock_ = make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
    fscryptKeyMock_ = make_shared<FscryptKeyV2Moc>();
    FscryptKeyV2Moc::fscryptKeyV2Moc = fscryptKeyMock_;
}

void KeyManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    FscryptControlMoc::fscryptControlMoc = nullptr;
    fscryptControlMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
    FscryptKeyV2Moc::fscryptKeyV2Moc = nullptr;
    fscryptKeyMock_ = nullptr;
}

void KeyManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
}

void KeyManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
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
    EXPECT_EQ(KeyManager::GetInstance()->GetBaseKey("/data/test"), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance()->GetBaseKey("/data/test"), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V1));
    EXPECT_NE(KeyManager::GetInstance()->GetBaseKey("/data/test"), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V1));
    EXPECT_NE(KeyManager::GetInstance()->GetBaseKey("/data/test"), nullptr);
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
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallDeviceKey("/data/test"), -EOPNOTSUPP);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallDeviceKey("/data/test"), -EFAULT);
    EXPECT_EQ(KeyManager::GetInstance()->globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(false));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(false));
    #endif
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallDeviceKey("/data/test"), -EFAULT);
    EXPECT_EQ(KeyManager::GetInstance()->globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(true));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(true));
    #endif
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallDeviceKey("/data/test"), -EFAULT);
    EXPECT_EQ(KeyManager::GetInstance()->globalEl1Key_, nullptr);
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
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(true));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(true));
    #endif

    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallDeviceKey("/data/test"), 0);
    EXPECT_EQ(KeyManager::GetInstance()->hasGlobalDeviceKey_, true);
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
    KeyManager::GetInstance()->globalEl1Key_ = nullptr;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreDeviceKey("/data/test"), -EOPNOTSUPP);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreDeviceKey("/data/test"), -EFAULT);
    EXPECT_EQ(KeyManager::GetInstance()->globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreDeviceKey("/data/test"), -EFAULT);
    EXPECT_EQ(KeyManager::GetInstance()->globalEl1Key_, nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreDeviceKey("/data/test"), -EFAULT);
    EXPECT_EQ(KeyManager::GetInstance()->globalEl1Key_, nullptr);
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
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreDeviceKey("/data/test"), 0);
    EXPECT_NE(KeyManager::GetInstance()->globalEl1Key_, nullptr);

    EXPECT_EQ(KeyManager::GetInstance()->RestoreDeviceKey("/data/test"), 0);
    EXPECT_NE(KeyManager::GetInstance()->globalEl1Key_, nullptr);
    KeyManager::GetInstance()->globalEl1Key_ = nullptr;
    GTEST_LOG_(INFO) << "KeyManager_RestoreDeviceKey_002 end";
}

/**
 * @tc.name: KeyManager_ActiveUserKey
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_ActiveUserKey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveUserKey Start";
    unsigned int user = 1;
    const std::vector<uint8_t> token = {1};
    const std::vector<uint8_t> secret = {1};
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveUserKey(user, token, secret), 0);
    GTEST_LOG_(INFO) << "KeyManager_ActiveUserKey end";
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
    unsigned int user = 1;
    std::vector<uint8_t> token = {};
    std::vector<uint8_t> secret = {};
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), 0);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance()->userEl2Key_[user] = tmpKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL2_KEY, token, secret), 0);
    
    int eL6Key = 6;
    KeyType type = static_cast<KeyType>(eL6Key);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, type, token, secret), E_KEY_TYPE_INVAL);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), -ENOENT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), -ENOENT);

    std::string keyDir = USER_EL1_DIR + "/" + std::to_string(user);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), -EOPNOTSUPP);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL1_KEY, token, secret), -EFAULT);
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
    unsigned int user = 1;
    std::vector<uint8_t> token = {};
    std::vector<uint8_t> secret = {};
    
    std::string keyUeceDir = UECE_DIR + "/" + std::to_string(user);
    std::string keyDir = USER_EL5_DIR + "/" + std::to_string(user);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyUeceDir));
    std::ofstream file(keyDir + "/test.txt");
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), -EFAULT);
    OHOS::RemoveFile(keyDir + "/test.txt");
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyUeceDir));
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyUeceDir));
    std::ofstream file1(keyDir + "/test1.txt");
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), 0);
    OHOS::RemoveFile(keyDir + "/test1.txt");
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
    std::string keyDir = USER_EL1_DIR + "/" + std::to_string(user);
    std::string keyUeceDir = UECE_DIR + "/" + std::to_string(user);

    EXPECT_EQ(KeyManager::GetInstance()->CheckAndDeleteEmptyEl5Directory(keyDir, user), -ENOENT);

    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyUeceDir));
    EXPECT_EQ(KeyManager::GetInstance()->CheckAndDeleteEmptyEl5Directory(keyDir, user), -ENOENT);
    OHOS::ForceRemoveDirectory(keyDir);
    OHOS::ForceRemoveDirectory(keyUeceDir);

    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyUeceDir));
    std::ofstream file(keyDir + "/test.txt");
    EXPECT_EQ(KeyManager::GetInstance()->CheckAndDeleteEmptyEl5Directory(keyDir, user), 0);
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
    std::string keyDir = USER_EL1_DIR + "/" + std::to_string(user);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveElXUserKey(user, token, keyDir, secret, elKey), -EFAULT);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveElXUserKey(user, token, keyDir, secret, elKey), -EFAULT);
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey end";
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
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserScreen(user, token, secret), 0);
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserScreen end";
}

/**
 * @tc.name: KeyManager_UnlockUserAppKeys
 * @tc.desc: Verify the UnlockUserAppKeys function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_UnlockUserAppKeys, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserAppKeys Start";
    uint32_t userId = 1;
    bool needGetAllAppKey = true;

    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), 0);

    std::ofstream file(UECE_PATH);
    EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserAppKeys(userId, needGetAllAppKey), 0);
    OHOS::RemoveFile(UECE_PATH);
    GTEST_LOG_(INFO) << "KeyManager_UnlockUserAppKeys end";
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
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), -ENOENT);
    
    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance()->userEl1Key_[user] = tmpKey;
    vec.push_back({1, "/test"});
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(-EINVAL));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), 0);

    int eL6Key = 6;
    type = static_cast<KeyType>(eL6Key);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), 0);

    type = EL3_KEY;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), -ENOENT);

    KeyManager::GetInstance()->userEl2Key_[user] = tmpKey;
    KeyManager::GetInstance()->userEl3Key_[user] = tmpKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(-EINVAL));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec), 0);
    GTEST_LOG_(INFO) << "KeyManager_SetDirectoryElPolicy end";
}

/**
 * @tc.name: KeyManager_UpdateCeEceSeceKeyContext
 * @tc.desc: Verify the UpdateCeEceSeceKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_UpdateCeEceSeceKeyContext, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceKeyContext Start";
    uint32_t userId = 1;
    KeyType type = EL1_KEY;

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceKeyContext(userId, type), 0);

    KeyManager::GetInstance()->userEl1Key_.erase(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceKeyContext(userId, type), -ENOENT);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance()->userEl1Key_[userId] = tmpKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceKeyContext(userId, type), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceKeyContext(userId, type), 0);
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceKeyContext end";
}

/**
 * @tc.name: KeyManager_UpdateKeyContext
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_UpdateKeyContext, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext Start";
    uint32_t userId = 1;

    KeyManager::GetInstance()->userEl2Key_.erase(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), -ENOENT);

    KeyManager::GetInstance()->userEl3Key_.erase(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(2).WillOnce(Return(false))\
        .WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), -ENOENT);

    KeyManager::GetInstance()->userEl4Key_.erase(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), -ENOENT);

    KeyManager::GetInstance()->userEl5Key_.erase(userId);
    KeyManager::GetInstance()->saveESecretStatus.insert(std::make_pair(1, true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), 0);
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContextt end";
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
    EXPECT_FALSE(KeyManager::GetInstance()->IsUeceSupport());

    EXPECT_TRUE(OHOS::ForceCreateDirectory(UECE_PATH));
    EXPECT_FALSE(KeyManager::GetInstance()->IsUeceSupport());
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(UECE_PATH));

    std::ofstream file(UECE_PATH);
    EXPECT_TRUE(KeyManager::GetInstance()->IsUeceSupport());
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
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(userId, type), -EFAULT);

    type = EL1_KEY;
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(userId, type), -ENOENT);

    std::string keyDir = USER_EL1_DIR + "/" + std::to_string(userId);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(userId, type), 0);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    #endif
    GTEST_LOG_(INFO) << "KeyManager_RestoreUserKey_000 end";
}
}
