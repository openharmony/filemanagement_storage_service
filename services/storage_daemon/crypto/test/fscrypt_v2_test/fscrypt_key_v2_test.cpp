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
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "directory_ex.h"
#include "file_ex.h"

#include "fbex.h"
#include "fscrypt_key_v2.h"
#include "key_blob.h"
#include "libfscrypt/fscrypt_control.h"
#include "storage_service_errno.h"

using namespace testing::ext;

namespace {
const std::string TEST_MNT = "/data";
const std::string TEST_DIR_LEGACY = "/data/test/crypto_dir_legacy";
const std::string TEST_DIR_V2 = "/data/test/crypto_dir";
const std::string TEST_KEYPATH = "/data/test/key/el2/80";

OHOS::StorageDaemon::FscryptKeyV2 g_testKeyV2 {TEST_KEYPATH};
}

namespace OHOS::StorageDaemon {
class FscryptKeyV2Test : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    UserAuth emptyUserAuth {};
};

void FscryptKeyV2Test::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void FscryptKeyV2Test::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void FscryptKeyV2Test::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void FscryptKeyV2Test::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: fscrypt_key_v2_active_support
 * @tc.desc: Verify the fscrypt V2 active function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_active_support, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_active_support start";
    uint32_t flag = 1;
    g_testKeyV2.ClearKey();
    EXPECT_FALSE(g_testKeyV2.ActiveKey(flag, TEST_MNT));

    g_testKeyV2.keyInfo_.key.Alloc(FSCRYPT_MAX_KEY_SIZE + 1);
    EXPECT_FALSE(g_testKeyV2.ActiveKey(flag, TEST_MNT));

    g_testKeyV2.ClearKey();
    std::string emptyStr;
    g_testKeyV2.keyInfo_.key.Alloc(FSCRYPT_MAX_KEY_SIZE);
    EXPECT_FALSE(g_testKeyV2.ActiveKey(flag, emptyStr));

    g_testKeyV2.ClearKey();
    g_testKeyV2.keyInfo_.keyId.Clear();
    g_testKeyV2.keyInfo_.key.Alloc(FSCRYPT_MAX_KEY_SIZE);
    g_testKeyV2.keyInfo_.keyId.Alloc(FSCRYPT_KEY_IDENTIFIER_SIZE + 1);
    EXPECT_FALSE(g_testKeyV2.ActiveKey(flag, TEST_MNT));

    g_testKeyV2.ClearKey();
    g_testKeyV2.keyInfo_.keyId.Clear();
    g_testKeyV2.keyInfo_.key.Alloc(FSCRYPT_MAX_KEY_SIZE);
    g_testKeyV2.keyInfo_.keyId.Alloc(FSCRYPT_KEY_IDENTIFIER_SIZE);
    EXPECT_FALSE(g_testKeyV2.ActiveKey(flag, TEST_MNT));
    GTEST_LOG_(INFO) << "fscrypt_key_v2_active_support end";
}

