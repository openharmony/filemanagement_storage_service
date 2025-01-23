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
#include "key_control_mock.h"
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v1.h"
#include "recover_manager_mock.h"
#include "storage_service_errno.h"
#include "utils/file_utils.h"

using namespace std;
using namespace testing::ext;
using namespace testing;
 
namespace OHOS::StorageDaemon {
class KeyMgrAnotherTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<RecoveryMgrMock> recoveryMgrMock_ = nullptr;
    static inline shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
    static inline shared_ptr<FscryptControlMoc> fscryptControlMock_ = nullptr;
    bool globalUserEl1PathFlag = true;
    bool deviceEl1DirFlag = true;
};

const std::string GLOBAL_USER_EL1_PATH = std::string(USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);

void KeyMgrAnotherTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    recoveryMgrMock_ = make_shared<RecoveryMgrMock>();
    RecoveryMgrMock::recoveryMgrMock = recoveryMgrMock_;
    baseKeyMock_ = make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
    fscryptControlMock_ = make_shared<FscryptControlMoc>();
    FscryptControlMoc::fscryptControlMoc = fscryptControlMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
}

void KeyMgrAnotherTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    RecoveryMgrMock::recoveryMgrMock = nullptr;
    recoveryMgrMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
    FscryptControlMoc::fscryptControlMoc = nullptr;
    fscryptControlMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
}

void KeyMgrAnotherTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
    if (access(GLOBAL_USER_EL1_PATH.c_str(), F_OK) != 0) {
        globalUserEl1PathFlag = false;
        OHOS::ForceCreateDirectory(GLOBAL_USER_EL1_PATH);
    }

    if (access(DEVICE_EL1_DIR.c_str(), F_OK) != 0) {
        deviceEl1DirFlag = false;
        OHOS::ForceCreateDirectory(DEVICE_EL1_DIR);
    }
}

void KeyMgrAnotherTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    if (!globalUserEl1PathFlag) {
        OHOS::ForceRemoveDirectory(GLOBAL_USER_EL1_PATH);
    }

    if (!deviceEl1DirFlag) {
        OHOS::ForceRemoveDirectory(DEVICE_EL1_DIR);
    }
}

/**
 * @tc.name: KeyManager_CreateRecoverKey_000
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyMgrAnotherTest, KeyManager_CreateRecoverKey_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_CreateRecoverKey_000 Start";
    uint32_t userId = 1111;
    uint32_t userType = 0;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret), -EOPNOTSUPP);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(-1));
    EXPECT_EQ(KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret), -EFAULT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetOriginKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret), -ENOENT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(-1)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetOriginKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret), -ENOENT);
    GTEST_LOG_(INFO) << "KeyManager_CreateRecoverKey_000 end";
}

/**
 * @tc.name: KeyManager_CreateRecoverKey_001
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyMgrAnotherTest, KeyManager_CreateRecoverKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_CreateRecoverKey_001 Start";
    uint32_t userId = 1111;
    uint32_t userType = 0;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetOriginKey(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_EQ(KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret), -ENOENT);
    GTEST_LOG_(INFO) << "KeyManager_CreateRecoverKey_001 end";
}

/**
 * @tc.name: KeyManager_CreateRecoverKey_002
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyMgrAnotherTest, KeyManager_CreateRecoverKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_CreateRecoverKey_001 Start";
    uint32_t userId = 1111;
    uint32_t userType = 0;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    std::string el1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);
    OHOS::ForceCreateDirectory(el1Path);
    OHOS::ForceCreateDirectory(el2Path);
    OHOS::ForceCreateDirectory(el3Path);
    OHOS::ForceCreateDirectory(el4Path);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK))
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetOriginKey(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*recoveryMgrMock_, CreateRecoverKey(_, _, _, _, _)).WillOnce(Return(-EFAULT));
    EXPECT_EQ(KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret), -ENOENT);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2))
        .WillOnce(Return(FSCRYPT_V2)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*baseKeyMock_, RestoreKey(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK))
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetOriginKey(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*recoveryMgrMock_, CreateRecoverKey(_, _, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret), E_OK);
    OHOS::ForceRemoveDirectory(el1Path);
    OHOS::ForceRemoveDirectory(el2Path);
    OHOS::ForceRemoveDirectory(el3Path);
    OHOS::ForceRemoveDirectory(el4Path);
    GTEST_LOG_(INFO) << "KeyManager_CreateRecoverKey_001 end";
}

/**
 * @tc.name: KeyManager_SetRecoverKey_000
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyMgrAnotherTest, KeyManager_SetRecoverKey_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetRecoverKey_000 Start";
    std::vector<uint8_t> key;

    EXPECT_CALL(*recoveryMgrMock_, SetRecoverKey(_)).WillOnce(Return(-EFAULT));
    EXPECT_EQ(KeyManager::GetInstance()->SetRecoverKey(key), -ENOENT);
    GTEST_LOG_(INFO) << "KeyManager_SetRecoverKey_000 end";
}

/**
 * @tc.name: KeyManager_SetRecoverKey_001
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyMgrAnotherTest, KeyManager_SetRecoverKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_SetRecoverKey_001 Start";
    std::vector<uint8_t> key;

    EXPECT_CALL(*recoveryMgrMock_, SetRecoverKey(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManager::GetInstance()->SetRecoverKey(key), E_OK);
    GTEST_LOG_(INFO) << "KeyManager_SetRecoverKey_001 end";
}
}
