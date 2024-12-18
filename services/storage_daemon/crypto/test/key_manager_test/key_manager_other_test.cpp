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

#include "directory_ex.h"

#include "base_key_mock.h"
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v1.h"
#include "fscrypt_key_v2_mock.h"
#include "iam_client_mock.h"
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
class KeyManagerOtherTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<IamClientMoc> iamClientMoc_ = nullptr;
    static inline shared_ptr<FscryptControlMoc> fscryptControlMock_ = nullptr;
    static inline shared_ptr<FscryptKeyV2Moc> fscryptKeyMock_ = nullptr;
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
    static inline shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
};
void KeyManagerOtherTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    iamClientMoc_ = make_shared<IamClientMoc>();
    IamClientMoc::iamClientMoc = iamClientMoc_;
    fscryptControlMock_ = make_shared<FscryptControlMoc>();
    FscryptControlMoc::fscryptControlMoc = fscryptControlMock_;
    fscryptKeyMock_ = make_shared<FscryptKeyV2Moc>();
    FscryptKeyV2Moc::fscryptKeyV2Moc = fscryptKeyMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
    baseKeyMock_ = make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
}

void KeyManagerOtherTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    IamClientMoc::iamClientMoc = nullptr;
    iamClientMoc_ = nullptr;
    FscryptControlMoc::fscryptControlMoc = nullptr;
    fscryptControlMock_ = nullptr;
    FscryptKeyV2Moc::fscryptKeyV2Moc = nullptr;
    fscryptKeyMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
}

void KeyManagerOtherTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
}

void KeyManagerOtherTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
}

