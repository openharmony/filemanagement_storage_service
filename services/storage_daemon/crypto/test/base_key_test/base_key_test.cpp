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

#include "fbex.h"
#include "fscrypt_key_v2.h"
#include "huks_master_mock.h"
#include "key_blob.h"
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
};

void BaseKeyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    huksMasterMock_ = make_shared<HuksMasterMock>();
    HuksMasterMock::huksMasterMock = huksMasterMock_;
    opensslCryptoMock_ = make_shared<OpensslCryptoMock>();
    OpensslCryptoMock::opensslCryptoMock = opensslCryptoMock_;
}

void BaseKeyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    OpensslCryptoMock::opensslCryptoMock = nullptr;
    opensslCryptoMock_ = nullptr;
    HuksMasterMock::huksMasterMock = nullptr;
    huksMasterMock_ = nullptr;
}

void BaseKeyTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void BaseKeyTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
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
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->Decrypt(emptyUserAuth));

    elKey->keyEncryptType_ = BaseKey::KeyEncryptType::KEY_CRYPT_HUKS;
    EXPECT_CALL(*huksMasterMock_, DecryptKey(_, _, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->Decrypt(emptyUserAuth));

    elKey->keyEncryptType_ = BaseKey::KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL;
    EXPECT_FALSE(elKey->Decrypt(emptyUserAuth));

    int encryptType = 4;
    elKey->keyEncryptType_ = static_cast<BaseKey::KeyEncryptType>(encryptType);
    EXPECT_FALSE(elKey->Decrypt(emptyUserAuth));
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
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx));

    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx));

    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx));

    keyType = TYPE_EL3;
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx));

    keyType = TYPE_EL4;
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DecryptReal(emptyUserAuth, keyType, keyCtx));
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
    std::string keyPath = "/test";
    bool needSyncCandidate = false;
    EXPECT_TRUE(elKey->UpdateKey(keyPath, needSyncCandidate));

    needSyncCandidate = true;
    EXPECT_FALSE(elKey->UpdateKey(keyPath, needSyncCandidate));
    
    keyPath = "";
    EXPECT_FALSE(elKey->UpdateKey(keyPath, needSyncCandidate));

    elKey->dir_ = "/data/foo/bar/el5/100";
    EXPECT_TRUE(elKey->UpdateKey(keyPath, needSyncCandidate));
    GTEST_LOG_(INFO) << "BaseKey_UpdateKey_001 end";
}

