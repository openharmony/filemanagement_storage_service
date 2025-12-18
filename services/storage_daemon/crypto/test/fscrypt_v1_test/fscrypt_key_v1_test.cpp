/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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

#include "base_key_mock.h"
#include "directory_ex.h"
#include "fbex.h"
#include "file_ex.h"
#include "fscrypt_key_v1.h"
#include "fscrypt_key_v1_ext_mock.h"
#include "key_blob.h"
#include "storage_service_errno.h"
#include "file_utils_mock.h"

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
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
    UserAuth emptyUserAuth {};
    static inline std::shared_ptr<FscryptKeyV1ExtMock> fscryptKeyExtMock_ = nullptr;
    static inline std::shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
};

void FscryptKeyV1Test::SetUp(void)
{
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    fscryptKeyExtMock_ = std::make_shared<FscryptKeyV1ExtMock>();
    FscryptKeyV1ExtMock::fscryptKeyV1ExtMock = fscryptKeyExtMock_;
    baseKeyMock_ = std::make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
}

void FscryptKeyV1Test::TearDown(void)
{
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    FscryptKeyV1ExtMock::fscryptKeyV1ExtMock = nullptr;
    fscryptKeyExtMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    KeyBlob appKey;
    EXPECT_NE(g_testKeyV1->GenerateAppKeyDesc(appKey), E_OK);

    KeyBlob appKey1(FBEX_KEYID_SIZE);
    EXPECT_EQ(g_testKeyV1->GenerateAppKeyDesc(appKey1), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    bool isFbeSupport = true;
    EXPECT_CALL(*fscryptKeyExtMock_, LockUeceExt(_)).WillOnce(Return(1));
    EXPECT_NE(g_testKeyV1->LockUece(isFbeSupport), E_OK);

    EXPECT_CALL(*fscryptKeyExtMock_, LockUeceExt(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->LockUece(isFbeSupport), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    bool isFbeSupport = true;
    uint32_t userId = 100;
    EXPECT_CALL(*fscryptKeyExtMock_, ChangePinCodeClassE(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->ChangePinCodeClassE(isFbeSupport, userId), E_OK);

    EXPECT_CALL(*fscryptKeyExtMock_, ChangePinCodeClassE(_, _)).WillOnce(Return(-1));
    EXPECT_NE(g_testKeyV1->ChangePinCodeClassE(isFbeSupport, userId), E_OK);
    GTEST_LOG_(INFO) << "fscrypt_key_v1_ChangePinCodeClassE end";
}

/**
 * @tc.name: fscrypt_key_v1_UpdateClassEBackUp
 * @tc.desc: Verify the fscrypt V1 UpdateClassEBackUp.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_UpdateClassEBackUp, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_UpdateClassEBackUp start";
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    uint32_t userId = 100;
    EXPECT_CALL(*fscryptKeyExtMock_, UpdateClassEBackUp(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->UpdateClassEBackUp(userId), E_OK);

    EXPECT_CALL(*fscryptKeyExtMock_, UpdateClassEBackUp(_)).WillOnce(Return(-1));
    EXPECT_NE(g_testKeyV1->UpdateClassEBackUp(userId), E_OK);
    GTEST_LOG_(INFO) << "fscrypt_key_v1_UpdateClassEBackUp end";
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    uint32_t userId = 100;
    EXPECT_CALL(*fscryptKeyExtMock_, DeleteClassEPinCode(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->DeleteClassEPinCode(userId), E_OK);

    EXPECT_CALL(*fscryptKeyExtMock_, DeleteClassEPinCode(_)).WillOnce(Return(-1));
    EXPECT_NE(g_testKeyV1->DeleteClassEPinCode(userId), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    uint32_t status = 100;
    bool isSupport = true;
    bool isNeedEncryptClassE = true;

    EXPECT_CALL(*fscryptKeyExtMock_, AddClassE(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->AddClassE(isNeedEncryptClassE, isSupport, status), E_OK);

    EXPECT_CALL(*fscryptKeyExtMock_, AddClassE(_, _, _)).WillOnce(Return(-1));
    EXPECT_NE(g_testKeyV1->AddClassE(isNeedEncryptClassE, isSupport, status), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    std::string keyId = "";
    EXPECT_EQ(g_testKeyV1->DeleteAppkey(keyId), E_KEY_TYPE_INVALID);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    std::string keyId = "";
    EXPECT_EQ(g_testKeyV1->UninstallKeyForAppKeyToKeyring(keyId), E_KEY_TYPE_INVALID);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    KeyBlob appKeyRaw(17);
    EXPECT_EQ(g_testKeyV1->InstallKeyForAppKeyToKeyring(appKeyRaw), E_OK);
    appKeyRaw.Clear();

    KeyBlob appKey(10);
    EXPECT_EQ(g_testKeyV1->InstallKeyForAppKeyToKeyring(appKey), E_OK);
    appKey.Clear();
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    uint32_t flag = 1;
    uint32_t sdpClass = 1;
    const std::string mnt = "test";

    g_testKeyV1->keyInfo_.key.Clear();
    EXPECT_EQ(g_testKeyV1->UnlockUserScreen({}, flag, sdpClass, mnt), E_KEY_EMPTY_ERROR);

    g_testKeyV1->keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*fscryptKeyExtMock_, UnlockUserScreenExt(_, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(g_testKeyV1->UnlockUserScreen({}, flag, sdpClass, mnt), 1);

    g_testKeyV1->keyInfo_.key.Clear();
    g_testKeyV1->keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*fscryptKeyExtMock_, UnlockUserScreenExt(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->UnlockUserScreen({}, flag, sdpClass, mnt), E_OK);

    sdpClass = 2;
    g_testKeyV1->keyInfo_.key.Clear();
    g_testKeyV1->keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*fscryptKeyExtMock_, UnlockUserScreenExt(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, SaveKeyBlob(_, _)).WillOnce(Return(true));
    EXPECT_EQ(g_testKeyV1->UnlockUserScreen({}, flag, sdpClass, mnt), E_OK);
    g_testKeyV1->keyInfo_.key.Clear();
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    uint32_t userId = 100;
    uint32_t hashId = 2;
    std::string keyDesc = "test";

    EXPECT_CALL(*fscryptKeyExtMock_, GenerateAppkey(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_NE(g_testKeyV1->GenerateAppkey(userId, hashId, keyDesc), E_OK);

    EXPECT_CALL(*fscryptKeyExtMock_, GenerateAppkey(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->GenerateAppkey(userId, hashId, keyDesc), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    uint32_t flag = 1;
    uint32_t sdpClass = 2;

    uint32_t elType = TYPE_EL4;
    g_testKeyV1->keyInfo_.keyDesc.Alloc(TEST_KEYID_SIZE);
    GTEST_LOG_(INFO) << "xxxxxx " <<  g_testKeyV1->keyInfo_.keyDesc.IsEmpty();
    EXPECT_CALL(*fscryptKeyExtMock_, LockUserScreenExt(_, _)).WillOnce(Return(1));
    EXPECT_EQ(g_testKeyV1->LockUserScreen(flag, sdpClass, TEST_MNT), 1);

    elType = TYPE_EL4;
    g_testKeyV1->keyInfo_.keyDesc.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*fscryptKeyExtMock_, LockUserScreenExt(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->LockUserScreen(flag, sdpClass, TEST_MNT), E_OK);

    elType = TYPE_EL1;
    g_testKeyV1->keyInfo_.keyDesc.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*fscryptKeyExtMock_, LockUserScreenExt(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->LockUserScreen(flag, sdpClass, TEST_MNT), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    uint32_t sdpClass = 2;
    EXPECT_EQ(g_testKeyV1->InstallEceSeceKeyToKeyring(sdpClass), E_KEY_SIZE_ERROR);

    g_testKeyV1->keyInfo_.key.Clear();
    g_testKeyV1->keyInfo_.keyDesc.Clear();
    g_testKeyV1->keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*baseKeyMock_, SaveKeyBlob(_, _)).WillOnce(Return(true));
    EXPECT_EQ(g_testKeyV1->InstallEceSeceKeyToKeyring(sdpClass), E_OK);

    g_testKeyV1->keyInfo_.key.Clear();
    g_testKeyV1->keyInfo_.keyDesc.Clear();
    g_testKeyV1->keyInfo_.key.Alloc(TEST_KEYID_SIZE);
    g_testKeyV1->keyInfo_.keyDesc.Alloc(TEST_KEYID_SIZE);
    EXPECT_CALL(*baseKeyMock_, SaveKeyBlob(_, _)).WillOnce(Return(true));
    EXPECT_EQ(g_testKeyV1->InstallEceSeceKeyToKeyring(sdpClass), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    bool isSupport = true;
    bool eBufferStatue = true;
    uint32_t user = 1;
    uint32_t status = 1;

    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _)).WillOnce(Return(1));
    EXPECT_EQ(g_testKeyV1->DecryptClassE(emptyUserAuth, isSupport, eBufferStatue, user, status), 1);

    g_testKeyV1->keyInfo_.key.Clear();
    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->DecryptClassE(emptyUserAuth, isSupport, eBufferStatue, user, status), E_OK);

    g_testKeyV1->keyInfo_.key.Clear();
    const UserAuth auth{KeyBlob(8), KeyBlob(8), 0};
    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, GetCandidateDir()).WillOnce(Return("test"));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*baseKeyMock_, DecryptKeyBlob(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fscryptKeyExtMock_, WriteClassE(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(g_testKeyV1->DecryptClassE(auth, isSupport, eBufferStatue, user, status), E_OK);
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
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    bool isSupport = true;
    uint32_t user = 1;
    uint32_t status = 1;

    EXPECT_CALL(*fscryptKeyExtMock_, ReadClassE(_, _, _)).WillOnce(Return(1));
    EXPECT_NE(g_testKeyV1->EncryptClassE(emptyUserAuth, isSupport, user, status), E_OK);
    GTEST_LOG_(INFO) << "fscrypt_key_v1_EncryptClassE end";
}

/**
 * @tc.name: fscrypt_key_v1_DoDecryptClassE_001
 * @tc.desc: Verify the fscrypt V1 DoDecryptClassE.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV1Test, fscrypt_key_v1_DoDecryptClassE_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DoDecryptClassE_001 start";
    auto g_testKeyV1 = std::make_shared<OHOS::StorageDaemon::FscryptKeyV1>(TEST_KEYPATH);
    UserAuth auth;
    KeyBlob eSecretFBE;
    KeyBlob decryptedKey;
    std::vector<std::string> fileNames = {
        "latest",
        "version_1000",
        "version_1001",
        "invalidPath1",
        "invalidPath2",
    };

    EXPECT_CALL(*baseKeyMock_, GetCandidateDir()).WillOnce(Return("test"));
    EXPECT_CALL(*baseKeyMock_, DecryptKeyBlob(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(0));
    EXPECT_EQ(g_testKeyV1->DoDecryptClassE(auth, eSecretFBE, decryptedKey), E_OK);

    EXPECT_CALL(*baseKeyMock_, GetCandidateDir()).WillOnce(Return("test"));
    EXPECT_CALL(*baseKeyMock_, DecryptKeyBlob(_, _, _, _)).WillRepeatedly(Return(E_KEY_SIZE_ERROR));
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(SetArgReferee<1>(fileNames));
    EXPECT_EQ(g_testKeyV1->DoDecryptClassE(auth, eSecretFBE, decryptedKey), E_KEY_SIZE_ERROR);

    EXPECT_CALL(*baseKeyMock_, GetCandidateDir()).WillOnce(Return("test"));
    EXPECT_CALL(*baseKeyMock_, DecryptKeyBlob(_, _, _, _)).WillOnce(Return(E_KEY_SIZE_ERROR)).WillOnce(Return(E_OK));
    EXPECT_CALL(*baseKeyMock_, UpdateKey(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(SetArgReferee<1>(fileNames));
    EXPECT_EQ(g_testKeyV1->DoDecryptClassE(auth, eSecretFBE, decryptedKey), E_OK);
    GTEST_LOG_(INFO) << "fscrypt_key_v1_DoDecryptClassE_001 end";
}
} // OHOS::StorageDaemon