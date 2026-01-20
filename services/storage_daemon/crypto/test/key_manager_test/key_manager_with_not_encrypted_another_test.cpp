/*
 * Copyright (C) 2025-2025 Huawei Device Co., Ltd.
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
#include "file_utils_mock.h"
#include "key_control_mock.h"
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v1.h"
#include "fscrypt_key_v2_mock.h"
#include "recover_manager_mock.h"
#include "storage_service_errno.h"
#include "utils/file_utils.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS::StorageDaemon {
class KeyMgrWithNotEncryptedAnotherTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<RecoveryMgrMock> recoveryMgrMock_ = nullptr;
    static inline shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
    static inline shared_ptr<FscryptControlMoc> fscryptControlMock_ = nullptr;
    static inline shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline shared_ptr<FscryptKeyV2Moc> fscryptKeyMock_ = nullptr;
};

void KeyMgrWithNotEncryptedAnotherTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void KeyMgrWithNotEncryptedAnotherTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void KeyMgrWithNotEncryptedAnotherTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
    recoveryMgrMock_ = make_shared<RecoveryMgrMock>();
    RecoveryMgrMock::recoveryMgrMock = recoveryMgrMock_;
    baseKeyMock_ = make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
    fscryptControlMock_ = make_shared<FscryptControlMoc>();
    FscryptControlMoc::fscryptControlMoc = fscryptControlMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
    fileUtilMoc_ = make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    fscryptKeyMock_ = make_shared<FscryptKeyV2Moc>();
    FscryptKeyV2Moc::fscryptKeyV2Moc = fscryptKeyMock_;
}

void KeyMgrWithNotEncryptedAnotherTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    RecoveryMgrMock::recoveryMgrMock = nullptr;
    recoveryMgrMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
    FscryptControlMoc::fscryptControlMoc = nullptr;
    fscryptControlMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    FscryptKeyV2Moc::fscryptKeyV2Moc = nullptr;
    fscryptKeyMock_ = nullptr;
}

/**
 * @tc.name: KeyManager_ResetSecretWithRecoveryKey_NotEncrypted_000
 * @tc.desc: Verify the ResetSecretWithRecoveryKey function with not encrypted.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyMgrWithNotEncryptedAnotherTest, KeyManager_ResetSecretWithRecoveryKey_NotEncrypted_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManager_ResetSecretWithRecoveryKey_NotEncrypted_000 Start";
    uint32_t userId = 1112;
    uint32_t rkType = 6;
    std::vector<uint8_t> key;
    std::string globalUserEl1Path = std::string(MAINTAIN_USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
    std::string el1Path = std::string(MAINTAIN_USER_EL1_DIR) + "/" + std::to_string(userId);
    std::string el2Path = std::string(MAINTAIN_USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(MAINTAIN_USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(MAINTAIN_USER_EL4_DIR) + "/" + std::to_string(userId);
    OHOS::ForceCreateDirectory(MAINTAIN_DEVICE_EL1_DIR);
    OHOS::ForceCreateDirectory(globalUserEl1Path);
    OHOS::ForceCreateDirectory(el1Path);
    OHOS::ForceCreateDirectory(el2Path);
    OHOS::ForceCreateDirectory(el3Path);
    OHOS::ForceCreateDirectory(el4Path);
    
    EXPECT_EQ(KeyManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key),
              E_GLOBAL_KEY_NULLPTR);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_V2));
    EXPECT_CALL(*fileUtilMoc_, UMount(_)).WillOnce(Return(1));
    userId = 300;
    errno = 0;
    EXPECT_EQ(KeyManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key), E_UMOUNT_FBE);

    EXPECT_CALL(*fscryptControlMock_, GetFscryptVersionFromPolicy()).WillRepeatedly(Return(FSCRYPT_V2));
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillRepeatedly(Return(FSCRYPT_V2));
    EXPECT_CALL(*fileUtilMoc_, UMount(_)).WillOnce(Return(0));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*baseKeyMock_, InitKey(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*baseKeyMock_, StoreKey(_)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*fscryptKeyMock_, ActiveKey(_, _, _)).WillRepeatedly(Return(E_OK));
    userId = 300;
    errno = 0;
    EXPECT_EQ(KeyManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key), E_OK);

    OHOS::ForceRemoveDirectory(MAINTAIN_DEVICE_EL1_DIR);
    OHOS::ForceRemoveDirectory(globalUserEl1Path);
    OHOS::ForceRemoveDirectory(el1Path);
    OHOS::ForceRemoveDirectory(el2Path);
    OHOS::ForceRemoveDirectory(el3Path);
    OHOS::ForceRemoveDirectory(el4Path);
    GTEST_LOG_(INFO) << "KeyManager_ResetSecretWithRecoveryKey_NotEncrypted_000 end";
}

/**
 * @tc.name: KeyMgrWithNotEncryptedAnotherTest_SetDirectoryElPolicy_001
 * @tc.desc: Verify the KeyManager SetDirectoryElPolicy function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(KeyMgrWithNotEncryptedAnotherTest, KeyMgrWithNotEncryptedAnotherTest_SetDirectoryElPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyMgrWithNotEncryptedAnotherTest_SetDirectoryElPolicy_001 start";
    uint32_t userId = 124;
    KeyType type = EL1_KEY;
    std::vector<FileList> vec;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    auto ret = KeyManager::GetInstance().SetDirectoryElPolicy(userId, type, vec);
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    ret = KeyManager::GetInstance().SetDirectoryElPolicy(userId, type, vec);
    EXPECT_EQ(ret, 0);
    
    GTEST_LOG_(INFO) << "KeyMgrWithNotEncryptedAnotherTest_SetDirectoryElPolicy_001 end";
}
}