/**
 * @tc.name: BaseKey_DoLatestBackUp_001
 * @tc.desc: Verify the DoLatestBackUp function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(BaseKeyTest, BaseKey_DoLatestBackUp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoLatestBackUp_001 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    elKey->DoLatestBackUp();

    std::string pathLatest = elKey->GetDir() + PATH_LATEST;
    EXPECT_TRUE(OHOS::ForceCreateDirectory(pathLatest));
    std::string pathLatestBak = elKey->GetDir() + PATH_LATEST_BACKUP;
    elKey->DoLatestBackUp();
    EXPECT_FALSE(access(pathLatest.c_str(), F_OK) == 0);
    EXPECT_TRUE(access(pathLatestBak.c_str(), F_OK) == 0);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(pathLatestBak));
    GTEST_LOG_(INFO) << "BaseKey_DoLatestBackUp_001 end";
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
    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->EncryptEceSece(auth, keyType, keyCtx));

    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->EncryptEceSece(auth, keyType, keyCtx));

    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->EncryptEceSece(auth, keyType, keyCtx));
    EXPECT_EQ(elKey->keyEncryptType_, BaseKey::KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL);

    keyType = TYPE_EL3;
    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->EncryptEceSece(auth, keyType, keyCtx));

    keyType = TYPE_EL4;
    EXPECT_CALL(*huksMasterMock_, EncryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*opensslCryptoMock_, AESEncrypt(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->EncryptEceSece(auth, keyType, keyCtx));
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
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    std::string test = "1234567890";
    string pathEncrypt = path + PATH_ENCRYPTED;
    ASSERT_TRUE(SaveStringToFileSync(pathEncrypt, test));
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));
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
    ASSERT_TRUE(SaveStringToFileSync(pathEncrypt, rightContent));
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    string pathSecdisc = path + PATH_SECDISC;
    ASSERT_TRUE(SaveStringToFileSync(path + PATH_SECDISC, test));
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));
    
    string pathShield = path + PATH_SHIELD;
    ASSERT_TRUE(SaveStringToFileSync(path + PATH_SHIELD, test));
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    std::vector<uint8_t> secretVec{1, 2, 3, 4, 5};
    std::vector<uint8_t> tokenVec{6, 7, 8};
    auth.secret.Alloc(secretVec.size());
    std::copy(secretVec.begin(), secretVec.end(), auth.secret.data.get());
    auth.token.Alloc(tokenVec.size());
    std::copy(tokenVec.begin(), tokenVec.end(), auth.token.data.get());
    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

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
    EXPECT_TRUE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    elKey->keyContext_.shield.Alloc(tokenVec.size());
    std::copy(tokenVec.begin(), tokenVec.end(), elKey->keyContext_.shield.data.get());
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    elKey->keyContext_.shield.Clear();
    elKey->keyContext_.rndEnc.Alloc(tokenVec.size());
    std::copy(tokenVec.begin(), tokenVec.end(), elKey->keyContext_.rndEnc.data.get());
    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));

    EXPECT_CALL(*huksMasterMock_, DecryptKeyEx(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));
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
    EXPECT_FALSE(elKey->DoRestoreKeyCeEceSece(auth, path, keyType));
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
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey));
    
    std::vector<uint8_t> vec{1, 2, 3, 4, 5};
    KeyBlob keyOut(vec);
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(Return(true));
    EXPECT_FALSE(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey));

    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](KeyBlob &value) {
        std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
        value.Alloc(vecIn.size());
        std::copy(vecIn.begin(), vecIn.end(), value.data.get());
    })), Return(true)));
    std::string test = "1234567890";
    string pathShield = path + PATH_SHIELD;
    ASSERT_TRUE(SaveStringToFileSync(pathShield, test));
    EXPECT_CALL(*huksMasterMock_, GenerateRandomKey(_)).WillOnce(Return(keyOut));
    EXPECT_CALL(*huksMasterMock_, EncryptKey(_, _, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey));
    
    string pathSecdisc = path + PATH_SECDISC;
    std::string testSec(CRYPTO_KEY_SECDISC_SIZE, 'c');
    ASSERT_TRUE(SaveStringToFileSync(pathSecdisc, testSec));
    EXPECT_CALL(*huksMasterMock_, GenerateKey(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](KeyBlob &value) {
        std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
        value.Alloc(vecIn.size());
        std::copy(vecIn.begin(), vecIn.end(), value.data.get());
    })), Return(true)));
    EXPECT_CALL(*huksMasterMock_, GenerateRandomKey(_)).WillOnce(Return(keyOut));
    EXPECT_CALL(*huksMasterMock_, EncryptKey(_, _, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->EncryptKeyBlob(auth, path, planKey, encryptedKey));
    auto ret = unlink(pathShield.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_EncryptKeyBlob_000! " << errno;
    ret = unlink(pathSecdisc.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_EncryptKeyBlob_000! " << errno;
    OHOS::ForceRemoveDirectory(path);
    GTEST_LOG_(INFO) << "BaseKey_EncryptKeyBlob_000 end";
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
    string pathShield = path + PATH_SHIELD;
    std::string test = "1234567890";
    ASSERT_TRUE(SaveStringToFileSync(pathShield, test));

    uint32_t vecSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + 10;
    std::vector<uint8_t> vecIn(vecSize, 5);
    KeyBlob planKey(vecIn);
    UserAuth auth;
    KeyBlob decryptedKey;
    EXPECT_FALSE(elKey->DecryptKeyBlob(auth, path, planKey, decryptedKey));

    string pathSecdisc = path + PATH_SECDISC;
    std::string testSec(CRYPTO_KEY_SECDISC_SIZE, 'c');
    ASSERT_TRUE(SaveStringToFileSync(pathSecdisc, testSec));
    EXPECT_CALL(*huksMasterMock_, DecryptKey(_, _, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(elKey->DecryptKeyBlob(auth, path, planKey, decryptedKey));

    EXPECT_CALL(*huksMasterMock_, DecryptKey(_, _, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DecryptKeyBlob(auth, path, planKey, decryptedKey));

    auto ret = unlink(pathShield.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DecryptKeyBlob_000! " << errno;
    ret = unlink(pathSecdisc.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DecryptKeyBlob_000! " << errno;
    GTEST_LOG_(INFO) << "BaseKey_DecryptKeyBlob_000 end";
}

/**
 * @tc.name: BaseKey_DoRestoreKeyOld_000
 * @tc.desc: Verify the DoRestoreKeyOld function.
 * @tc.type: FUNC
 * @tc.require: IAXJFK
 */
