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

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
using namespace OHOS::StorageDaemon;

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
 * @tc.name: base_key_generate_and_save_key_blob
 * @tc.desc: Verify the BaseKey GenerateAndSaveKeyBlob
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
    EXPECT_FALSE(ret)
    GTEST_LOG_(INFO) << "CryptoTest_GenerateAndSaveKeyBlob_0100 end";
}

/**
 * @tc.name: huks_master_encrypt_key
 * @tc.desc: Verify the HuksMaster EncryptKey
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

    bool ret = HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_FALSE(ret);

    ret = HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterEncryptKey_0100 end";
}

/**
 * @tc.name: huks_master_decrypt_key
 * @tc.desc: Verify the HuksMaster DecrptyKey
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

    bool ret = HuksMaster::GetInstance().DecryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_FALSE(ret);

    ret = HuksMaster::GetInstance().DecryptKeyEx(ctx, auth, rnd);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0100 end";
}

/**
 * @tc.name: openssl_crypto_aes_encrypt
 * @tc.desc: Verify the OpensslCrypto AESEncrypt and AESDecrypt
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Openssl_Crypto_AES_Encrypt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESEncrypt_0100 start";
    const KeyBlob preKey;
    KeyContext keyContext;
    KeyBlob plainText;

    bool ret = OpensslCrypto::AESEncrypt(preKey, plainText, keyContext);
    EXPECT_TRUE(ret);

    ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESEncrypt_0100 end";
}
}
}