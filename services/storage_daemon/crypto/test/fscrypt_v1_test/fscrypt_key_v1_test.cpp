/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "directory_ex.h"
#include "fbex.h"
#include "file_ex.h"
#include "fscrypt_key_v1.h"
#include "../mock/fscrypt_key_v1_ext_mock.h"
#include "key_blob.h"
#include "storage_service_errno.h"

using namespace testing::ext;
using namespace testing;
namespace {
const std::string TEST_MNT = "/data";
const std::string TEST_KEYPATH = "/data/test/key/el2/80";
constexpr uint32_t TEST_KEYID_SIZE = 64;
}

namespace OHOS::StorageDaemon {
class FscryptKeyV1Test : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    UserAuth emptyUserAuth {};
    static inline std::shared_ptr<FscryptKeyV1ExtMock> fscryptKeyExtMock_ = nullptr;
};

void FscryptKeyV1Test::SetUpTestCase(void)
{
    fscryptKeyExtMock_ = std::make_shared<FscryptKeyV1ExtMock>();
    FscryptKeyV1ExtMock::fscryptKeyV1ExtMock = fscryptKeyExtMock_;
}

void FscryptKeyV1Test::TearDownTestCase(void)
{
    FscryptKeyV1ExtMock::fscryptKeyV1ExtMock = nullptr;
    fscryptKeyExtMock_ = nullptr;
}

void FscryptKeyV1Test::SetUp(void)
{
}

void FscryptKeyV1Test::TearDown(void)
{
}

