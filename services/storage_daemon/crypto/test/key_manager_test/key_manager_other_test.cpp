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
constexpr uint32_t FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG = 0xFBE30034;
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
}

void KeyManagerOtherTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void KeyManagerOtherTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
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

void KeyManagerOtherTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
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

/**
 * @tc.name: KeyManager_LoadAllUsersEl1Key_000
 * @tc.desc: Verify the LoadAllUsersEl1Key function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_LoadAllUsersEl1Key_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_LoadAllUsersEl1Key_000 Start";
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillRepeatedly(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillRepeatedly(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, UpgradeKeys()).WillRepeatedly(Return(true));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().LoadAllUsersEl1Key(), E_OK);
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
    EXPECT_EQ(KeyManager::GetInstance().LockUserScreen(user), 0);

    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().LockUserScreen(user), 0);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().LockUserScreen(user), 0);
    EXPECT_NE(KeyManager::GetInstance().userPinProtect.find(user), KeyManager::GetInstance().userPinProtect.end());
    EXPECT_EQ(KeyManager::GetInstance().userPinProtect[user], true);

    EXPECT_NE(KeyManager::GetInstance().saveLockScreenStatus.find(user),
        KeyManager::GetInstance().saveLockScreenStatus.end());
    EXPECT_EQ(KeyManager::GetInstance().saveLockScreenStatus[user], false);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().LockUserScreen(user), E_NON_EXIST);

    KeyManager::GetInstance().userPinProtect[user] = false;
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().LockUserScreen(user), 0);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().LockUserScreen(user), 0);
    EXPECT_NE(KeyManager::GetInstance().userPinProtect.find(user), KeyManager::GetInstance().userPinProtect.end());
    EXPECT_EQ(KeyManager::GetInstance().userPinProtect[user], true);

    EXPECT_NE(KeyManager::GetInstance().saveLockScreenStatus.find(user),
        KeyManager::GetInstance().saveLockScreenStatus.end());
    EXPECT_EQ(KeyManager::GetInstance().saveLockScreenStatus[user], false);
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
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    auto dir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL2_KEY);
    ForceRemoveDirectory(dir);
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(true));
    KeyManager::GetInstance().DeleteElKey(user, EL2_KEY);
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_PARAMS_INVALID);
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
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secretEmpty), E_OK);

    std::vector<uint8_t> secret{1, 2, 3, 4, 5, 6};
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUserCeEceSeceKey(user, EL2_KEY, token, secret), E_OK);
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
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUeceKey(user, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    std::string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL5_KEY);
    ForceRemoveDirectory(keyDir);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUeceKey(user, token, secret), E_PARAMS_NULLPTR_ERR);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUeceKey(user, token, secret), E_OK);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUeceKey(user, token, secret), E_ELX_KEY_UPDATE_ERROR);
    ForceRemoveDirectory(keyDir);
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
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
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUeceKey(user, token, secretEmpty), E_PARAMS_NULLPTR_ERR);

    std::vector<uint8_t> secret{1, 2, 3, 4, 5, 6};
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID))
        .WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().TryToFixUeceKey(user, token, secret), E_PARAMS_NULLPTR_ERR);
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
    auto dir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY);
    OHOS::ForceRemoveDirectory(dir);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(E_OK));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    #endif
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_OK);

    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(E_OK));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(
        KeyManager::GetInstance().ActiveElXUserKey(user, token, EL1_KEY, secret, elKey), E_TRY_TO_FIX_USER_KEY_ERR);
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
    auto dir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY);
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(-1)).WillOnce(Return(-1));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield),
        E_RESTORE_KEY_FAILED);
    #else
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type), -EFAULT);
    #endif
    
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(
        KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield), E_OK);
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type), E_OK);
    #endif
    
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield),
        E_ELX_KEY_STORE_ERROR);
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type), E_ELX_KEY_STORE_ERROR);
    #endif

    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
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
    auto dir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY);
    #ifdef USER_CRYPTO_MIGRATE_KEY
    bool needGenerateShield = true;
    #endif
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(
        KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield), E_OK);
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type), E_OK);
    #endif
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    OHOS::ForceRemoveDirectory(dir);
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_002 end";
}

/**
 * @tc.name: KeyManager_UpdateCeEceSeceUserAuth_003
 * @tc.desc: Verify the UpdateCeEceSeceUserAuth function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateCeEceSeceUserAuth_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_003 Start";
    unsigned int user = 999;
    struct UserTokenSecret userTokenSecret;
    KeyType type = EL1_KEY;
    #ifdef USER_CRYPTO_MIGRATE_KEY
    bool needGenerateShield = true;
    #endif
    auto dir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL1_KEY);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    OHOS::ForceCreateDirectory(dir);
 
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(-1)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*baseKeyMock_, StoreKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(
        KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type, needGenerateShield), E_OK);
    #else
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceUserAuth(user, userTokenSecret, type), E_OK);
    #endif
 
    KeyManager::GetInstance().DeleteElKey(user, EL1_KEY);
    OHOS::ForceRemoveDirectory(dir);
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceUserAuth_003 end";
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
    auto dir = KeyManager::GetInstance().GetKeyDirByUserAndType(user, EL5_KEY);
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true)).WillOnce(Return(false))
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(E_OK));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret, needGenerateShield), E_OK);
    #else
    EXPECT_EQ(KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret), E_OK);
    #endif

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, DeleteClassEPinCode(_)).WillOnce(Return(-1));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret, needGenerateShield), E_EL5_DELETE_CLASS_ERROR);
    #else
    EXPECT_EQ(KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret), E_EL5_DELETE_CLASS_ERROR);
    #endif
    KeyManager::GetInstance().DeleteElKey(user, EL5_KEY);
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
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false))
        .WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret, needGenerateShield), E_PARAMS_NULLPTR_ERR);
    #else
    EXPECT_EQ(KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret), E_PARAMS_NULLPTR_ERR);
    #endif

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false)).WillOnce(Return(false))
        .WillOnce(Return(false)).WillOnce(Return(true));
    #ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_EQ(
        KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret, needGenerateShield), E_PARAMS_NULLPTR_ERR);
    #else
    EXPECT_EQ(KeyManager::GetInstance().UpdateUserAuth(user, userTokenSecret), E_PARAMS_NULLPTR_ERR);
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
    auto dir = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, EL5_KEY);
    OHOS::ForceCreateDirectory(dir);
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    auto ret = KeyManager::GetInstance().UpdateESecret(userId, newTokenSecret);
    EXPECT_EQ(ret, E_EL5_ENCRYPT_CLASS_ERROR);

    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    ret = KeyManager::GetInstance().UpdateESecret(userId, newTokenSecret);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    ret = KeyManager::GetInstance().UpdateESecret(userId, newTokenSecret);
    EXPECT_EQ(ret, E_OK);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);
    OHOS::ForceRemoveDirectory(dir);
    GTEST_LOG_(INFO) << "KeyManager_UpdateESecret_0100 end";
}

/**
 * @tc.name: KeyManager_UpdateClassEBackUpFix_001
 * @tc.desc: Verify the KeyManager UpdateClassEBackUpFix function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateClassEBackUpFix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateClassEBackUpFix_0100 start";
    uint32_t userId = 100;
    EXPECT_EQ(KeyManager::GetInstance().UpdateClassEBackUpFix(userId), E_NON_EXIST);
    GTEST_LOG_(INFO) << "KeyManager_UpdateClassEBackUpFix_0100 end";
}

/**
 * @tc.name: KeyManager_UpdateClassEBackUp_001
 * @tc.desc: Verify the KeyManager UpdateClassEBackUp function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateClassEBackUp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateClassEBackUp_0100 start";
    uint32_t userId = 100;
    EXPECT_EQ(KeyManager::GetInstance().UpdateClassEBackUp(userId), E_NON_EXIST);
    GTEST_LOG_(INFO) << "KeyManager_UpdateClassEBackUp_0100 end";
}

/**
 * @tc.name: KeyManager_UpdateCeEceSeceKeyContext
 * @tc.desc: Verify the UpdateCeEceSeceKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateCeEceSeceKeyContext, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceKeyContext Start";
    uint32_t userId = 1;
    KeyType type = EL1_KEY;

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceKeyContext(userId, type), 0);

    KeyManager::GetInstance().DeleteElKey(userId, EL1_KEY);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceKeyContext(userId, type), E_PARAMS_INVALID);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(userId, EL1_KEY, tmpKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceKeyContext(userId, type), E_ELX_KEY_UPDATE_ERROR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().UpdateCeEceSeceKeyContext(userId, type), 0);
    GTEST_LOG_(INFO) << "KeyManager_UpdateCeEceSeceKeyContext end";
}

/**
 * @tc.name: KeyManager_UpdateKeyContext
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateKeyContext, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext Start";
    uint32_t userId = 1;
    EXPECT_TRUE(OHOS::RemoveFile(UECE_PATH));
    KeyManager::GetInstance().DeleteElKey(userId, EL2_KEY);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), E_PARAMS_INVALID);

    KeyManager::GetInstance().DeleteElKey(userId, EL3_KEY);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(2).WillOnce(Return(false))\
        .WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), E_PARAMS_INVALID);

    KeyManager::GetInstance().DeleteElKey(userId, EL4_KEY);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), E_PARAMS_INVALID);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), 0);
    std::ofstream file(UECE_PATH);
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContextt end";
}

/**
 * @tc.name: KeyManager_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext function, uece not supported.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_001 Start";
    EXPECT_TRUE(OHOS::RemoveFile(UECE_PATH));
    uint32_t userId = 100;
    KeyManager::GetInstance().saveESecretStatus[userId] = false;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
           .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_FALSE(KeyManager::GetInstance().IsUeceSupport());
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), 0); //uece support failed at fileOpen
    std::ofstream file(UECE_PATH);
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_001 End";
}

/**
 * @tc.name: KeyManager_UpdateKeyContext_002
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateKeyContext_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_002 Start";

    uint32_t userId = 100;
    std::ofstream file(UECE_PATH);
    EXPECT_TRUE(KeyManager::GetInstance().IsUeceSupport());
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, EL5_KEY);
    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fscryptKeyMock_, UpdateClassEBackUp(_)).WillOnce(Return(-1));
    EXPECT_NE(KeyManager::GetInstance().UpdateKeyContext(userId), 0); //updateClassEbackUp failed

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(3).WillOnce(Return(false))\
           .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fscryptKeyMock_, UpdateClassEBackUp(_)).WillOnce(Return(0));
    KeyManager::GetInstance().saveESecretStatus[userId] = false;
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), 0); //do not update el5 context

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
           .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fscryptKeyMock_, UpdateClassEBackUp(_)).WillOnce(Return(0));
    KeyManager::GetInstance().saveESecretStatus[userId] = true;
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), 0); //update el5 context SUCC

    EXPECT_TRUE(OHOS::RemoveFile(UECE_PATH));
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_002 End";
}

/**
 * @tc.name: KeyManager_UpdateKeyContext_003
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_UpdateKeyContext_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_003 Start";

    std::ofstream file(UECE_PATH);
    EXPECT_TRUE(KeyManager::GetInstance().IsUeceSupport());

    uint32_t userId = 100;
    string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, EL5_KEY);
    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
        .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, UpdateClassEBackUp(_)).WillOnce(Return(0));
    KeyManager::GetInstance().saveESecretStatus[userId] = true;
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), E_ELX_KEY_UPDATE_ERROR);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);

    userId = 240;
    keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, EL5_KEY);
    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
           .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, UpdateClassEBackUp(_)).WillOnce(Return(0));
    KeyManager::GetInstance().saveESecretStatus[userId] = true;
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), E_ELX_KEY_UPDATE_ERROR);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);

    userId = 219;
    keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, EL5_KEY);
    ASSERT_TRUE(OHOS::ForceCreateDirectory(keyDir));
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).Times(4).WillOnce(Return(false))\
           .WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*fscryptKeyMock_, UpdateClassEBackUp(_)).WillOnce(Return(0));
    KeyManager::GetInstance().saveESecretStatus[userId] = true;
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(-1));
    EXPECT_NE(KeyManager::GetInstance().GetUserElKey(userId, EL5_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance().UpdateKeyContext(userId), 0);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(keyDir));
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);
    EXPECT_TRUE(OHOS::RemoveFile(UECE_PATH));
    GTEST_LOG_(INFO) << "KeyManager_UpdateKeyContext_003 End";
}

/**
 * @tc.name: KeyManager_CheckUserPinProtect_001
 * @tc.desc: Verify the CheckUserPinProtect function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_CheckUserPinProtect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_CheckUserPinProtect_001 Start";
    unsigned int userId = 999;
    std::vector<uint8_t> tokenEmpty;
    std::vector<uint8_t> secretEmpty;

    std::string basePath = std::string(USER_EL2_DIR) + "/" + std::to_string(userId) + "/latest";
    OHOS::ForceRemoveDirectory(basePath);
    EXPECT_EQ(KeyManager::GetInstance().CheckUserPinProtect(userId, tokenEmpty, secretEmpty), E_OK);

    ASSERT_TRUE(OHOS::ForceCreateDirectory(basePath));
    std::string restorePath = basePath + SUFFIX_NEED_RESTORE;
    auto fd = open(restorePath.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);
    close(fd);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().CheckUserPinProtect(userId, tokenEmpty, secretEmpty), E_ERR);

    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().CheckUserPinProtect(userId, tokenEmpty, secretEmpty), E_OK);

    std::vector<uint8_t> secret{ 1, 2, 3, 4, 5 };
    EXPECT_EQ(KeyManager::GetInstance().CheckUserPinProtect(userId, tokenEmpty, secret), E_OK);

    std::vector<uint8_t> token{ 1, 2, 3, 4, 5 };
    EXPECT_EQ(KeyManager::GetInstance().CheckUserPinProtect(userId, token, secretEmpty), E_OK);
    ASSERT_EQ(remove(restorePath.c_str()), 0);
    OHOS::ForceRemoveDirectory(restorePath);
    GTEST_LOG_(INFO) << "KeyManager_CheckUserPinProtect_001 end";
}

/**
 * @tc.name: KeyManager_ActiveUeceUserKey_001
 * @tc.desc: Verify the ActiveUeceUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_ActiveUeceUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveUeceUserKey_001 Start";
    unsigned int userId = 999;
    std::vector<uint8_t> tokenEmpty;
    std::vector<uint8_t> secretEmpty;
    std::shared_ptr<BaseKey> elKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));

    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(
        KeyManager::GetInstance().ActiveUeceUserKey(userId, tokenEmpty, secretEmpty, elKey), E_EL5_DELETE_CLASS_ERROR);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);

    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveUeceUserKey(userId, tokenEmpty, secretEmpty, elKey), 0);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);

    std::vector<uint8_t> token{ 1, 2, 3, 4, 5 };
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveUeceUserKey(userId, token, secretEmpty, elKey), 0);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);

    std::vector<uint8_t> secret{ 1, 2, 3, 4, 5 };
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveUeceUserKey(userId, token, secret, elKey), 0);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);
    GTEST_LOG_(INFO) << "KeyManager_ActiveUeceUserKey_001 end";
}

/**
 * @tc.name: KeyManager_ActiveUeceUserKey_002
 * @tc.desc: Verify the ActiveUeceUserKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_ActiveUeceUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveUeceUserKey_002 Start";
    unsigned int userId = 777;
    std::vector<uint8_t> tokenEmpty;
    std::vector<uint8_t> secretEmpty;
    std::shared_ptr<BaseKey> elKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));

    std::vector<uint8_t> token{ 1, 2, 3, 4, 5 };
    std::vector<uint8_t> secret{ 1, 2, 3, 4, 5 };
    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(true), Return(E_OK)));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().ActiveUeceUserKey(userId, token, secret, elKey), 0);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);

    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(true), Return(E_OK)));
    EXPECT_CALL(*iamClientMoc_, HasPinProtect(_)).WillOnce(Return(true));
    EXPECT_CALL(*iamClientMoc_, GetSecureUid(_, _)).WillOnce(Return(true));
    std::string keyDir = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, EL5_KEY);
    OHOS::ForceCreateDirectory(keyDir);
    EXPECT_CALL(*fscryptKeyMock_, EncryptClassE(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance().ActiveUeceUserKey(userId, token, secret, elKey), E_TRY_TO_FIX_USER_KEY_ERR);
    OHOS::ForceRemoveDirectory(keyDir);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);
    GTEST_LOG_(INFO) << "KeyManager_ActiveUeceUserKey_002 end";
}

/**
 * @tc.name: KeyManager_ActiveUece_001
 * @tc.desc: Verify the ActiveUece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerOtherTest, KeyManager_ActiveUece_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ActiveUece_001 Start";
    unsigned int userId = 999;
    std::vector<uint8_t> tokenEmpty;
    std::vector<uint8_t> secretEmpty;
    std::shared_ptr<BaseKey> elKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));

    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance().ActiveUece(userId, elKey, tokenEmpty, secretEmpty), E_ELX_KEY_ACTIVE_ERROR);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);
    KeyManager::GetInstance().saveLockScreenStatus.erase(userId);

    EXPECT_CALL(*fscryptKeyMock_, DecryptClassE(_, _, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance().ActiveUece(userId, elKey, tokenEmpty, secretEmpty), 0);
    KeyManager::GetInstance().DeleteElKey(userId, EL5_KEY);
    KeyManager::GetInstance().saveLockScreenStatus.erase(userId);
    GTEST_LOG_(INFO) << "KeyManager_ActiveUece_001 end";
}
}
