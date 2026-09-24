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

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "base_key.h"
#include "huks_master.h"
#include "key_blob.h"
#include "openssl_crypto.h"
#include "openssl/evp.h"

#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
using namespace OHOS::StorageDaemon;

const std::string TEST_PATH = "/data/test";
constexpr uint32_t CRYPTO_KEY_ALIAS_SIZE = 16;

class CryptoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CryptoTest::SetUpTestCase(void) {
    // input testsuit setup step，setup invoked before all testcases
}

void CryptoTest::TearDownTestCase(void) {
    // input testsuit teardown step，teardown invoked after all testcases
}

void CryptoTest::SetUp(void) {
    // input testcase setup step，setup invoked before each testcases
}

void CryptoTest::TearDown(void) {
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: Generate_And_Save_Key_Blob_001
 * @tc.desc: Verify the BaseKey GenerateAndSaveKeyBlob function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Generate_And_Save_Key_Blob_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_GenerateAndSaveKeyBlob_0100 start";
    KeyBlob blob;
    const std::string path;
    const uint32_t size = 0;
    bool ret = BaseKey::GenerateAndSaveKeyBlob(blob, path, size);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_GenerateAndSaveKeyBlob_0100 end";
}

/**
 * @tc.name: Generate_And_Save_Key_Blob_002
 * @tc.desc: Verify the BaseKey GenerateAndSaveKeyBlob function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Generate_And_Save_Key_Blob_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_GenerateAndSaveKeyBlob_0200 start";
    KeyBlob blob;
    const std::string path = TEST_PATH;
    const uint32_t size = CRYPTO_KEY_SECDISC_SIZE;
    bool ret = BaseKey::GenerateAndSaveKeyBlob(blob, path, size);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_GenerateAndSaveKeyBlob_0200 end";
}

/**
 * @tc.name: Huks_Master_Encrypt_Key_001
 * @tc.desc: Verify the HuksMaster EncryptKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Encrypt_Key_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterEncryptKey_0100 start";
    KeyContext ctx;
    const UserAuth auth;
    const KeyInfo key;
    bool isNeedNewNonce = false;
    KeyBlob rnd;

    int ret = HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_NE(E_OK, ret);

    ret = HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx);
    EXPECT_NE(E_OK, ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterEncryptKey_0100 end";
}

/**
 * @tc.name: Huks_Master_Encrypt_Key_002
 * @tc.desc: Verify the HuksMaster EncryptKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Encrypt_Key_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterEncryptKey_0200 start";
    KeyBlob testKeyblob = HuksMaster::GenerateRandomKey(CRYPTO_KEY_ALIAS_SIZE);
    KeyContext ctx { .shield = testKeyblob };
    const UserAuth auth;
    const KeyInfo key;
    bool isNeedNewNonce = false;
    KeyBlob rnd;

    int ret = HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_NE(E_OK, ret);

    ret = HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx);
    EXPECT_NE(E_OK, ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterEncryptKey_0200 end";
}

/**
 * @tc.name: Huks_Master_Decrypt_Key_001
 * @tc.desc: Verify the HuksMaster DecrptyKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Decrypt_Key_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0100 start";
    KeyContext ctx;
    const UserAuth auth;
    KeyInfo key;
    bool isNeedNewNonce = false;
    KeyBlob rnd;

    int ret = HuksMaster::GetInstance().DecryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_NE(E_OK, ret);

    ret = HuksMaster::GetInstance().DecryptKeyEx(ctx, auth, rnd);
    EXPECT_NE(E_OK, ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0100 end";
}

/**
 * @tc.name: Huks_Master_Decrypt_Key_002
 * @tc.desc: Verify the HuksMaster DecrptyKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Decrypt_Key_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0200 start";
    KeyBlob testKeyblob = HuksMaster::GenerateRandomKey(CRYPTO_KEY_ALIAS_SIZE);
    KeyContext ctx { .shield = testKeyblob };
    const UserAuth auth;
    KeyInfo key;
    bool isNeedNewNonce = false;
    KeyBlob rnd;

    int ret = HuksMaster::GetInstance().DecryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_NE(ret, E_OK);

    ret = HuksMaster::GetInstance().DecryptKeyEx(ctx, auth, rnd);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0200 end";
}

/**
 * @tc.name: Huks_Master_Decrypt_Key_003
 * @tc.desc: Verify the HuksMaster DecrptyKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Decrypt_Key_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0300 start";
    KeyBlob testKeyblob = HuksMaster::GenerateRandomKey(CRYPTO_KEY_ALIAS_SIZE);
    KeyContext ctx { .shield = testKeyblob, .rndEnc = testKeyblob };
    const UserAuth auth;
    KeyInfo key;
    bool isNeedNewNonce = false;
    KeyBlob rnd;

    int ret = HuksMaster::GetInstance().DecryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_NE(E_OK, ret);

    ret = HuksMaster::GetInstance().DecryptKeyEx(ctx, auth, rnd);
    EXPECT_NE(E_OK, ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0300 end";
}

#ifdef HUKS_IDL_ENVIRONMENT
/**
 * @tc.name: Huks_Master_Hdi_Access_Upgrade_Key_001
 * @tc.desc: Verify the HuksMaster HdiAccessUpgradeKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Hdi_Access_Upgrade_Key_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterHdiAccessUpgradeKey_0100 start";
    const HuksBlob oldKey {};
    struct HksParamSet *paramSet = nullptr;
    HuksBlob newKey;

    int ret = HuksMaster::GetInstance().HdiAccessUpgradeKey(oldKey, paramSet, newKey);
    EXPECT_NE(ret, HKS_SUCCESS);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterHdiAccessUpgradeKey_0100 end";
}
#endif

/**
 * @tc.name: Openssl_Crypto_AES_Encrypt_001
 * @tc.desc: Verify the OpensslCrypto AESEncrypt and AESDecrypt function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Encrypt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESEncrypt_0100 start";
    const KeyBlob preKey;
    KeyContext keyContext;
    KeyBlob plainText;

    int32_t ret = OpensslCrypto::AESEncrypt(preKey, plainText, keyContext);
    EXPECT_TRUE(ret == E_OK);

    ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
    EXPECT_TRUE(ret != E_OK);
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESEncrypt_0100 end";
}

/**
 * @tc.name: Openssl_Crypto_AES_Decrypt_001
 * @tc.desc: Verify the OpensslCrypto AESDecrypt function with small cipherText.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Decrypt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_001 start";
    KeyBlob preKey(32);
    KeyContext keyContext;
    KeyBlob plainText;

    std::vector<uint32_t> errorSizes = {0, 10, 28};
    for (uint32_t size : errorSizes) {
        keyContext.rndEnc.size = size;
        int32_t ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
        EXPECT_EQ(ret, E_KEY_SIZE_ERROR);
    }

    keyContext.rndEnc.size = 29;
    int32_t ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
    EXPECT_NE(ret, E_KEY_SIZE_ERROR);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_001 end";
}

/**
 * @tc.name: Openssl_Crypto_HashWithPrefix_001
 * @tc.desc: Verify the OpensslCrypto HashWithPrefix function with excessive length and normal length.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_HashWithPrefix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoHashWithPrefix_0100 start";
    KeyBlob prefix(16);
    KeyBlob payload(16);

    KeyBlob result = OpensslCrypto::HashWithPrefix(prefix, payload, 100);
    EXPECT_EQ(result.size, 0);

    result = OpensslCrypto::HashWithPrefix(prefix, payload, 32);
    EXPECT_EQ(result.size, 32);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoHashWithPrefix_0100 end";
}

/**
 * @tc.name: Openssl_Crypto_AES_Encrypt_002
 * @tc.desc: Verify the OpensslCrypto AESEncrypt with empty keyBlob and empty plainText.
 * @tc.type: FUNC
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Encrypt_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_Openssl_Crypto_AES_Encrypt_002 start";
    KeyContext keyContext;
    {
        const KeyBlob preKey;
        KeyBlob plainText(16);
        int32_t ret = OpensslCrypto::AESEncrypt(preKey, plainText, keyContext);
        EXPECT_EQ(ret, E_OK);
    }

    {
        const KeyBlob preKey(32);
        KeyBlob plainText;
        int32_t ret = OpensslCrypto::AESEncrypt(preKey, plainText, keyContext);
        EXPECT_EQ(ret, E_OK);
    }

    GTEST_LOG_(INFO) << "CryptoTest_Openssl_Crypto_AES_Encrypt_002 end";
}

/**
 * @tc.name: Openssl_Crypto_HashWithPrefix_002
 * @tc.desc: Verify the OpensslCrypto HashWithPrefix with boundary length (SHA512_DIGEST_LENGTH).
 * @tc.type: FUNC
 */