HWTEST_F(BaseKeyTest, BaseKey_DoRestoreKeyOld_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyOld_000 start";
    std::shared_ptr<FscryptKeyV2> elKey = std::make_shared<FscryptKeyV2>("/data/test");
    std::string path = "/data/test";
    std::string test = "1234567890";
    elKey->keyInfo_.version = FSCRYPT_INVALID;

    UserAuth auth;
    EXPECT_FALSE(elKey->DoRestoreKeyOld(auth, path));

    elKey->keyInfo_.version = KeyCtrlLoadVersion(elKey->GetDir().c_str());
    EXPECT_FALSE(elKey->DoRestoreKeyOld(auth, path));

    string pathEncrpted = path + PATH_ENCRYPTED;
    ASSERT_TRUE(SaveStringToFileSync(pathEncrpted, test));
    EXPECT_FALSE(elKey->DoRestoreKeyOld(auth, path));

    string pathShield = path + PATH_SHIELD;
    ASSERT_TRUE(SaveStringToFileSync(pathShield, test));
    EXPECT_FALSE(elKey->DoRestoreKeyOld(auth, path));

    string pathSecdisc = path + PATH_SECDISC;
    std::string testSec(CRYPTO_KEY_SECDISC_SIZE, 'c');
    ASSERT_TRUE(SaveStringToFileSync(pathSecdisc, testSec));
    EXPECT_CALL(*huksMasterMock_, DecryptKey(_, _, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DoRestoreKeyOld(auth, path));

    auto ret = unlink(pathShield.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyOld_000! " << errno;
    ret = unlink(pathSecdisc.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyOld_000! " << errno;
    ret = unlink(pathEncrpted.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyOld_000! " << errno;
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyOld_000 end";
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
    EXPECT_FALSE(elKey->DoRestoreKeyOld(auth, path));

    std::vector<uint8_t> vec{1, 2, 3, 4, 5};
    auth.secret.Alloc(vec.size());
    std::copy(vec.begin(), vec.end(), auth.secret.data.get());
    EXPECT_FALSE(elKey->DoRestoreKeyOld(auth, path));
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
    const std::string needUpdataDir = elKey->GetDir() + PATH_LATEST;
    EXPECT_TRUE(OHOS::ForceCreateDirectory(needUpdataDir));

    std::string test = "1234567890";
    string needUpdataPath = needUpdataDir + SUFFIX_NEED_UPDATE;
    ASSERT_TRUE(SaveStringToFileSync(needUpdataPath, test));
    elKey->keyInfo_.version = KeyCtrlLoadVersion(elKey->GetDir().c_str());
    
    std::string path = "/data/test";
    string pathEncrpted = path + PATH_ENCRYPTED;
    ASSERT_TRUE(SaveStringToFileSync(pathEncrpted, test));
    string pathSecdisc = path + PATH_SECDISC;
    std::string testSec(CRYPTO_KEY_SECDISC_SIZE, 'c');
    ASSERT_TRUE(SaveStringToFileSync(pathSecdisc, testSec));

    UserAuth auth;
    std::vector<uint8_t> vec{1, 2, 3, 4, 5};
    auth.secret.Alloc(vec.size());
    std::copy(vec.begin(), vec.end(), auth.secret.data.get());

    EXPECT_CALL(*opensslCryptoMock_, AESDecrypt(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(elKey->DoRestoreKeyOld(auth, path));
    auto ret = unlink(needUpdataPath.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyOld_002! " << errno;
    ret = unlink(pathSecdisc.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyOld_002! " << errno;
    ret = unlink(pathEncrpted.c_str());
    ASSERT_TRUE(ret != -1) << "Failed to delete file in BaseKey_DoRestoreKeyOld_002! " << errno;
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(needUpdataDir));
    GTEST_LOG_(INFO) << "BaseKey_DoRestoreKeyOld_002 end";
}
} // OHOS::StorageDaemon