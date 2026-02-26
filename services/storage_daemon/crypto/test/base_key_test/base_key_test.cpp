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
#include "base_key.h"
#include <filesystem>
#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <unistd.h>

#include "directory_ex.h"
#include "file_ex.h"

#include "common_utils_mock.h"
#include "fbex.h"
#include "file_utils_mock.h"
#include "fscrypt_key_v2.h"
#include "huks_master_mock.h"
#include "key_blob.h"
#include "key_control_mock.h"
#include "openssl_crypto_mock.h"
#include "storage_service_errno.h"
#include "string_utils.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace {
    const std::string PATH_LATEST_BACKUP = "/latest_bak";
}

namespace OHOS::StorageDaemon {
class BaseKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<HuksMasterMock> huksMasterMock_ = nullptr;
    static inline shared_ptr<OpensslCryptoMock> opensslCryptoMock_ = nullptr;
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
    static inline shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline shared_ptr<CommonUtilsMock> commonUtilMoc_ = nullptr;
};

void BaseKeyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void BaseKeyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void BaseKeyTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
    huksMasterMock_ = make_shared<HuksMasterMock>();
    HuksMasterMock::huksMasterMock = huksMasterMock_;
    opensslCryptoMock_ = make_shared<OpensslCryptoMock>();
    OpensslCryptoMock::opensslCryptoMock = opensslCryptoMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
    fileUtilMoc_ = make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    commonUtilMoc_ = make_shared<CommonUtilsMock>();
    CommonUtilsMock::utils = commonUtilMoc_;
}

void BaseKeyTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
    OpensslCryptoMock::opensslCryptoMock = nullptr;
    opensslCryptoMock_ = nullptr;
    HuksMasterMock::huksMasterMock = nullptr;
    huksMasterMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    CommonUtilsMock::utils = nullptr;
    commonUtilMoc_ = nullptr;
}

