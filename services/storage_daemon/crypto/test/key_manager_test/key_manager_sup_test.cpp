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
    EXPECT_CALL(*mountManagerMoc_, CheckMountFileByUser(_)).WillOnce(Return(true));
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
}