/**
 * @tc.name: fscrypt_key_v2_policy_inactive_support
 * @tc.desc: Verify the fscryptV2 InactiveKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_policy_inactive_support, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_policy_inactive_support start";
    uint32_t flag = 1;
    
    g_testKeyV2.keyInfo_.keyId.Clear();
    g_testKeyV2.keyInfo_.keyId.Alloc(FSCRYPT_KEY_IDENTIFIER_SIZE + 1);
    EXPECT_FALSE(g_testKeyV2.InactiveKey(flag, TEST_MNT));

    g_testKeyV2.keyInfo_.keyId.Clear();
    g_testKeyV2.keyInfo_.keyId.Alloc(FSCRYPT_KEY_IDENTIFIER_SIZE);
    EXPECT_FALSE(g_testKeyV2.InactiveKey(flag, TEST_MNT));
    g_testKeyV2.ClearKey();
    g_testKeyV2.keyInfo_.keyId.Clear();
    GTEST_LOG_(INFO) << "fscrypt_key_v2_policy_inactive_support end";
}

/**
 * @tc.name: fscrypt_key_v2_LockUserScreen_UnLockUserScreen
 * @tc.desc: Verify the fscrypt V2 LockUserScreen/UnLockUserScreen.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_LockUserScreen_UnLockUserScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LockUserScreen_UnLockUserScreen start";
    uint32_t flag = 1;
    uint32_t sdpClass = 1;
    EXPECT_TRUE(g_testKeyV2.LockUserScreen(flag, sdpClass, TEST_MNT));
    EXPECT_TRUE(g_testKeyV2.UnlockUserScreen(flag, sdpClass, TEST_MNT));
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LockUserScreen_UnLockUserScreen end";
}

/**
 * @tc.name: fscrypt_key_v2_LockUece
 * @tc.desc: Verify the fscrypt V2 LockUece.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_LockUece, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LockUece start";
    bool isFbeSupport = true;
    EXPECT_TRUE(g_testKeyV2.LockUece(isFbeSupport));
    EXPECT_FALSE(isFbeSupport);
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LockUece end";
}

/**
 * @tc.name: fscrypt_key_v2_GenerateAppkey_DeleteAppkey
 * @tc.desc: Verify the fscrypt V2 GenerateAppkey/DeleteAppkey.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_GenerateAppkey_DeleteAppkey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_GenerateAppkey_DeleteAppkey start";
    uint32_t userId = 1;
    uint32_t appUid = 1;
    std::string TEST_KEYID = "1234";
    EXPECT_NE(g_testKeyV2.GenerateAppkey(userId, appUid, TEST_KEYID), E_OK);
    EXPECT_NE(g_testKeyV2.DeleteAppkey(TEST_KEYID), E_OK);
    GTEST_LOG_(INFO) << "fscrypt_key_v2_GenerateAppkey_DeleteAppkey end";
}

/**
 * @tc.name: fscrypt_key_v2_AddClassE_ChangePinCodeClassE
 * @tc.desc: Verify the fscrypt V2 AddClassE/ChangePinCodeClassE.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_AddClassE_ChangePinCodeClassE, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_AddClassE_ChangePinCodeClassE start";
    bool isNeedEncryptClassE = true;
    bool isSupport = true;
    EXPECT_EQ(g_testKeyV2.AddClassE(isNeedEncryptClassE, isSupport), E_OK);
    bool isFbeSupport = true;
    EXPECT_EQ(g_testKeyV2.ChangePinCodeClassE(isFbeSupport), E_OK);
    EXPECT_FALSE(isFbeSupport);
    GTEST_LOG_(INFO) << "fscrypt_key_v2_AddClassE_ChangePinCodeClassE end";
}

/**
 * @tc.name: fscrypt_key_v2_DecryptClassE_EncryptClassE_DeleteClassEPinCode
 * @tc.desc: Verify the fscrypt V2 DecryptClassE/EncryptClassE/DeleteClassEPinCode.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_DecryptClassE_EncryptClassE_DeleteClassEPinCode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_DecryptClassE_EncryptClassE_DeleteClassEPinCode start";
    bool isSupport = true;
    bool eBufferStatue = true;
    uint32_t user = 1;
    uint32_t status = 1;
    EXPECT_EQ(g_testKeyV2.DecryptClassE(emptyUserAuth, isSupport, eBufferStatue, user, status), E_OK);
    EXPECT_FALSE(isSupport);
    isSupport = true;
    EXPECT_EQ(g_testKeyV2.EncryptClassE(emptyUserAuth, isSupport), E_OK);
    EXPECT_FALSE(isSupport);
    EXPECT_EQ(g_testKeyV2.DeleteClassEPinCode(user), E_OK);
    GTEST_LOG_(INFO) << "fscrypt_key_v2_DecryptClassE_EncryptClassE_DeleteClassEPinCode end";
}

/**
 * @tc.name: fscrypt_key_v2_LoadAndSetPolicy
 * @tc.desc: Verify the fscrypt V2 LoadAndSetPolicy
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_LoadAndSetPolicy, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LoadAndSetPolicy start";
    OHOS::ForceRemoveDirectory(TEST_DIR_V2);
    OHOS::ForceCreateDirectory(TEST_DIR_V2);
    OHOS::ForceRemoveDirectory(TEST_DIR_LEGACY);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_LEGACY));
    std::string testVersionFile = TEST_DIR_LEGACY + "/fscrypt_version";
    EXPECT_TRUE(OHOS::SaveStringToFile(testVersionFile, "2\n"));
    EXPECT_NE(0, LoadAndSetPolicy(TEST_DIR_LEGACY.c_str(), TEST_DIR_V2.c_str()));
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LoadAndSetPolicy end";
}

/**
 * @tc.name: fscrypt_key_v2_LoadAndSetEceAndSecePolicy
 * @tc.desc: Verify the fscrypt V2 LoadAndSetEceAndSecePolicy
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_LoadAndSetEceAndSecePolicy, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LoadAndSetEceAndSecePolicy start";
    int type = 1;
    const char* dir = TEST_MNT.c_str();
    EXPECT_EQ(LoadAndSetEceAndSecePolicy(nullptr, nullptr, type), -EINVAL);
    EXPECT_EQ(LoadAndSetEceAndSecePolicy(TEST_DIR_LEGACY.c_str(), nullptr, type), -EINVAL);
    EXPECT_EQ(LoadAndSetEceAndSecePolicy(nullptr, dir, type), -EINVAL);
    EXPECT_EQ(LoadAndSetEceAndSecePolicy(TEST_DIR_LEGACY.c_str(), dir, type), 0);

    OHOS::ForceRemoveDirectory(TEST_DIR_LEGACY);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_LEGACY));
    std::string testVersionFile = TEST_DIR_LEGACY + "/fscrypt_version";
    EXPECT_TRUE(OHOS::SaveStringToFile(testVersionFile, "1\n"));
    EXPECT_EQ(LoadAndSetEceAndSecePolicy(TEST_DIR_LEGACY.c_str(), dir, type), 0);
    EXPECT_EQ(type, 1);

    if (KeyCtrlLoadVersion(TEST_DIR_LEGACY.c_str()) == FSCRYPT_V1) {
        type = 3;
        EXPECT_NE(LoadAndSetEceAndSecePolicy(TEST_DIR_LEGACY.c_str(), dir, type), 0);

        type = 4;
        EXPECT_NE(LoadAndSetEceAndSecePolicy(TEST_DIR_LEGACY.c_str(), dir, type), 0);

        OHOS::ForceRemoveDirectory(TEST_DIR_LEGACY);
        EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_LEGACY));
        testVersionFile = TEST_DIR_LEGACY + "/fscrypt_version";
        EXPECT_TRUE(OHOS::SaveStringToFile(testVersionFile, "2\n"));
        EXPECT_NE(LoadAndSetEceAndSecePolicy(TEST_DIR_LEGACY.c_str(), dir, type), 0);
    }
    GTEST_LOG_(INFO) << "fscrypt_key_v2_LoadAndSetEceAndSecePolicy end";
}

/**
 * @tc.name: fscrypt_key_v2_GetOriginKey
 * @tc.desc: Verify the fscrypt V2 SetOriginKey/GetOriginKey.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_GetOriginKey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_GetOriginKey start";
    KeyBlob appKey(10);
    g_testKeyV2.SetOriginKey(appKey);
    KeyBlob originKey;
    EXPECT_TRUE(g_testKeyV2.GetOriginKey(originKey));
    EXPECT_FALSE(originKey.IsEmpty());

    originKey.Clear();
    g_testKeyV2.SetOriginKey(originKey);
    KeyBlob testKey;
    EXPECT_FALSE(g_testKeyV2.GetOriginKey(testKey));
    GTEST_LOG_(INFO) << "fscrypt_key_v2_GetOriginKey end";
}

/**
 * @tc.name: fscrypt_key_v2_KeyEncryptTypeToString
 * @tc.desc: Verify the fscrypt V2 KeyEncryptTypeToString.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_KeyEncryptTypeToString, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_KeyEncryptTypeToString start";
    EXPECT_EQ(g_testKeyV2.KeyEncryptTypeToString(BaseKey::KeyEncryptType::KEY_CRYPT_OPENSSL), "KEY_CRYPT_OPENSSL");
    EXPECT_EQ(g_testKeyV2.KeyEncryptTypeToString(BaseKey::KeyEncryptType::KEY_CRYPT_HUKS), "KEY_CRYPT_HUKS");
    EXPECT_EQ(g_testKeyV2.KeyEncryptTypeToString(
        BaseKey::KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL), "KEY_CRYPT_HUKS_OPENSSL");
    GTEST_LOG_(INFO) << "fscrypt_key_v2_KeyEncryptTypeToString end";
}

/**
 * @tc.name: fscrypt_key_v2_CombKeyCtx
 * @tc.desc: Verify the fscrypt V2 CombKeyCtx.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_CombKeyCtx, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_CombKeyCtx start";
    std::vector<uint8_t> nonceVct(5, 1);
    std::vector<uint8_t> rndEncVct(3, 2);
    std::vector<uint8_t> aadVct(4, 3);
    KeyBlob nonce(nonceVct);
    KeyBlob rndEnc(rndEncVct);
    KeyBlob aad(aadVct);
    KeyBlob keyOut(nonceVct.size() + rndEncVct.size() + aadVct.size());
    EXPECT_TRUE(g_testKeyV2.CombKeyCtx(nonce, rndEnc, aad, keyOut));
    std::vector<uint8_t> keyOutVct(keyOut.data.get(), keyOut.data.get() + keyOut.size);
    std::vector<uint8_t> keyOutVctEpt;
    keyOutVctEpt.insert(keyOutVctEpt.end(), nonceVct.begin(), nonceVct.end());
    keyOutVctEpt.insert(keyOutVctEpt.end(), rndEncVct.begin(), rndEncVct.end());
    keyOutVctEpt.insert(keyOutVctEpt.end(), aadVct.begin(), aadVct.end());
    EXPECT_EQ(keyOutVct, keyOutVctEpt);
    keyOut.Clear();
    rndEnc.Clear();
    EXPECT_FALSE(g_testKeyV2.CombKeyCtx(nonce, rndEnc, aad, keyOut));
    EXPECT_TRUE(keyOut.IsEmpty());

    aad.Clear();
    EXPECT_FALSE(g_testKeyV2.CombKeyCtx(nonce, rndEnc, aad, keyOut));
    EXPECT_TRUE(keyOut.IsEmpty());

    nonce.Clear();
    EXPECT_FALSE(g_testKeyV2.CombKeyCtx(nonce, rndEnc, aad, keyOut));
    EXPECT_TRUE(keyOut.IsEmpty());
    GTEST_LOG_(INFO) << "fscrypt_key_v2_CombKeyCtx end";
}

/**
 * @tc.name: fscrypt_key_v2_SplitKeyCtx
 * @tc.desc: Verify the fscrypt V2 SplitKeyCtx.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_SplitKeyCtx, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_SplitKeyCtx start";
    std::vector<uint8_t> nonceVct(5, 1);
    std::vector<uint8_t> aadVct(4, 3);
    KeyBlob nonce(nonceVct);
    KeyBlob aad(aadVct);
    KeyBlob keyIn;
    KeyBlob rndEnc;
    EXPECT_FALSE(g_testKeyV2.SplitKeyCtx(keyIn, nonce, rndEnc, aad));

    std::vector<uint8_t> keyInVct(11, 2);
    KeyBlob keyIn2(keyInVct);
    EXPECT_TRUE(g_testKeyV2.SplitKeyCtx(keyIn2, nonce, rndEnc, aad));
    EXPECT_EQ(rndEnc.size, 2);
    std::vector<uint8_t> rndEncVct(rndEnc.data.get(), rndEnc.data.get() + rndEnc.size);
    std::vector<uint8_t> rndEncVctEpt(2, 2);
    EXPECT_EQ(rndEncVct, rndEncVctEpt);
    GTEST_LOG_(INFO) << "fscrypt_key_v2_SplitKeyCtx end";
}

/**
 * @tc.name: fscrypt_key_v2_CombKeyBlob
 * @tc.desc: Verify the fscrypt V2 CombKeyBlob.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_CombKeyBlob, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_CombKeyBlob start";
    std::vector<uint8_t> encAadVec{1, 2, 3, 4, 5};
    std::vector<uint8_t> endVec{6, 7, 8};
    KeyBlob encAad(encAadVec);
    KeyBlob end(endVec);
    KeyBlob keyOut(encAad.size + end.size);

    std::vector<uint8_t> expect{1, 2, 3, 4, 5, 6, 7, 8};
    g_testKeyV2.CombKeyBlob(encAad, end, keyOut);
    std::vector<uint8_t> result(keyOut.data.get(), keyOut.data.get() + keyOut.size);
    EXPECT_EQ(result, expect);
    GTEST_LOG_(INFO) << "fscrypt_key_v2_CombKeyBlob end";
}

/**
 * @tc.name: fscrypt_key_v2_SplitKeyBlob
 * @tc.desc: Verify the fscrypt V2 SplitKeyBlob.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_SplitKeyBlob, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_SplitKeyBlob start";
    std::vector<uint8_t> keyInVct{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    KeyBlob encAad;
    KeyBlob nonce;
    KeyBlob keyIn(keyInVct);
    uint32_t start = 6;

    g_testKeyV2.SplitKeyBlob(keyIn, encAad, nonce, start);
    std::vector<uint8_t> encAadVct(encAad.data.get(), encAad.data.get() + encAad.size);
    std::vector<uint8_t> encAadVctEpt{1, 2, 3, 4, 5, 6};
    EXPECT_EQ(encAadVct, encAadVctEpt);

    std::vector<uint8_t> nonceVct(nonce.data.get(), nonce.data.get() + nonce.size);
    std::vector<uint8_t> nonceVctEpt{7, 8, 9, 10};
    EXPECT_EQ(nonceVct, nonceVctEpt);
    GTEST_LOG_(INFO) << "fscrypt_key_v2_SplitKeyBlob end";
}

/**
 * @tc.name: fscrypt_key_v2_ClearKeyContext
 * @tc.desc: Verify the fscrypt V2 ClearKeyContext.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(FscryptKeyV2Test, fscrypt_key_v2_ClearKeyContext, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fscrypt_key_v2_ClearKeyContext start";
    KeyContext keyCtx;
    keyCtx.rndEnc.Alloc(5);
    keyCtx.shield.Alloc(5);
    keyCtx.nonce.Alloc(5);
    keyCtx.aad.Alloc(5);
    keyCtx.secDiscard.Alloc(5);

    g_testKeyV2.ClearKeyContext(keyCtx);
    EXPECT_TRUE(keyCtx.rndEnc.IsEmpty());
    EXPECT_TRUE(keyCtx.shield.IsEmpty());
    EXPECT_TRUE(keyCtx.nonce.IsEmpty());
    EXPECT_TRUE(keyCtx.aad.IsEmpty());
    EXPECT_TRUE(keyCtx.secDiscard.IsEmpty());
    GTEST_LOG_(INFO) << "fscrypt_key_v2_ClearKeyContext end";
}
} // OHOS::StorageDaemon