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
#include "utils/file_utils.h"

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
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), -EFAULT);
    EXPECT_NE(KeyManager::GetInstance()->userEl5Key_.find(user), KeyManager::GetInstance()->userEl5Key_.end());

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(true));
    KeyManager::GetInstance()->userEl5Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->ActiveCeSceSeceUserKey(user, EL5_KEY, token, secret), 0);
    EXPECT_NE(KeyManager::GetInstance()->userEl5Key_.find(user), KeyManager::GetInstance()->userEl5Key_.end());
    KeyManager::GetInstance()->userEl5Key_.erase(user);
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

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), -EFAULT);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), -EFAULT);
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
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserScreen(user, token, secret), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UnlockUserScreen(100, token, secret), 0);
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

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), 0);
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContextt end";
}

/**
 * @tc.name: KeyManager_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_UpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_001 Start";
    uint32_t userId = 1;
    KeyManager::GetInstance()->userEl5Key_.erase(userId);
    KeyManager::GetInstance()->saveESecretStatus[userId] = false;
    auto fd = open(UECE_PATH, O_RDWR);
    bool exist = true;
    if (fd < 0) {
        EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
           .WillOnce(Return(false)).WillOnce(Return(false));
        EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), 0);
        close(fd);
        exist = false;
        fd = open(UECE_PATH, O_RDWR | O_CREAT);
        ASSERT_TRUE(fd != -1) << "UpdateKeyContext Create File Failed!" << errno;;
        close(fd);
    }

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), 0);

    KeyManager::GetInstance()->saveESecretStatus[userId] = true;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), -ENOENT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateKeyContext(userId), 0);
    KeyManager::GetInstance()->saveESecretStatus.erase(userId);

    if (!exist) {
        auto ret = unlink(UECE_PATH);
        ASSERT_TRUE(ret != -1) << "Failed to delete file in KeyManager_UpdateKeyContext_001! " << errno;
    }
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_001 end";
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
    KeyManager::GetInstance()->userEl1Key_.erase(800);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), -EOPNOTSUPP);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(false));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(false));
    #endif
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(true));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(true));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), -EFAULT);
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
    KeyManager::GetInstance()->userEl1Key_.erase(800);
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
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL1_KEY), true);
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL1_KEY), true);
    KeyManager::GetInstance()->userEl1Key_.erase(800);

    KeyManager::GetInstance()->userEl2Key_.erase(800);
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
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL2_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL2_KEY), true);
    KeyManager::GetInstance()->userEl2Key_.erase(800);
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
    KeyManager::GetInstance()->userEl3Key_.erase(800);
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
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL3_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL3_KEY), true);
    KeyManager::GetInstance()->userEl3Key_.erase(800);

    KeyManager::GetInstance()->userEl4Key_.erase(800);
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
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAndInstallUserKey(800, "/data/test", auth, EL4_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL4_KEY), true);
    KeyManager::GetInstance()->userEl4Key_.erase(800);
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
    KeyManager::GetInstance()->userEl1Key_.erase(800);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL1_KEY), -EOPNOTSUPP);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL1_KEY), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL1_KEY), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL1_KEY), -EFAULT);
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
    KeyManager::GetInstance()->userEl1Key_.erase(800);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL1_KEY), true);
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL1_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL1_KEY), true);
    KeyManager::GetInstance()->userEl1Key_.erase(800);

    KeyManager::GetInstance()->userEl2Key_.erase(800);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL2_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL2_KEY), true);
    KeyManager::GetInstance()->userEl2Key_.erase(800);

    KeyManager::GetInstance()->userEl3Key_.erase(800);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL3_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL3_KEY), true);
    KeyManager::GetInstance()->userEl3Key_.erase(800);
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
    KeyManager::GetInstance()->userEl4Key_.erase(800);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL4_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL4_KEY), true);
    KeyManager::GetInstance()->userEl4Key_.erase(800);

    KeyManager::GetInstance()->userEl5Key_.erase(800);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->RestoreUserKey(800, "/data/test", auth, EL5_KEY), 0);
    EXPECT_EQ(KeyManager::GetInstance()->HasElkey(800, EL5_KEY), true);
    KeyManager::GetInstance()->userEl5Key_.erase(800);
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
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL1_KEY), USER_EL1_DIR + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL2_KEY), USER_EL2_DIR + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL3_KEY), USER_EL3_DIR + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL4_KEY), USER_EL4_DIR + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL5_KEY), USER_EL5_DIR + "/" + to_string(user));
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByUserAndType(user, static_cast<KeyType>(0)), "");
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
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByType(EL1_KEY), USER_EL1_DIR);
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByType(EL2_KEY), USER_EL2_DIR);
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByType(EL3_KEY), USER_EL3_DIR);
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByType(EL4_KEY), USER_EL4_DIR);
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByType(EL5_KEY), USER_EL5_DIR);
    EXPECT_EQ(KeyManager::GetInstance()->GetKeyDirByType(static_cast<KeyType>(0)), "");
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
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL1_KEY);
    OHOS::ForceRemoveDirectory(keyDir);
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->userEl1Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance()->userEl1Key_.erase(user);
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
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL2_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->userEl2Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance()->userEl2Key_.erase(user);
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
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL3_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->userEl3Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance()->userEl3Key_.erase(user);
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
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL4_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->userEl4Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance()->userEl4Key_.erase(user);
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
    string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL5_KEY);
    OHOS::ForceRemoveDirectory(keyDir);

    KeyManager::GetInstance()->userEl5Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance()->userEl5Key_.erase(user);
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
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, static_cast<KeyType>(0)), nullptr);
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
    EXPECT_EQ(KeyManager::GetInstance()->GetUserDelayHandler(user, userDelayHandler), true);

    KeyManager::GetInstance()->userLockScreenTask_[user] = nullptr;
    EXPECT_EQ(KeyManager::GetInstance()->GetUserDelayHandler(user, userDelayHandler), false);
    KeyManager::GetInstance()->userLockScreenTask_.erase(user);
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
    EXPECT_EQ(KeyManager::GetInstance()->GetLockScreenStatus(user, lockScreenStatus), false);

    KeyManager::GetInstance()->saveLockScreenStatus[user] = true;
    shared_ptr<DelayHandler> delayHandler;
    EXPECT_EQ(KeyManager::GetInstance()->GetUserDelayHandler(user, delayHandler), true);
    KeyManager::GetInstance()->saveLockScreenStatus.erase(user);
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
    string keyId;
    std::map<unsigned int, std::shared_ptr<BaseKey>> userElxKey_;
    EXPECT_EQ(KeyManager::GetInstance()->InactiveUserElKey(user, userElxKey_), -ENOENT);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    userElxKey_[user] = elKey;

    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->InactiveUserElKey(user, userElxKey_), -EFAULT);

    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InactiveUserElKey(user, userElxKey_), 0);
    EXPECT_TRUE(userElxKey_.find(user) == userElxKey_.end());
    GTEST_LOG_(INFO) << "KeyManager_DeleteAppkey_001 end";
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
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), -ENOENT);

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), -ENOENT);
    EXPECT_TRUE(KeyManager::GetInstance()->userEl2Key_.find(user) == KeyManager::GetInstance()->userEl2Key_.end());

    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    KeyManager::GetInstance()->userEl3Key_[user] = elKey;
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), -ENOENT);
    EXPECT_TRUE(KeyManager::GetInstance()->userEl2Key_.find(user) == KeyManager::GetInstance()->userEl2Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userEl3Key_.find(user) == KeyManager::GetInstance()->userEl3Key_.end());
    GTEST_LOG_(INFO) << "KeyManager_DeleteAppkey_001 end";
}

/**
 * @tc.name: KeyManager_InActiveUserKey_002
 * @tc.desc: Verify the InActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_InActiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_InActiveUserKey_002 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");

    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    KeyManager::GetInstance()->userEl3Key_[user] = elKey;
    KeyManager::GetInstance()->userEl4Key_[user] = elKey;
    KeyManager::GetInstance()->userEl5Key_[user] = elKey;
    KeyManager::GetInstance()->userLockScreenTask_[user] = std::make_shared<DelayHandler>(user);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), -ENOENT);
    EXPECT_TRUE(KeyManager::GetInstance()->userEl2Key_.find(user) == KeyManager::GetInstance()->userEl2Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userEl3Key_.find(user) == KeyManager::GetInstance()->userEl3Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userEl4Key_.find(user) == KeyManager::GetInstance()->userEl4Key_.end());
    EXPECT_FALSE(KeyManager::GetInstance()->userLockScreenTask_.find(user) ==
        KeyManager::GetInstance()->userLockScreenTask_.end());

    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    KeyManager::GetInstance()->userEl3Key_[user] = elKey;
    KeyManager::GetInstance()->userEl4Key_[user] = elKey;
    KeyManager::GetInstance()->userEl5Key_[user] = elKey;
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), -ENOENT);
    EXPECT_TRUE(KeyManager::GetInstance()->userEl2Key_.find(user) == KeyManager::GetInstance()->userEl2Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userEl3Key_.find(user) == KeyManager::GetInstance()->userEl3Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userEl4Key_.find(user) == KeyManager::GetInstance()->userEl4Key_.end());
    GTEST_LOG_(INFO) << "KeyManager_InActiveUserKey_002 end";
}

/**
 * @tc.name: KeyManager_getEceSeceKeyPath_001
 * @tc.desc: Verify the getEceSeceKeyPath function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_getEceSeceKeyPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_getEceSeceKeyPath_001 Start";
    unsigned int user = 800;
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    std::string eceSeceKeyPath;
    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    KeyManager::GetInstance()->userEl3Key_[user] = elKey;
    KeyManager::GetInstance()->userEl4Key_[user] = elKey;
    EXPECT_EQ(KeyManager::GetInstance()->getEceSeceKeyPath(user, EL3_KEY, eceSeceKeyPath), 0);
    EXPECT_EQ(eceSeceKeyPath, "/data/test");

    eceSeceKeyPath.clear();
    EXPECT_EQ(KeyManager::GetInstance()->getEceSeceKeyPath(user, EL4_KEY, eceSeceKeyPath), 0);
    EXPECT_EQ(eceSeceKeyPath, "/data/test");

    eceSeceKeyPath.clear();
    EXPECT_EQ(KeyManager::GetInstance()->getEceSeceKeyPath(user, EL2_KEY, eceSeceKeyPath), 0);
    EXPECT_EQ(eceSeceKeyPath, "");

    KeyManager::GetInstance()->userEl2Key_.erase(user);
    KeyManager::GetInstance()->userEl3Key_.erase(user);
    KeyManager::GetInstance()->userEl4Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->getEceSeceKeyPath(user, EL3_KEY, eceSeceKeyPath), -ENOENT);
    EXPECT_EQ(KeyManager::GetInstance()->getEceSeceKeyPath(user, EL4_KEY, eceSeceKeyPath), -ENOENT);
    GTEST_LOG_(INFO) << "KeyManager_getEceSeceKeyPath_001 end";
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
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL1_KEY, vec), 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL1_KEY, vec), -ENOENT);

    KeyManager::GetInstance()->userEl1Key_[user] = elKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL1_KEY, vec), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL1_KEY, vec), 0);
    KeyManager::GetInstance()->userEl1Key_.erase(user);
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
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL2_KEY, vec), -ENOENT);

    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL2_KEY, vec), 0);
    KeyManager::GetInstance()->userEl2Key_.erase(user);
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
    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL3_KEY, vec), -ENOENT);

    KeyManager::GetInstance()->userEl3Key_[user] = elKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL3_KEY, vec), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL3_KEY, vec), 0);
    KeyManager::GetInstance()->userEl2Key_.erase(user);
    KeyManager::GetInstance()->userEl3Key_.erase(user);
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
    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    KeyManager::GetInstance()->userEl4Key_[user] = elKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetEceAndSecePolicy(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL4_KEY, vec), 0);
    KeyManager::GetInstance()->userEl4Key_.erase(user);

    KeyManager::GetInstance()->userEl5Key_[user] = elKey;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, LoadAndSetPolicy(_, _)).WillOnce(Return(0));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, EL5_KEY, vec), 0);
    KeyManager::GetInstance()->userEl5Key_.erase(user);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->SetDirectoryElPolicy(user, static_cast<KeyType>(0), vec), 0);
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
    auto ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, USER_EL5_DIR, badUserAuth);
    EXPECT_EQ(ret, -EOPNOTSUPP);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _)).WillOnce(Return(false));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, USER_EL5_DIR, badUserAuth);
    EXPECT_EQ(ret, -EFAULT);
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
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(true)));
    auto ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);

    std::string token = "bad_token";
    std::string secret = "bad_secret";
    std::vector<uint8_t> badToken(token.begin(), token.end());
    std::vector<uint8_t> badSecret(secret.begin(), secret.end());
    badUserAuth.token = badToken;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(true)));
    ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);

    badUserAuth.secret = badSecret;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(false));
    EXPECT_CALL(*baseKeyMock_, ClearKey(_)).WillOnce(Return(true));
    ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(true), SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(true));
    ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
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
        .WillOnce(DoAll(SetArgReferee<0>(false), SetArgReferee<1>(false), Return(true)));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(false));
    auto ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, AddClassE(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(false), SetArgReferee<1>(false), Return(true)));
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(true));
    ret = KeyManager::GetInstance()->GenerateAndInstallEl5Key(userId, TEST_DIR, badUserAuth);
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
    auto ret = KeyManager::GetInstance()->IsNeedClearKeyFile(TEST_DIR);
    EXPECT_FALSE(ret);

    TEST_DIR = "/data/test";
    ret = KeyManager::GetInstance()->IsNeedClearKeyFile(TEST_DIR);
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
    KeyManager::GetInstance()->ProcUpgradeKey(vec);
    auto ret = KeyManager::GetInstance()->IsNeedClearKeyFile("/test/path/latest/need_restore");
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
    KeyManager::GetInstance()->ProcUpgradeKey(vec);
    auto ret = KeyManager::GetInstance()->IsNeedClearKeyFile("/data/test/latest/need_restore");
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
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(userId, flags);
    EXPECT_EQ(ret, 0);

    flags = 0;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    ret = KeyManager::GetInstance()->GenerateUserKeys(userId, flags);
    EXPECT_EQ(ret, 0);
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
    auto ret = KeyManager::GetInstance()->GenerateElxAndInstallUserKey(userId);
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
    const std::string EL1_PATH = USER_EL1_DIR + "/" + std::to_string(userId);
    const std::string EL2_PATH = USER_EL1_DIR + "/" + std::to_string(userId);
    const std::string EL3_PATH = USER_EL1_DIR + "/" + std::to_string(userId);
    const std::string EL4_PATH = USER_EL1_DIR + "/" + std::to_string(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(EL1_PATH, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    auto ret = KeyManager::GetInstance()->GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(EL1_PATH);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(EL2_PATH, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    ret = KeyManager::GetInstance()->GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(EL2_PATH);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(EL3_PATH, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    ret = KeyManager::GetInstance()->GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(EL3_PATH);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(EL4_PATH, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    ret = KeyManager::GetInstance()->GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);

    RmDirRecurse(EL4_PATH);
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
    const std::string EL5_PATH = USER_EL1_DIR + "/" + std::to_string(userId);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    MkDir(EL5_PATH, S_IRWXU);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    auto ret = KeyManager::GetInstance()->GenerateElxAndInstallUserKey(userId);
    EXPECT_EQ(ret, -EEXIST);
    RmDirRecurse(EL5_PATH);
    GTEST_LOG_(INFO) << "KeyManager_GenerateElxAndInstallUserKey_0102 end";
}

/**
 * @tc.name: KeyManager_Generate_User_Key_By_Type_001
 * @tc.desc: Verify the KeyManager GenerateUserKeyByType function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyManagerTest, KeyManager_Generate_User_Key_By_Type_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateUserKeyByType_0100 start";
    uint32_t userId = 127;
    KeyType keyType = EL1_KEY;
    std::string token = "bad_token";
    std::string secret = "bad_secret";
    std::vector<uint8_t> badToken(token.begin(), token.end());
    std::vector<uint8_t> badSecret(secret.begin(), secret.end());
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    auto ret = KeyManager::GetInstance()->GenerateUserKeyByType(userId, keyType, badToken, badSecret);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "KeyManager_GenerateUserKeyByType_0100 end";
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
    auto ret = KeyManager::GetInstance()->GenerateUserKeyByType(userId, keyType, badToken, badSecret);
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    std::string elPath = KeyManager::GetInstance()->GetKeyDirByType(keyType);
    RmDirRecurse(elPath);
    ret = KeyManager::GetInstance()->GenerateUserKeyByType(userId, keyType, badToken, badSecret);
    EXPECT_EQ(ret, -ENOENT);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    elPath = KeyManager::GetInstance()->GetKeyDirByType(keyType);
    EXPECT_FALSE(MkDir(elPath, S_IRWXU));
    std::string elUserKeyPath = elPath + "/" + std::to_string(userId);
    EXPECT_FALSE(MkDir(elUserKeyPath, S_IRWXU));
    ret = KeyManager::GetInstance()->GenerateUserKeyByType(userId, keyType, badToken, badSecret);
    EXPECT_EQ(ret, -EEXIST);
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
    auto ret = KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType, false);
    #else
    auto ret = KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType);
    #endif
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    ret = KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType, false);
    #else
    ret = KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType);
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

    auto ret = KeyManager::GetInstance()->UpdateESecret(userId, emptyTokenSecret);
    EXPECT_NE(ret, 0);

    UserTokenSecret userTokenSecret = {.token = {'t', 'o', 'k', 'e', 'n'}, .oldSecret = {},
            .newSecret = {'s', 'e', 'c', 'r', 'e', 't'}, .secureUid = 0};
    ret = KeyManager::GetInstance()->UpdateESecret(userId, userTokenSecret);
    EXPECT_NE(ret, 0);

    UserTokenSecret newTokenSecret = {.token = {'t', 'o', 'k', 'e', 'n'}, .oldSecret = {'t', 'e', 's', 't'},
            .newSecret = {'s', 'e', 'c', 'r', 'e', 't'}, .secureUid = 0};
    ret = KeyManager::GetInstance()->UpdateESecret(userId, newTokenSecret);
    EXPECT_NE(ret, 0);
    GTEST_LOG_(INFO) << "KeyManager_UpdateESecret_0100 end";
}
}