/**
 * @tc.name: fscrypt_key_v1_GenerateAppKeyDesc
 * @tc.desc: Verify the fscrypt V1 GenerateAppKeyDesc.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_GenerateAppKeyDesc, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_GenerateAppKeyDesc start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    KeyBlob appKey;
    EXPECT_FALSE(g_testKeyV1.GenerateAppKeyDesc(appKey));

    KeyBlob appKey1(FBEX_KEYID_SIZE);
    EXPECT_TRUE(g_testKeyV1.GenerateAppKeyDesc(appKey1));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_GenerateAppKeyDesc end";
}

/**
 * @tc.name: fscrypt_key_v1_LockUece
 * @tc.desc: Verify the fscrypt V1 LockUece.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_LockUece, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_LockUece start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    bool isFbeSupport = true;
    EXPECT_CALL(*fscryptKeyExtMock_, LockUeceExt(_)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.LockUece(isFbeSupport));

    EXPECT_CALL(*fscryptKeyExtMock_, LockUeceExt(_)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.LockUece(isFbeSupport));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_LockUece end";
}

/**
 * @tc.name: fscrypt_key_v1_ChangePinCodeClassE
 * @tc.desc: Verify the fscrypt V1 ChangePinCodeClassE.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_ChangePinCodeClassE, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_ChangePinCodeClassE start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    bool isFbeSupport = true;
    uint32_t userId = 100;
    EXPECT_CALL(*fscryptKeyExtMock_, ChangePinCodeClassE(_, _)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.ChangePinCodeClassE(isFbeSupport, userId));

    EXPECT_CALL(*fscryptKeyExtMock_, ChangePinCodeClassE(_, _)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.ChangePinCodeClassE(isFbeSupport, userId));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_ChangePinCodeClassE end";
}

/**
 * @tc.name: fscrypt_key_v1_DeleteClassEPinCode
 * @tc.desc: Verify the fscrypt V1 DeleteClassEPinCode.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_DeleteClassEPinCode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DeleteClassEPinCode start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    uint32_t userId = 100;
    EXPECT_CALL(*fscryptKeyExtMock_, DeleteClassEPinCode(_)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.DeleteClassEPinCode(userId));

    EXPECT_CALL(*fscryptKeyExtMock_, DeleteClassEPinCode(_)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.DeleteClassEPinCode(userId));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DeleteClassEPinCode end";
}

/**
 * @tc.name: fscrypt_key_v1_AddClassE
 * @tc.desc: Verify the fscrypt V1 AddClassE.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_AddClassE, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_AddClassE start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    uint32_t status = 100;
    bool isSupport = true;
    bool isNeedEncryptClassE = true;

    EXPECT_CALL(*fscryptKeyExtMock_, AddClassE(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.AddClassE(isNeedEncryptClassE, isSupport, status));

    EXPECT_CALL(*fscryptKeyExtMock_, AddClassE(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.AddClassE(isNeedEncryptClassE, isSupport, status));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_AddClassE end";
}

/**
 * @tc.name: fscrypt_key_v1_DeleteAppkey
 * @tc.desc: Verify the fscrypt V1 DeleteAppkey.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_DeleteAppkey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DeleteAppkey start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    std::string KeyId = "";
    EXPECT_FALSE(g_testKeyV1.DeleteAppkey(KeyId));

    KeyId = "test";
    EXPECT_FALSE(g_testKeyV1.DeleteAppkey(KeyId));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DeleteAppkey end";
}

/**
 * @tc.name: fscrypt_key_v1_UninstallKeyForAppKeyToKeyring
 * @tc.desc: Verify the fscrypt V1 UninstallKeyForAppKeyToKeyring.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_UninstallKeyForAppKeyToKeyring, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_UninstallKeyForAppKeyToKeyring start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    std::string keyId = "";
    EXPECT_FALSE(g_testKeyV1.UninstallKeyForAppKeyToKeyring(keyId));

    keyId = "test";
    EXPECT_FALSE(g_testKeyV1.UninstallKeyForAppKeyToKeyring(keyId));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_UninstallKeyForAppKeyToKeyring end";
}

/**
 * @tc.name: fscrypt_key_v1_InstallKeyForAppKeyToKeyring
 * @tc.desc: Verify the fscrypt V1 InstallKeyForAppKeyToKeyring.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_InstallKeyForAppKeyToKeyring, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_InstallKeyForAppKeyToKeyring start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    uint32_t *appKeyraw = new uint32_t[17];
    EXPECT_FALSE(g_testKeyV1.InstallKeyForAppKeyToKeyring(appKeyraw));
    delete[] appKeyraw;
    appKeyraw = nullptr;
    
    uint32_t *appKey = new uint32_t[10];
    EXPECT_FALSE(g_testKeyV1.InstallKeyForAppKeyToKeyring(appKey));
    delete[] appKey;
    appKey = nullptr;
    GTEST_LOG_(INFO) << "fscrypt_key_v1_InstallKeyForAppKeyToKeyring end";
}

/**
 * @tc.name: fscrypt_key_v1_UnlockUserScreen
 * @tc.desc: Verify the fscrypt V1 UnlockUserScreen.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_UnlockUserScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_UnlockUserScreen start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    uint32_t flag = 1;
    uint32_t sdpClass = 1;
    const std::string mnt = "test";
    
    EXPECT_FALSE(g_testKeyV1.UnlockUserScreen(flag, sdpClass, mnt));

    g_testKeyV1.keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*fscryptKeyExtMock_, UnlockUserScreenExt(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.UnlockUserScreen(flag, sdpClass, mnt));

    g_testKeyV1.ClearKey();
    g_testKeyV1.keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*fscryptKeyExtMock_, UnlockUserScreenExt(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.UnlockUserScreen(flag, sdpClass, mnt));

    sdpClass = 2;
    EXPECT_CALL(*fscryptKeyExtMock_, UnlockUserScreenExt(_, _, _)).WillOnce(Return(true));
    EXPECT_FALSE(g_testKeyV1.UnlockUserScreen(flag, sdpClass, mnt));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_UnlockUserScreen end";
}

/**
 * @tc.name: fscrypt_key_v1_GenerateAppkey
 * @tc.desc: Verify the fscrypt V1 GenerateAppkey.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_GenerateAppkey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_GenerateAppkey start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    uint32_t userId = 100;
    uint32_t appUid = 2;
    std::string keyDesc = "test";
    
    EXPECT_CALL(*fscryptKeyExtMock_, GenerateAppkey(_, _, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.GenerateAppkey(userId, appUid, keyDesc));

    EXPECT_CALL(*fscryptKeyExtMock_, GenerateAppkey(_, _, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.GenerateAppkey(userId, appUid, keyDesc));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_GenerateAppkey end";
}

/**
 * @tc.name: fscrypt_key_v1_LockUserScreen
 * @tc.desc: Verify the fscrypt V1 LockUserScreen.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_LockUserScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_LockUserScreen start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    uint32_t flag = 1;
    uint32_t sdpClass = 2;
    
    uint32_t elType = TYPE_EL4;
    EXPECT_CALL(*fscryptKeyExtMock_, SetElType()).WillOnce(Return(elType));
    EXPECT_CALL(*fscryptKeyExtMock_, LockUserScreenExt(_, _)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.LockUserScreen(flag, sdpClass, TEST_MNT));

    elType = TYPE_EL4;
    g_testKeyV1.keyInfo_.keyDesc.Clear();
    EXPECT_CALL(*fscryptKeyExtMock_, SetElType()).WillOnce(Return(elType));
    EXPECT_CALL(*fscryptKeyExtMock_, LockUserScreenExt(_, _)).WillOnce(Return(true));
    EXPECT_FALSE(g_testKeyV1.LockUserScreen(flag, sdpClass, TEST_MNT));

    elType = TYPE_EL1;
    EXPECT_CALL(*fscryptKeyExtMock_, SetElType()).WillOnce(Return(elType));
    EXPECT_CALL(*fscryptKeyExtMock_, LockUserScreenExt(_, _)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.LockUserScreen(flag, sdpClass, TEST_MNT));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_LockUserScreen end";
}

/**
 * @tc.name: fscrypt_key_v1_InstallEceSeceKeyToKeyring
 * @tc.desc: Verify the fscrypt V1 InstallEceSeceKeyToKeyring.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_InstallEceSeceKeyToKeyring, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_InstallEceSeceKeyToKeyring start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    uint32_t sdpClass = 2;
    EXPECT_FALSE(g_testKeyV1.InstallEceSeceKeyToKeyring(sdpClass));

    g_testKeyV1.keyInfo_.key.Clear();
    g_testKeyV1.keyInfo_.keyDesc.Clear();
    g_testKeyV1.keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    EXPECT_FALSE(g_testKeyV1.InstallEceSeceKeyToKeyring(sdpClass));

    g_testKeyV1.keyInfo_.key.Clear();
    g_testKeyV1.keyInfo_.keyDesc.Clear();
    g_testKeyV1.keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    g_testKeyV1.keyInfo_.keyDesc.Alloc(TEST_KEYID_SIZE);
    EXPECT_FALSE(g_testKeyV1.InstallEceSeceKeyToKeyring(sdpClass));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_InstallEceSeceKeyToKeyring end";
}

/**
 * @tc.name: fscrypt_key_v1_DecryptClassE
 * @tc.desc: Verify the fscrypt V1 DecryptClassE.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_DecryptClassE, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DecryptClassE start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    bool isSupport = true;
    uint32_t user = 1;
    uint32_t status = 1;

    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.DecryptClassE(emptyUserAuth, isSupport, user, status));

    g_testKeyV1.ClearKey();
    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(g_testKeyV1.DecryptClassE(emptyUserAuth, isSupport, user, status));

    g_testKeyV1.ClearKey();
    const UserAuth auth{KeyBlob(8), KeyBlob(8), 0};
    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _, _)).WillOnce(Return(true));
    EXPECT_FALSE(g_testKeyV1.DecryptClassE(auth, isSupport, user, status));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DecryptClassE end";
}

/**
 * @tc.name: fscrypt_key_v1_EncryptClassE
 * @tc.desc: Verify the fscrypt V1 EncryptClassE.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_EncryptClassE, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_EncryptClassE start";
    OHOS::StorageDaemon::FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
    bool isSupport = true;
    uint32_t user = 1;
    uint32_t status = 1;

    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(g_testKeyV1.EncryptClassE(emptyUserAuth, isSupport, user, status));
    GTEST_LOG_(INFO) << "fscrypt_key_v1_EncryptClassE end";
}
} // OHOS::StorageDaemon