/**
 * @tc.name: KeyManager_LoadAllUsersEl1Key_000
 * @tc.desc: Verify the LoadAllUsersEl1Key function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_LoadAllUsersEl1Key_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_LoadAllUsersEl1Key_000 Start";
    EXPECT_EQ(KeyManager::GetInstance()->LoadAllUsersEl1Key(), 0);
    GTEST_LOG_(INFO) << "KeyManager_LoadAllUsersEl1Key_000 end";
}

/**
 * @tc.name: KeyManager_LockUserScreen_000
 * @tc.desc: Verify the LockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_LockUserScreen_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_LockUserScreen_000 Start";
    uint32_t user = 800;
    string basePath = "/data/app/el2/" + to_string(user);
    string path = basePath + "/base";
    ForceRemoveDirectory(basePath);
    EXPECT_EQ(KeyManager::GetInstance()->LockUserScreen(user), 0);

    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_CALL(*iamClientMoc_, HasFaceFinger(_, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance()->LockUserScreen(user), 0);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_CALL(*iamClientMoc_, HasFaceFinger(_, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance()->LockUserScreen(user), 0);
    EXPECT_NE(KeyManager::GetInstance()->userPinProtect.find(user), KeyManager::GetInstance()->userPinProtect.end());
    EXPECT_EQ(KeyManager::GetInstance()->userPinProtect[user], true);

    EXPECT_NE(KeyManager::GetInstance()->saveLockScreenStatus.find(user),
        KeyManager::GetInstance()->saveLockScreenStatus.end());
    EXPECT_EQ(KeyManager::GetInstance()->saveLockScreenStatus[user], false);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, HasFaceFinger(_, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance()->LockUserScreen(user), -ENOENT);

    KeyManager::GetInstance()->userPinProtect[user] = false;
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_CALL(*iamClientMoc_, HasFaceFinger(_, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance()->LockUserScreen(user), 0);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_CALL(*iamClientMoc_, HasFaceFinger(_, _)).WillOnce(Return(1));
    EXPECT_EQ(KeyManager::GetInstance()->LockUserScreen(user), 0);
    EXPECT_NE(KeyManager::GetInstance()->userPinProtect.find(user), KeyManager::GetInstance()->userPinProtect.end());
    EXPECT_EQ(KeyManager::GetInstance()->userPinProtect[user], true);

    EXPECT_NE(KeyManager::GetInstance()->saveLockScreenStatus.find(user),
        KeyManager::GetInstance()->saveLockScreenStatus.end());
    EXPECT_EQ(KeyManager::GetInstance()->saveLockScreenStatus[user], false);
    ForceRemoveDirectory(basePath);
    GTEST_LOG_(INFO) << "KeyManager_LockUserScreen_000 end";
}

/**
 * @tc.name: KeyManager_TryToFixUserCeEceSeceKey_000
 * @tc.desc: Verify the TryToFixUserCeEceSeceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_TryToFixUserCeEceSeceKey_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUserCeEceSeceKey_000 Start";
    uint32_t user = 800;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    auto dir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL2_KEY);
    ForceRemoveDirectory(dir);
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), -EFAULT);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(true));
    KeyManager::GetInstance()->userEl2Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), -EFAULT);
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUserCeEceSeceKey_000 end";
}

/**
 * @tc.name: KeyManager_TryToFixUserCeEceSeceKey_001
 * @tc.desc: Verify the TryToFixUserCeEceSeceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_TryToFixUserCeEceSeceKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUserCeEceSeceKey_001 Start";
    uint32_t user = 800;
    std::vector<uint8_t> token{1, 2, 3, 4, 5};
    std::vector<uint8_t> secretEmpty;
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secretEmpty), E_OK);

    std::vector<uint8_t> secret{1, 2, 3, 4, 5, 6};
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_OK);
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUserCeEceSeceKey_001 end";
}

/**
 * @tc.name: KeyManager_TryToFixUeceKey_000
 * @tc.desc: Verify the TryToFixUeceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_TryToFixUeceKey_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUeceKey_000 Start";
    uint32_t user = 800;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUeceKey(user, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    std::string keyDir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL5_KEY);
    ForceRemoveDirectory(keyDir);
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUeceKey(user, token, secret), -EFAULT);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUeceKey(user, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUeceKey(user, token, secret),  -EFAULT);
    ForceRemoveDirectory(keyDir);
    KeyManager::GetInstance()->userEl5Key_.erase(user);
    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUeceKey_000 end";
}

/**
 * @tc.name: KeyManager_TryToFixUeceKey_001
 * @tc.desc: Verify the TryToFixUeceKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_TryToFixUeceKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUeceKey_001 Start";
    uint32_t user = 800;
    std::vector<uint8_t> token{1, 2, 3, 4, 5};
    std::vector<uint8_t> secretEmpty;
    bool existUece = true;
    if (access(UECE_PATH, F_OK) != 0) {
        existUece = false;
        std::ofstream file(UECE_PATH);
        EXPECT_GT(open(UECE_PATH, O_RDWR), 0);
    }
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUeceKey(user, token, secretEmpty), -EFAULT);

    std::vector<uint8_t> secret{1, 2, 3, 4, 5, 6};
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->TryToFixUeceKey(user, token, secret), -EFAULT);
    if (!existUece) {
        OHOS::RemoveFile(UECE_PATH);
    }
    GTEST_LOG_(INFO) << "KeyManager_TryToFixUeceKey_001 end";
}

/**
 * @tc.name: KeyManager_ActiveElXUserKey_001
 * @tc.desc: Verify the ActiveElXUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_ActiveElXUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey_001 Start";
    unsigned int user = 888;
    const std::vector<uint8_t> token = {};
    const std::vector<uint8_t> secret = {};
    std::shared_ptr<BaseKey> elKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    auto dir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL1_KEY);
    OHOS::ForceRemoveDirectory(dir);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(true));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(true));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_OK);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), -EFAULT);
    GTEST_LOG_(INFO) << "KeyManager_ActiveElXUserKey_001 end";
}

/**
 * @tc.name: KeyManager_UpdateCeEceSeceUserAuth_001
 * @tc.desc: Verify the UpdateCeEceSeceUserAuth function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateCeEceSeceUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_001 Start";
    unsigned int user = 999;
    struct UserTokenSecret userTokenSecret;
    KeyType type = EL1_KEY;
    #ifdef USER_CRYPTO_MIGRATE_KEY
    bool needGenerateShield = true;
    #endif
    auto dir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL1_KEY);
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(false));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield), -EFAULT);
    #else
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type), -EFAULT);
    #endif
    
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(false)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield), E_OK);
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type), E_OK);
    #endif
    
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(false));
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield), -EFAULT);
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type), -EFAULT);
    #endif
    KeyManager::GetInstance()->userEl1Key_.erase(user);
    OHOS::ForceRemoveDirectory(dir);
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_001 end";
}

/**
 * @tc.name: KeyManager_UpdateCeEceSeceUserAuth_002
 * @tc.desc: Verify the UpdateCeEceSeceUserAuth function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateCeEceSeceUserAuth_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_002 Start";
    unsigned int user = 999;
    UserTokenSecret userTokenSecret = {.token = {'t', 'o', 'k', 'e', 'n'}, .oldSecret = {},
        .newSecret = {'s', 'e', 'c', 'r', 'e', 't'}, .secureUid = 0};
    KeyType type = EL1_KEY;
    auto dir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL1_KEY);
    #ifdef USER_CRYPTO_MIGRATE_KEY
    bool needGenerateShield = true;
    #endif
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(true));
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield), E_OK);
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(user, userTokenSecret, type), E_OK);
    #endif
    KeyManager::GetInstance()->userEl1Key_.erase(user);
    OHOS::ForceRemoveDirectory(dir);
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_002 end";
}

/**
 * @tc.name: KeyManager_UpdateUserAuth_001
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateUserAuth_001 Start";
    unsigned int user = 999;
    struct UserTokenSecret userTokenSecret;
    #ifdef USER_CRYPTO_MIGRATE_KEY
    bool needGenerateShield = true;
    #endif
    auto dir = KeyManager::GetInstance()->GetKeyDirByUserAndType(user, EL5_KEY);
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false))
        .WillOnce(Return(false));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret, needGenerateShield), E_OK);
    #else
    EXPECT_EQ(KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret), E_OK);
    #endif

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false))
        .WillOnce(Return(false));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(false));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret, needGenerateShield), -EFAULT);
    #else
    EXPECT_EQ(KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret), -EFAULT);
    #endif
    KeyManager::GetInstance()->userEl5Key_.erase(user);
    OHOS::ForceRemoveDirectory(dir);
    GTEST_LOG_(INFO) << "KeyManager_UpdateUserAuth_001 end";
}

/**
 * @tc.name: KeyManager_UpdateUserAuth_002
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateUserAuth_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateUserAuth_002 Start";
    unsigned int user = 999;
    struct UserTokenSecret userTokenSecret;
    #ifdef USER_CRYPTO_MIGRATE_KEY
    bool needGenerateShield = true;
    #endif
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret, needGenerateShield), -ENOENT);
    #else
    EXPECT_EQ(KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret), -ENOENT);
    #endif

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false))
        .WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret, needGenerateShield), -ENOENT);
    #else
    EXPECT_EQ(KeyManager::GetInstance()->UpdateUserAuth(user, userTokenSecret), -ENOENT);
    #endif
    GTEST_LOG_(INFO) << "KeyManager_UpdateUserAuth_002 end";
}

/**
 * @tc.name: KeyManager_UpdateESecret_001
 * @tc.desc: Verify the KeyManager UpdateESecret function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateESecret_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateESecret_0100 start";
    uint32_t userId = 129;
    UserTokenSecret newTokenSecret = {.token = {'t', 'o', 'k', 'e', 'n'}, .oldSecret = {},
            .newSecret = {'t', 'e', 's', 't'}, .secureUid = 0};
    auto dir = KeyManager::GetInstance()->GetKeyDirByUserAndType(userId, EL5_KEY);
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(false));
    auto ret = KeyManager::GetInstance()->UpdateESecret(userId, newTokenSecret);
    EXPECT_EQ(ret, -EFAULT);

    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(true));
    ret = KeyManager::GetInstance()->UpdateESecret(userId, newTokenSecret);
    EXPECT_EQ(ret, E_OK);
    KeyManager::GetInstance()->userEl5Key_.erase(userId);
    OHOS::ForceRemoveDirectory(dir);
    GTEST_LOG_(INFO) << "KeyManager_UpdateESecret_0100 end";
}
}