/**
 * @tc.name: BaseKey_Decrypt_001
 * @tc.desc: Verify the Decrypt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_Decrypt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_Decrypt_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth emptyUserAuth;
    elKey->keyEncryptType_ = BaseKey::KeyEncryptType::KEY_CRYPT_OPENSSL;
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->Decrypt(emptyUserAuth), E_OK);

    elKey->keyEncryptType_ = BaseKey::KeyEncryptType::KEY_CRYPT_HUKS;
    EXPECT_CALL(*huksMasterMock_, DecryptKey(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->Decrypt(emptyUserAuth), E_OK);

    elKey->keyEncryptType_ = BaseKey::KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL;
    EXPECT_EQ(elKey->Decrypt(emptyUserAuth), E_PARAMS_INVALID);

    int encryptType = 4;
    elKey->keyEncryptType_ = static_cast<BaseKey::KeyEncryptType>(encryptType);
    EXPECT_EQ(elKey->Decrypt(emptyUserAuth), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "BaseKey_Decrypt_001 end";
}

/**
 * @tc.name: BaseKey_DecryptReal_001
 * @tc.desc: Verify the DecryptReal function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_DecryptReal_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DecryptReal_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth emptyUserAuth;
    uint32_t keyType = TYPE_EL2;
    KeyContext keyCtx;
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx), E_ERR);

    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx), E_ERR);

    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx), E_OK);

    keyType = TYPE_EL3;
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx), E_OK);

    keyType = TYPE_EL4;
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx), E_OK);
    GTEST_LOG_(INFO) << "BaseKey_DecryptReal_001 end";
}

/**
 * @tc.name: BaseKey_SaveAndCleanKeyBuff_001
 * @tc.desc: Verify the SaveAndCleanKeyBuff function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_SaveAndCleanKeyBuff_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_SaveAndCleanKeyBuff_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    std::string keyPath = "/test";
    KeyContext keyCtx;
    EXPECT_FALSE(elKey->SaveAndCleanKeyBuff(keyPath, keyCtx));

    std::vector<uint8_t> nonceVct(5, 1);
    std::vector<uint8_t> rndEncVct(3, 2);
    std::vector<uint8_t> aadVct(4, 3);
    keyCtx.nonce.Alloc(nonceVct.size());
    std::copy(nonceVct.begin(), nonceVct.end(), keyCtx.nonce.data.get());
    keyCtx.rndEnc.Alloc(rndEncVct.size());
    std::copy(rndEncVct.begin(), rndEncVct.end(), keyCtx.rndEnc.data.get());
    keyCtx.aad.Alloc(aadVct.size());
    std::copy(aadVct.begin(), aadVct.end(), keyCtx.aad.data.get());
    EXPECT_FALSE(elKey->SaveAndCleanKeyBuff(keyPath, keyCtx));

    keyPath = "/data/test";
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    EXPECT_TRUE(elKey->SaveAndCleanKeyBuff(keyPath, keyCtx));
    EXPECT_TRUE(keyCtx.nonce.IsEmpty());
    EXPECT_TRUE(keyCtx.rndEnc.IsEmpty());
    EXPECT_TRUE(keyCtx.aad.IsEmpty());

    const std::string needUpdatePath = keyPath + "/need_update";
    auto ret = unlink(needUpdatePath.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_SaveAndCleanKeyBuff_001! " << errno;
    GTEST_LOG_(INFO) << "BaseKey_SaveAndCleanKeyBuff_001 end";
}

/**
 * @tc.name: BaseKey_UpdateKey_001
 * @tc.desc: Verify the UpdateKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_UpdateKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_UpdateKey_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    std::string keyPath = "";
    bool needSyncCandidate = false;
    EXPECT_EQ(elKey->UpdateKey(keyPath, needSyncCandidate), E_OK);

    needSyncCandidate = true;
    elKey->dir_ = "/data/foo/bar/el5/100";
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(Return());
    EXPECT_EQ(elKey->UpdateKey(keyPath, needSyncCandidate), E_OK);

    needSyncCandidate = true;
    elKey->dir_ = "/data/foo/bar/el1/100";
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(Return());
    EXPECT_EQ(elKey->UpdateKey(keyPath, needSyncCandidate), E_EMPTY_CANDIDATE_ERROR);

    keyPath = "/test";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_EQ(elKey->UpdateKey(keyPath, needSyncCandidate), E_RENAME_FILE_ERROR);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_EQ(elKey->UpdateKey(keyPath, needSyncCandidate), E_RENAME_FILE_ERROR);

    keyPath = "/test/latest";
    elKey->dir_ = "/test";
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(Return());
    EXPECT_EQ(elKey->UpdateKey(keyPath, needSyncCandidate), E_OK);
    GTEST_LOG_(INFO) << "BaseKey_UpdateKey_001 end";
}

/**
 * @tc.name: BaseKey_EncryptEceSece_001
 * @tc.desc: Verify the EncryptEceSece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_EncryptEceSece_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_EncryptEceSece_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth auth;
    uint32_t keyType = TYPE_EL2;
    KeyContext keyCtx;
    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_NE(elKey->EncryptEceSece(auth, keyType, keyCtx), E_OK);

    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(-1));
    EXPECT_NE(elKey->EncryptEceSece(auth, keyType, keyCtx), E_OK);

    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->EncryptEceSece(auth, keyType, keyCtx), E_OK);
    EXPECT_EQ(elKey->keyEncryptType_, BaseKey::KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL);

    keyType = TYPE_EL3;
    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(-1));
    EXPECT_NE(elKey->EncryptEceSece(auth, keyType, keyCtx), E_OK);

    keyType = TYPE_EL4;
    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(-1));
    EXPECT_NE(elKey->EncryptEceSece(auth, keyType, keyCtx), E_OK);
    GTEST_LOG_(INFO) << "BaseKey_EncryptEceSece_001 end";
}

/**
 * @tc.name: BaseKey_DoRestoreKeyCeEceSece_001
 * @tc.desc: Verify the DoRestoreKeyCeEceSece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_DoRestoreKeyCeEceSece_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth auth;
    std::string path = "/data/test";
    uint32_t keyType = TYPE_EL2;
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    std::string test = "1234567890";
    string pathEncrypt = path + PATH_ENCRYPTED;
    std::string errMsg = "";
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(pathEncrypt, test, errMsg));
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);
    auto ret = unlink(pathEncrypt.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyCeEceSece_001! " << errno;
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_001 end";
}

/**
 * @tc.name: BaseKey_DoRestoreKeyCeEceSece_002
 * @tc.desc: Verify the DoRestoreKeyCeEceSece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_DoRestoreKeyCeEceSece_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_002 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth auth;
    std::string path = "/data/test";
    uint32_t keyType = TYPE_EL2;
    std::string test = "1234567890";
    std::string rightContent = "123456789012345678901234567890";
    string pathEncrypt = path + PATH_ENCRYPTED;
    std::string errMsg = "";
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(pathEncrypt, rightContent, errMsg));
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    string pathSecdisc = path + PATH_SECDISC;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(path + PATH_SECDISC, test, errMsg));
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    string pathShield = path + PATH_SHIELD;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(path + PATH_SHIELD, test, errMsg));
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(-1));
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    std::vector<uint8_t> secretVec{1, 2, 3, 4, 5};
    std::vector<uint8_t> tokenVec{6, 7, 8};
    auth.secret.Alloc(secretVec.size());
    std::copy(secretVec.begin(), secretVec.end(), auth.secret.data.get());
    auth.token.Alloc(tokenVec.size());
    std::copy(tokenVec.begin(), tokenVec.end(), auth.token.data.get());
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    auto ret = unlink(pathEncrypt.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyCeEceSece_002! " << errno;
    ret = unlink(pathSecdisc.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyCeEceSece_002! " << errno;
    ret = unlink(pathShield.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyCeEceSece_002! " << errno;
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_002 end";
}

/**
 * @tc.name: BaseKey_DoRestoreKeyCeEceSece_003
 * @tc.desc: Verify the DoRestoreKeyCeEceSece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_DoRestoreKeyCeEceSece_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_003 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth auth;
    std::string path = "/data/test";
    uint32_t keyType = TYPE_EL2;

    std::vector<uint8_t> tokenVec{6, 7, 8};
    auth.token.Alloc(tokenVec.size());
    std::copy(tokenVec.begin(), tokenVec.end(), auth.token.data.get());
    elKey->keyContext_.shield.Clear();
    elKey->keyContext_.rndEnc.Clear();
    EXPECT_EQ(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    elKey->keyContext_.shield.Alloc(tokenVec.size());
    std::copy(tokenVec.begin(), tokenVec.end(), elKey->keyContext_.shield.data.get());
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    elKey->keyContext_.shield.Clear();
    elKey->keyContext_.rndEnc.Alloc(tokenVec.size());
    std::copy(tokenVec.begin(), tokenVec.end(), elKey->keyContext_.rndEnc.data.get());
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);

    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_003 end";
}

/**
 * @tc.name: BaseKey_DoRestoreKeyCeEceSece_004
 * @tc.desc: Verify the DoRestoreKeyCeEceSece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_DoRestoreKeyCeEceSece_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_004 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth auth;
    std::string path = "/data/test";
    uint32_t keyType = TYPE_EL2;

    std::vector<uint8_t> secretVec{1, 2, 3, 4, 5};
    auth.secret.Alloc(secretVec.size());
    std::copy(secretVec.begin(), secretVec.end(), auth.secret.data.get());
    EXPECT_NE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType), E_OK);
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyCeEceSece_004 end";
}

/**
 * @tc.name: BaseKey_GetKeyDir_000
 * @tc.desc: Verify the GetKeyDir function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_GetKeyDir_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_GetKeyDir_000 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    EXPECT_EQ(elKey->GetKeyDir(), "");
    elKey->dir_ = "/data/foo/bar/el1/100";
    EXPECT_EQ(elKey->GetKeyDir(), "el1");
    elKey->dir_ = "/data/foo/bar/el2/100";
    EXPECT_EQ(elKey->GetKeyDir(), "el2");
    elKey->dir_ = "/data/foo/bar/el3/100";
    EXPECT_EQ(elKey->GetKeyDir(), "el3");
    elKey->dir_ = "/data/foo/bar/el4/100";
    EXPECT_EQ(elKey->GetKeyDir(), "el4");
    elKey->dir_ = "/data/foo/bar/el5/100";
    EXPECT_EQ(elKey->GetKeyDir(), "el5");
    GTEST_LOG_(INFO) << "BaseKey_GetKeyDir_000 end";
}

/**
 * @tc.name: BaseKey_EncryptKeyBlob_000
 * @tc.desc: Verify the EncryptKeyBlob function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_EncryptKeyBlob_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_EncryptKeyBlob_000 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    KeyBlob planKey;
    KeyBlob encryptedKey;
    UserAuth auth;
    std::string path = "/data/test/test1";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));

    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey), E_SHIELD_OPERATION_ERROR);

    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(HKS_SUCCESS));
    EXPECT_EQ(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey), E_SHIELD_OPERATION_ERROR);

    KeyBlob keyOut;
    std::string test = "1234567890";
    string pathShield = path + PATH_SHIELD;
    std::string errMsg = "";
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(pathShield, test, errMsg));
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](KeyBlob &value) {
        std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
        value.Alloc(vecIn.size());
        std::copy(vecIn.begin(), vecIn.end(), value.data.get());
    })), Return(HKS_SUCCESS)));
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateRandomKey(_)).WillOnce(Return(keyOut));
    EXPECT_EQ(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey), E_SAVE_KEY_BLOB_ERROR);

    auto ret = unlink(pathShield.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DecryptKeyBlob_000! " << errno;
    OHOS::ForceRemoveDirectory(path);
    GTEST_LOG_(INFO) << "BaseKey_EncryptKeyBlob_000 end";
}

/**
 * @tc.name: BaseKey_EncryptKeyBlob_001
 * @tc.desc: Verify the EncryptKeyBlob function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_EncryptKeyBlob_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_EncryptKeyBlob_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    KeyBlob planKey;
    KeyBlob encryptedKey;
    UserAuth auth;
    std::string path = "/data/test/test1";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(path));

    std::string errMsg = "";
    std::vector<uint8_t> vec{1, 2, 3, 4, 5};
    KeyBlob keyOut(vec);
    string pathSecdisc = path + PATH_SECDISC;
    std::string testSec(CRYPTO_KEY_SECDISC_SIZE, 'c');
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(pathSecdisc, testSec, errMsg));
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](KeyBlob &value) {
        std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
        value.Alloc(vecIn.size());
        std::copy(vecIn.begin(), vecIn.end(), value.data.get());
    })), Return(E_OK)));
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateRandomKey(_)).WillOnce(Return(keyOut));
    EXPECT_CALL(*huksMasterMock_, EncryptKey(_, _, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey), E_ERR);

    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](KeyBlob &value) {
        std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
        value.Alloc(vecIn.size());
        std::copy(vecIn.begin(), vecIn.end(), value.data.get());
    })), Return(E_OK)));
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateRandomKey(_)).WillOnce(Return(keyOut));
    EXPECT_CALL(*huksMasterMock_, EncryptKey(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey), E_OK);

    auto ret = unlink(pathSecdisc.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_EncryptKeyBlob_000! " << errno;
    OHOS::ForceRemoveDirectory(path);
    GTEST_LOG_(INFO) << "BaseKey_EncryptKeyBlob_001 end";
}

/**
 * @tc.name: BaseKey_DecryptKeyBlob_000
 * @tc.desc: Verify the DecryptKeyBlob function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_DecryptKeyBlob_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DecryptKeyBlob_000 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    std::string path = "/data/test";
    uint32_t vecSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + 10;
    std::vector<uint8_t> vecIn(vecSize, 5);
    KeyBlob planKey(vecIn);
    UserAuth auth;
    KeyBlob decryptedKey;

    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(Return());
    EXPECT_EQ(elKey->DecryptKeyBlob(auth, path, planKey, decryptedKey), E_LOAD_KEY_BLOB_ERROR);

    std::string errMsg = "";
    std::string test = "1234567890";
    string pathShield = path + PATH_SHIELD;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(pathShield, test, errMsg));
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(Return());
    EXPECT_EQ(elKey->DecryptKeyBlob(auth, path, planKey, decryptedKey), E_LOAD_KEY_BLOB_ERROR);

    string pathSecdisc = path + PATH_SECDISC;
    std::string testSec(CRYPTO_KEY_SECDISC_SIZE, 'c');
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(pathSecdisc, testSec, errMsg));
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(Return());
    EXPECT_CALL(*huksMasterMock_, DecryptKey(_, _, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(elKey->DecryptKeyBlob(auth, path, planKey, decryptedKey), E_ERR);

    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(Return());
    EXPECT_CALL(*huksMasterMock_, DecryptKey(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DecryptKeyBlob(auth, path, planKey, decryptedKey), E_OK);

    auto ret = unlink(pathSecdisc.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DecryptKeyBlob_000! " << errno;
    GTEST_LOG_(INFO) << "BaseKey_DecryptKeyBlob_000 end";
}

/**
 * @tc.name: BaseKey_DoRestoreKeyOld_001
 * @tc.desc: Verify the DoRestoreKeyOld function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_DoRestoreKeyOld_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyOld_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    std::string path = "/data/test";
    std::string test = "1234567890";
    elKey->keyInfo_.version = FSCRYPT_INVALID;

    UserAuth auth;
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(elKey->DoRestoreKeyOld(auth, path), E_PARAMS_INVALID);

    elKey->dir_ = "el100/100";
    auth.secret.Alloc(1);
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(elKey->DoRestoreKeyOld(auth, path), E_PARAMS_INVALID);

    elKey->dir_ = "el1/101";
    const std::string needUpdataDir = elKey->GetDir() + PATH_LATEST;
    EXPECT_TRUE(OHOS::ForceCreateDirectory(needUpdataDir));
    string needUpdataPath = needUpdataDir + SUFFIX_NEED_UPDATE;
    std::string errMsg = "";
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(needUpdataPath, test, errMsg));
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_EQ(elKey->DoRestoreKeyOld(auth, path), E_PARAMS_INVALID);

    auth.secret.Clear();
    elKey->keyInfo_.version = FSCRYPT_INVALID;
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_EQ(elKey->DoRestoreKeyOld(auth, path), E_PARAMS_INVALID);

    elKey->keyInfo_.version = FSCRYPT_INVALID_NOT_SUPPORT;
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_EQ(elKey->DoRestoreKeyOld(auth, path), E_LOAD_KEY_BLOB_ERROR);
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyOld_001 end";
}

/**
 * @tc.name: BaseKey_DoRestoreKeyOld_002
 * @tc.desc: Verify the DoRestoreKeyOld function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_DoRestoreKeyOld_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyOld_002 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    elKey->dir_ = "el1/101";
    const std::string needUpdataDir = elKey->GetDir() + PATH_LATEST;
    EXPECT_TRUE(OHOS::ForceCreateDirectory(needUpdataDir));

    UserAuth auth;
    std::string errMsg = "";
    std::string test = "1234567890";
    string needUpdataPath = needUpdataDir + PATH_ENCRYPTED;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(needUpdataPath, test, errMsg));
    elKey->keyInfo_.version = FSCRYPT_INVALID_NOT_SUPPORT;
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_EQ(elKey->DoRestoreKeyOld(auth, needUpdataDir), E_LOAD_KEY_BLOB_ERROR);

    auth.secret.Alloc(1);
    needUpdataPath = needUpdataDir + SUFFIX_NEED_UPDATE;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(needUpdataPath, test, errMsg));
    std::string testSec(CRYPTO_KEY_SECDISC_SIZE, 'c');
    needUpdataPath = needUpdataDir + PATH_SECDISC;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(needUpdataPath, testSec, errMsg));
    elKey->keyInfo_.version = FSCRYPT_INVALID_NOT_SUPPORT;
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(elKey->DoRestoreKeyOld(auth, needUpdataDir), E_OK);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(elKey->dir_));
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyOld_002 end";
}

/**
 * @tc.name: BaseKey_InitKey_001
 * @tc.desc: Verify the InitKey function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_InitKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_InitKey_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    elKey->keyInfo_.version = FSCRYPT_INVALID;
    EXPECT_FALSE(elKey->BaseKey::InitKey(false));

    elKey->keyInfo_.version = FSCRYPT_INVALID_NOT_SUPPORT;
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID));
    EXPECT_FALSE(elKey->BaseKey::InitKey(false));

    elKey->keyInfo_.key.Clear();
    elKey->keyInfo_.key.Alloc(1);
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_FALSE(elKey->BaseKey::InitKey(false));

    KeyBlob keyOut;
    elKey->keyInfo_.key.Clear();
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_CALL(*huksMasterMock_, GenerateRandomKey(_)).WillOnce(Return(keyOut));
    EXPECT_FALSE(elKey->BaseKey::InitKey(true));

    keyOut.Alloc(1);
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_CALL(*huksMasterMock_, GenerateRandomKey(_)).WillOnce(Return(keyOut));
    EXPECT_TRUE(elKey->BaseKey::InitKey(true));

    elKey->keyInfo_.key.Clear();
    EXPECT_CALL(*keyControlMock_, KeyCtrlGetFscryptVersion(_)).WillOnce(Return(FSCRYPT_INVALID_NOT_SUPPORT));
    EXPECT_TRUE(elKey->BaseKey::InitKey(false));
    GTEST_LOG_(INFO) << "BaseKey_InitKey_001 end";
}

/**
 * @tc.name: BaseKey_DoStoreKey_001
 * @tc.desc: Verify the DoStoreKey function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_DoStoreKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoStoreKey_001 start";
    UserAuth user;
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    elKey->keyInfo_.version = FSCRYPT_INVALID_NOT_SUPPORT;
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(Return(true));
    EXPECT_EQ(elKey->BaseKey::DoStoreKey(user), E_VERSION_ERROR);

    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(Return(true));
    EXPECT_EQ(elKey->BaseKey::DoStoreKey(user), E_VERSION_ERROR);

    elKey->dir_ = "";
    elKey->keyInfo_.version = FSCRYPT_INVALID;
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(true)));
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(elKey->BaseKey::DoStoreKey(user), E_PARAMS_INVALID);

    elKey->dir_ = "el1/100";
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(true)));
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(elKey->BaseKey::DoStoreKey(user), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "BaseKey_DoStoreKey_001 end";
}

/**
 * @tc.name: BaseKey_CheckAndUpdateVersion_001
 * @tc.desc: Verify the CheckAndUpdateVersion function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_CheckAndUpdateVersion_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_CheckAndUpdateVersion_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    elKey->keyInfo_.version = FSCRYPT_INVALID;
    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(Return(true));
    EXPECT_FALSE(elKey->BaseKey::CheckAndUpdateVersion());

    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(true)));
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillRepeatedly(Return(false));
    EXPECT_TRUE(elKey->BaseKey::CheckAndUpdateVersion());

    elKey->dir_ = "el1/101";
    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->BaseKey::CheckAndUpdateVersion());

    elKey->dir_ = "";
    EXPECT_CALL(*commonUtilMoc_, LoadStringFromFile(_, _)).WillOnce(Return(false));
    EXPECT_TRUE(elKey->BaseKey::CheckAndUpdateVersion());
    GTEST_LOG_(INFO) << "BaseKey_CheckAndUpdateVersion_001 end";
}

/**
 * @tc.name: BaseKey_LoadAndSaveShield_001
 * @tc.desc: Verify the LoadAndSaveShield function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_LoadAndSaveShield_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_LoadAndSaveShield_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth auth;
    KeyContext keyCtx;
    string errMsg = "";
    string test = "1234567890";
#ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(elKey->BaseKey::LoadAndSaveShield(auth, "", true, keyCtx), E_PARAMS_INVALID);

    elKey->dir_ = "el1/101";
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(HKS_SUCCESS));
    EXPECT_EQ(elKey->BaseKey::LoadAndSaveShield(auth, "", true, keyCtx), E_SAVE_KEY_BLOB_ERROR);

    elKey->dir_ = "";
    EXPECT_EQ(elKey->BaseKey::LoadAndSaveShield(auth, "", false, keyCtx), E_LOAD_KEY_BLOB_ERROR);

    elKey->dir_ = "el1/101";
    const std::string needUpdataDir = elKey->GetDir() + PATH_LATEST;
    EXPECT_TRUE(OHOS::ForceCreateDirectory(needUpdataDir));
    string pathShield = needUpdataDir + PATH_SHIELD;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(pathShield, test, errMsg));
    EXPECT_EQ(elKey->BaseKey::LoadAndSaveShield(auth, "", false, keyCtx), E_SAVE_KEY_BLOB_ERROR);
#else
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(elKey->BaseKey::LoadAndSaveShield(auth, "", true, keyCtx), E_PARAMS_INVALID);
#endif
    keyCtx.shield.Clear();
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(HKS_SUCCESS));
    EXPECT_EQ(elKey->BaseKey::LoadAndSaveShield(auth, "", true, keyCtx), E_SAVE_KEY_BLOB_ERROR);

    keyCtx.shield.Alloc(1);
    elKey->dir_ = "el1/101";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(elKey->GetDir() + PATH_LATEST));
    string path = elKey->GetDir() + PATH_LATEST + PATH_SHIELD;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(path, test, errMsg));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(HKS_SUCCESS));
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillOnce(Return(false));
    EXPECT_EQ(elKey->BaseKey::LoadAndSaveShield(auth, path, true, keyCtx), E_OK);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(elKey->dir_));
    GTEST_LOG_(INFO) << "BaseKey_LoadAndSaveShield_001 end";
}

/**
 * @tc.name: BaseKey_InitKeyContext_001
 * @tc.desc: Verify the InitKeyContext function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_InitKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_InitKeyContext_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    UserAuth auth;
    string keyPath;
    KeyContext keyCtx;
    string errMsg = "";
    string test = "1234567890";

    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(elKey->BaseKey::InitKeyContext(auth, keyPath, keyCtx), E_PARAMS_INVALID);

    KeyBlob keyOut;
    keyOut.Clear();
    keyCtx.shield.Alloc(1);
    elKey->dir_ = "el1/101";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(elKey->GetDir() + PATH_LATEST));
    string path = elKey->GetDir() + PATH_LATEST + PATH_SHIELD;
    EXPECT_CALL(*fileUtilMoc_, ChMod(_, _)).WillRepeatedly(Return(false));
    ASSERT_TRUE(SaveStringToFileSync(path, test, errMsg));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(HKS_SUCCESS));
    EXPECT_EQ(elKey->BaseKey::InitKeyContext(auth, keyPath, keyCtx), E_GENERATE_DISCARD_ERROR);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory(elKey->dir_));
    GTEST_LOG_(INFO) << "BaseKey_InitKeyContext_001 end";
}

/**
 * @tc.name: BaseKey_KeyEncryptTypeToString_001
 * @tc.desc: Verify the KeyEncryptTypeToString function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_KeyEncryptTypeToString_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_KeyEncryptTypeToString_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    EXPECT_EQ(elKey->BaseKey::KeyEncryptTypeToString(BaseKey::KeyEncryptType::KEY_CRYPT_OPENSSL), "KEY_CRYPT_OPENSSL");
    EXPECT_EQ(elKey->BaseKey::KeyEncryptTypeToString(BaseKey::KeyEncryptType::KEY_CRYPT_HUKS), "KEY_CRYPT_HUKS");
    EXPECT_EQ(elKey->BaseKey::KeyEncryptTypeToString(BaseKey::KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL), 
        "KEY_CRYPT_HUKS_OPENSSL");

    GTEST_LOG_(INFO) << "BaseKey_KeyEncryptTypeToString_001 end";
}

/**
 * @tc.name: BaseKey_CombKeyCtx_001
 * @tc.desc: Verify the CombKeyCtx function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_CombKeyCtx_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_CombKeyCtx_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    KeyBlob nonce;
    KeyBlob rndEnc;
    KeyBlob aad;
    KeyBlob keyOut;
    EXPECT_FALSE(elKey->BaseKey::CombKeyCtx(nonce, rndEnc, aad, keyOut));

    nonce.Alloc(1);
    EXPECT_FALSE(elKey->BaseKey::CombKeyCtx(nonce, rndEnc, aad, keyOut));

    aad.Alloc(1);
    EXPECT_FALSE(elKey->BaseKey::CombKeyCtx(nonce, rndEnc, aad, keyOut));

    rndEnc.Alloc(1);
    keyOut.Alloc(1);
    EXPECT_TRUE(elKey->BaseKey::CombKeyCtx(nonce, rndEnc, aad, keyOut));
    GTEST_LOG_(INFO) << "BaseKey_CombKeyCtx_001 end";
}

/**
 * @tc.name: BaseKey_SplitKeyCtx_001
 * @tc.desc: Verify the SplitKeyCtx function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_SplitKeyCtx_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_SplitKeyCtx_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    KeyBlob keyIn;
    KeyBlob nonce;
    KeyBlob rndEnc;
    KeyBlob aad;
    nonce.Alloc(1);
    EXPECT_FALSE(elKey->BaseKey::SplitKeyCtx(keyIn, nonce, rndEnc, aad));

    nonce.Clear();
    EXPECT_TRUE(elKey->BaseKey::SplitKeyCtx(keyIn, nonce, rndEnc, aad));
    GTEST_LOG_(INFO) << "BaseKey_SplitKeyCtx_001 end";
}

/**
 * @tc.name: BaseKey_GetTypeFromDir_001
 * @tc.desc: Verify the GetTypeFromDir function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_GetTypeFromDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_GetTypeFromDir_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    elKey->dir_ = "";
    EXPECT_EQ(elKey->BaseKey::GetTypeFromDir(), TYPE_GLOBAL_EL1);

    elKey->dir_ = "/";
    EXPECT_EQ(elKey->BaseKey::GetTypeFromDir(), TYPE_GLOBAL_EL1);

    elKey->dir_ = "100/";
    EXPECT_EQ(elKey->BaseKey::GetTypeFromDir(), TYPE_GLOBAL_EL1);

    elKey->dir_ = "foo/el5/100";
    EXPECT_EQ(elKey->BaseKey::GetTypeFromDir(), TYPE_EL5);

    elKey->dir_ = "foo/el6/100";
    EXPECT_EQ(elKey->BaseKey::GetTypeFromDir(), TYPE_GLOBAL_EL1);
    GTEST_LOG_(INFO) << "BaseKey_GetTypeFromDir_001 end";
}

/**
 * @tc.name: BaseKey_GetIdFromDir_001
 * @tc.desc: Verify the GetIdFromDir function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_GetIdFromDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_GetIdFromDir_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    elKey->dir_ = "";
    EXPECT_EQ(elKey->BaseKey::GetIdFromDir(), USERID_GLOBAL_EL1);

    elKey->dir_ = "el2/100";
    EXPECT_EQ(elKey->BaseKey::GetIdFromDir(), 100);
    GTEST_LOG_(INFO) << "BaseKey_GetIdFromDir_001 end";
}

/**
 * @tc.name: BaseKey_GetHashKey_001
 * @tc.desc: Verify the GetHashKey function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(BaseKeyTest, BaseKey_GetHashKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_GetHashKey_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");

    KeyBlob hashKey;
    EXPECT_EQ(elKey->BaseKey::GetHashKey(hashKey), false);

    std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
    elKey->keyInfo_.keyHash.Clear();
    elKey->keyInfo_.keyHash.Alloc(vecIn.size());
    std::copy(vecIn.begin(), vecIn.end(), elKey->keyInfo_.keyHash.data.get());
    EXPECT_EQ(elKey->BaseKey::GetHashKey(hashKey), true);

    GTEST_LOG_(INFO) << "BaseKey_GetHashKey_001 end";
}

/**
 * @tc.name: BaseKey_GenerateHashKey_001
 * @tc.desc: Verify the GenerateHashKey function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(BaseKeyTest, BaseKey_GenerateHashKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_GenerateHashKey_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    EXPECT_EQ(elKey->BaseKey::GenerateHashKey(), false);

    std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
    elKey->keyInfo_.keyHash.Clear();
    elKey->keyInfo_.keyHash.Alloc(vecIn.size());
    std::copy(vecIn.begin(), vecIn.end(), elKey->keyInfo_.keyHash.data.get());

    elKey->keyInfo_.key.Clear();
    elKey->keyInfo_.key.Alloc(vecIn.size());
    std::copy(vecIn.begin(), vecIn.end(), elKey->keyInfo_.key.data.get());

    KeyBlob keyOut(vecIn);
    EXPECT_CALL(*opensslCryptoMock_, HashWithPrefix(_, _, _)).WillOnce(Return(keyOut));
    EXPECT_EQ(elKey->BaseKey::GenerateHashKey(), true);
    GTEST_LOG_(INFO) << "BaseKey_GenerateHashKey_001 end";
}

/**
 * @tc.name: BaseKey_RestoreKey_001
 * @tc.desc: Verify the RestoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(BaseKeyTest, BaseKey_RestoreKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_RestoreKey_001 start";
    std::vector<std::string> fileNames = {
        "latest",
        "version_1000",
        "version_1001",
        "invalidPath1",
        "invalidPath2",
    };

    UserAuth auth;
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    EXPECT_CALL(*keyControlMock_, KeyCtrlLoadVersion(_)).WillRepeatedly(Return(FSCRYPT_INVALID));
    EXPECT_CALL(*fileUtilMoc_, GetSubDirs(_, _)).WillOnce(SetArgReferee<1>(fileNames))
        .WillOnce(SetArgReferee<1>(fileNames));
    EXPECT_EQ(elKey->BaseKey::RestoreKey(auth), E_VERSION_ERROR);
    GTEST_LOG_(INFO) << "BaseKey_RestoreKey_001 end";
}
} // OHOS::StorageDaemon