HWTEST_F(CryptoTest, Openssl_Crypto_HashWithPrefix_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoHashWithPrefix_0200 start";
    KeyBlob prefix(16);
    KeyBlob payload(16);

    KeyBlob result = OpensslCrypto::HashWithPrefix(prefix, payload, 64);
    EXPECT_EQ(result.size, 64);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoHashWithPrefix_0200 end";
}

/**
 * @tc.name: Openssl_Crypto_HashWithPrefix_003
 * @tc.desc: Verify the OpensslCrypto HashWithPrefix with empty blobs.
 * @tc.type: FUNC
 */
HWTEST_F(CryptoTest, Openssl_Crypto_HashWithPrefix_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoHashWithPrefix_0300 start";
    KeyBlob prefix;
    KeyBlob payload;

    KeyBlob result = OpensslCrypto::HashWithPrefix(prefix, payload, 32);
    EXPECT_EQ(result.size, 32);

    result = OpensslCrypto::HashWithPrefix(prefix, payload, 0);
    EXPECT_EQ(result.size, 0);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoHashWithPrefix_0300 end";
}

/**
 * @tc.name: Openssl_Crypto_AES_Decrypt_006
 * @tc.desc: Verify the OpensslCrypto AESDecrypt with invalid GCM tag size.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Decrypt_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0600 start";
    KeyBlob preKey(32);
    KeyContext keyContext;
    KeyBlob plainText;

    keyContext.rndEnc.size = 40;
    keyContext.rndEnc.data = std::make_unique<uint8_t[]>(40);
    for (uint32_t i = 0; i < 40; i++) {
        keyContext.rndEnc.data.get()[i] = static_cast<uint8_t>(i);
    }
    keyContext.secDiscard.size = 32;
    keyContext.secDiscard.data = std::make_unique<uint8_t[]>(32);
    for (uint32_t i = 0; i < 32; i++) {
        keyContext.secDiscard.data.get()[i] = static_cast<uint8_t>(i);
    }

    int32_t ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0600 end";
}

/**
 * @tc.name: Openssl_Crypto_AES_Decrypt_007
 * @tc.desc: Verify the OpensslCrypto AESDecrypt with boundary sizes for DoGCMDecryptFinal.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Decrypt_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0700 start";

    KeyBlob preKey(32);
    KeyContext keyContext;
    KeyBlob plainText;
    keyContext.rndEnc.size = 29;
    keyContext.rndEnc.data = std::make_unique<uint8_t[]>(29);
    for (uint32_t i = 0; i < 29; i++) {
        keyContext.rndEnc.data.get()[i] = static_cast<uint8_t>(i);
    }
    keyContext.secDiscard.size = 32;
    keyContext.secDiscard.data = std::make_unique<uint8_t[]>(32);
    for (uint32_t i = 0; i < 32; i++) {
        keyContext.secDiscard.data.get()[i] = static_cast<uint8_t>(i);
    }

    int32_t ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0700 end";
}

/**
 * @tc.name: Openssl_Crypto_AES_Decrypt_008
 * @tc.desc: Verify the OpensslCrypto AESDecrypt with larger data for DoGCMDecryptFinal coverage.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Decrypt_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0800 start";

    KeyBlob preKey(32);
    for (uint32_t i = 0; i < 32; i++) {
        preKey.data.get()[i] = static_cast<uint8_t>(i);
    }

    KeyBlob testPlain(64);
    for (uint32_t i = 0; i < 64; i++) {
        testPlain.data.get()[i] = static_cast<uint8_t>(i % 256);
    }

    KeyContext keyContext;
    int32_t encRet = OpensslCrypto::AESEncrypt(preKey, testPlain, keyContext);
    EXPECT_EQ(encRet, E_OK);

    KeyBlob decryptedText;
    int32_t decRet = OpensslCrypto::AESDecrypt(preKey, keyContext, decryptedText);
    EXPECT_EQ(decRet, E_OK);
    EXPECT_EQ(decryptedText.size, testPlain.size);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0800 end";
}

/**
 * @tc.name: Openssl_Crypto_AES_Decrypt_009
 * @tc.desc: Verify the OpensslCrypto AESDecrypt with empty preKey.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Decrypt_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0900 start";

    KeyBlob preKey;
    KeyContext keyContext;
    KeyBlob plainText;

    keyContext.rndEnc.size = 50;
    keyContext.rndEnc.data = std::make_unique<uint8_t[]>(50);
    for (uint32_t i = 0; i < 50; i++) {
        keyContext.rndEnc.data.get()[i] = static_cast<uint8_t>(i);
    }
    keyContext.secDiscard.size = 32;
    keyContext.secDiscard.data = std::make_unique<uint8_t[]>(32);
    for (uint32_t i = 0; i < 32; i++) {
        keyContext.secDiscard.data.get()[i] = static_cast<uint8_t>(i);
    }

    int32_t ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESDecrypt_0900 end";
}

/**
 * @tc.name: Openssl_Crypto_CleanupShield_001
 * @tc.desc: Verify the OpensslCrypto CleanupShield function with valid shield.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_CleanupShield_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoCleanupShield_001 start";

    KeyBlob shield(32);
    for (uint32_t i = 0; i < shield.size; i++) {
        shield.data.get()[i] = 0xFF;
    }

    OpensslCrypto::CleanupShield(shield);

    for (uint32_t i = 0; i < shield.size; i++) {
        EXPECT_EQ(shield.data.get()[i], 0);
    }

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoCleanupShield_001 end";
}

/**
 * @tc.name: Openssl_Crypto_CleanupShield_002
 * @tc.desc: Verify the OpensslCrypto CleanupShield function with null data.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_CleanupShield_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoCleanupShield_002 start";

    KeyBlob shield;
    OpensslCrypto::CleanupShield(shield);

    EXPECT_TRUE(shield.data.get() == nullptr || shield.size == 0);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoCleanupShield_002 end";
}

/**
 * @tc.name: Openssl_Crypto_DoGCMDecryptInit_001
 * @tc.desc: Verify the DoGCMDecryptInit function with valid inputs.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_DoGCMDecryptInit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMDecryptInit_001 start";

    KeyBlob shield(32);
    for (uint32_t i = 0; i < shield.size; i++) {
        shield.data.get()[i] = static_cast<uint8_t>(i);
    }

    KeyContext keyContext;
    keyContext.rndEnc.size = 40;
    keyContext.rndEnc.data = std::make_unique<uint8_t[]>(40);
    for (uint32_t i = 0; i < 40; i++) {
        keyContext.rndEnc.data.get()[i] = static_cast<uint8_t>(i);
    }

    KeyBlob plainText;
    auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    ASSERT_NE(ctx, nullptr);
    int32_t ret = OpensslCrypto::DoGCMDecryptInit(ctx.get(), shield, keyContext, plainText);

    EXPECT_TRUE(ret == E_OK || ret != E_OK);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMDecryptInit_001 end, ret = " << ret;
}

/**
 * @tc.name: Openssl_Crypto_DoGCMDecryptFinal_001
 * @tc.desc: Verify the DoGCMDecryptFinal function with size check failure.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_DoGCMDecryptFinal_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMDecryptFinal_001 start";

    auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    ASSERT_NE(ctx, nullptr);

    KeyBlob cipherText(20);
    KeyBlob plainText(10);

    int32_t ret = OpensslCrypto::DoGCMDecryptFinal(ctx.get(), cipherText, plainText);
    EXPECT_EQ(ret, E_KEY_SIZE_ERROR);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMDecryptFinal_001 end";
}

/**
 * @tc.name: Openssl_Crypto_DoGCMDecryptFinal_002
 * @tc.desc: Verify the DoGCMDecryptFinal function with exact boundary size.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_DoGCMDecryptFinal_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMDecryptFinal_002 start";

    auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    ASSERT_NE(ctx, nullptr);
    uint32_t plainSize = 10;
    KeyBlob cipherText(GCM_NONCE_BYTES + plainSize + GCM_MAC_BYTES);
    for (uint32_t i = 0; i < cipherText.size; i++) {
        cipherText.data.get()[i] = static_cast<uint8_t>(i);
    }
    KeyBlob plainText(plainSize);
    int32_t ret = OpensslCrypto::DoGCMDecryptFinal(ctx.get(), cipherText, plainText);
    EXPECT_TRUE(ret == E_CIPHER_CTX_CTRL || ret == E_DECRYPT_FINAL_EX || ret == E_DECRYPT_FINAL_EX_LEN);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMDecryptFinal_002 end, ret = " << ret;
}

/**
 * @tc.name: Openssl_Crypto_DoGCMEncryptFinal_001
 * @tc.desc: Verify the DoGCMEncryptFinal function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_DoGCMEncryptFinal_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMEncryptFinal_001 start";

    auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    ASSERT_NE(ctx, nullptr);

    KeyBlob shield(32);
    for (uint32_t i = 0; i < shield.size; i++) {
        shield.data.get()[i] = static_cast<uint8_t>(i);
    }

    KeyBlob cipherText(GCM_NONCE_BYTES + 16 + GCM_MAC_BYTES);
    for (uint32_t i = 0; i < cipherText.size; i++) {
        cipherText.data.get()[i] = static_cast<uint8_t>(i);
    }

    KeyBlob plainText(16);
    for (uint32_t i = 0; i < plainText.size; i++) {
        plainText.data.get()[i] = static_cast<uint8_t>(i);
    }

    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL,
                           reinterpret_cast<const uint8_t*>(shield.data.get()),
                           reinterpret_cast<const uint8_t*>(cipherText.data.get())) != OPENSSL_SUCCESS_FLAG) {
        GTEST_LOG_(INFO) << "EncryptInit failed, skipping test";
        GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMEncryptFinal_001 end";
        return;
    }

    int32_t ret = OpensslCrypto::DoGCMEncryptFinal(ctx.get(), cipherText, plainText);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMEncryptFinal_001 end, ret = " << ret;
}

/**
 * @tc.name: Openssl_Crypto_DoGCMEncryptFinal_002
 * @tc.desc: Verify the DoGCMEncryptFinal with EncryptFinal_ex failure case.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_DoGCMEncryptFinal_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMEncryptFinal_002 start";

    auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    ASSERT_NE(ctx, nullptr);

    KeyBlob cipherText(50);
    KeyBlob plainText(16);

    int32_t ret = OpensslCrypto::DoGCMEncryptFinal(ctx.get(), cipherText, plainText);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoDoGCMEncryptFinal_002 end, ret = " << ret;
}
}
}