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
1
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "base_key_mock.h"
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v2_mock.h"
#include "fscrypt_key_v2.h"
#include "key_control_mock.h"

using namespace std;
using namespace testing::ext;
using namespace testing;
 
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
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->SaveUserElKey(user, EL1_KEY, elKey);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);
    KeyManager::GetInstance()->userEl1Key_.erase(user);

    KeyManager::GetInstance()->SaveUserElKey(user, static_cast<KeyType>(0), elKey);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, static_cast<KeyType>(0)), nullptr);
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
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);
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
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->SaveUserElKey(user, EL3_KEY, elKey);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);
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
    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->SaveUserElKey(user, EL4_KEY, elKey);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);
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
    KeyManager::GetInstance()->SaveUserElKey(user, EL5_KEY, elKey);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL1_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL2_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL3_KEY), nullptr);
    EXPECT_EQ(KeyManager::GetInstance()->GetUserElKey(user, EL4_KEY), nullptr);
    EXPECT_NE(KeyManager::GetInstance()->GetUserElKey(user, EL5_KEY), nullptr);
    KeyManager::GetInstance()->userEl5Key_.erase(user);
    GTEST_LOG_(INFO) << "KeyManager_SaveUserElKey_005 end";
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
 * @tc.name: KeyManager_GenerateAppkey_001
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_GenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_GenerateAppkey_001 Start";
    unsigned int user = 800;
    string keyId;

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), -EFAULT);

    EXPECT_CALL(*fscryptKeyMock_, GenerateAppkey(_, _, _)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), 0);

    KeyManager::GetInstance()->userEl2Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->GenerateAppkey(user, 100, keyId), -ENOENT);
    GTEST_LOG_(INFO) << "KeyManager_GenerateAppkey_001 end";
}

/**
 * @tc.name: KeyManager_DeleteAppkey_001
 * @tc.desc: Verify the DeleteAppkey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyManagerTest, KeyManager_DeleteAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_DeleteAppkey_001 Start";
    unsigned int user = 800;
    string keyId;

    shared_ptr<FscryptKeyV2> elKey = make_shared<FscryptKeyV2>("/data/test");
    KeyManager::GetInstance()->SaveUserElKey(user, EL2_KEY, elKey);
    EXPECT_CALL(*fscryptKeyMock_, DeleteAppkey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->DeleteAppkey(user, keyId), -EFAULT);

    EXPECT_CALL(*fscryptKeyMock_, DeleteAppkey(_)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->DeleteAppkey(user, keyId), 0);

    KeyManager::GetInstance()->userEl2Key_.erase(user);
    EXPECT_EQ(KeyManager::GetInstance()->DeleteAppkey(user, keyId), -ENOENT);
    GTEST_LOG_(INFO) << "KeyManager_DeleteAppkey_001 end";
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
    KeyManager::GetInstance()->userLockScreenTask_[user] = std::make_shared<DelayHandler>(user);
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), 0);
    EXPECT_TRUE(KeyManager::GetInstance()->userEl2Key_.find(user) == KeyManager::GetInstance()->userEl2Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userEl3Key_.find(user) == KeyManager::GetInstance()->userEl3Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userEl4Key_.find(user) == KeyManager::GetInstance()->userEl4Key_.end());
    EXPECT_TRUE(KeyManager::GetInstance()->userLockScreenTask_.find(user) ==
        KeyManager::GetInstance()->userLockScreenTask_.end());

    KeyManager::GetInstance()->userEl2Key_[user] = elKey;
    KeyManager::GetInstance()->userEl3Key_[user] = elKey;
    KeyManager::GetInstance()->userEl4Key_[user] = elKey;
    EXPECT_CALL(*fscryptKeyMock_, InactiveKey(_, _)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->InActiveUserKey(user), 0);
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
}
