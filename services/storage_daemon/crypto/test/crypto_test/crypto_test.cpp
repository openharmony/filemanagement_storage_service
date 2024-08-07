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

#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

#include "base_key.h"
#include "huks_master.h"
#include "key_blob.h"
#include "openssl_crypto.h"

#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
using namespace OHOS::StorageDaemon;

const int32_t PARAMS_SIZE_0 = 0;
const int32_t PARAMS_SIZE_1 = 1;
const int32_t PARAMS_SIZE_2 = 2;
const int32_t PARAMS_SIZE_3 = 3;
const int32_t PARAMS_SIZE_4 = 4;

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

int32_t CryptoTest::ExecSdcBinary(std::vector<std::string> params, int isCrypt)
{
    pid_t pid = fork();
    if (pid < 0) {
        return -EINVAL;
    }
    if (pid == 0) {
        int ret = -EINVAL;
        if (!isCrypt) {
            char *const argv[] = {const_cast<char *>("/system/bin/sdc"), const_cast<char *>("nullcmd"), nullptr};
            ret = execv(argv[0], argv);
        } else if (params.size() == PARAMS_SIZE_0) {
            char *const argv[] = {const_cast<char *>("/system/bin/sdc"), nullptr};
            ret = execv(argv[0], argv);
        } else if (params.size() == PARAMS_SIZE_1) {
            char *const argv[] = {const_cast<char *>("/system/bin/sdc"), const_cast<char *>("filecrypt"),
                const_cast<char *>(params[0].c_str()), nullptr};
            ret = execv(argv[0], argv);
        } else if (params.size() == PARAMS_SIZE_2) {
            char *const argv[] = {const_cast<char *>("/system/bin/sdc"), const_cast<char *>("filecrypt"),
                const_cast<char *>(params[0].c_str()), const_cast<char *>(params[1].c_str()), nullptr};
            ret = execv(argv[0], argv);
        } else if (params.size() == PARAMS_SIZE_3) {
            char *const argv[] = {const_cast<char *>("/system/bin/sdc"), const_cast<char *>("filecrypt"),
                const_cast<char *>(params[0].c_str()), const_cast<char *>(params[1].c_str()),
                const_cast<char *>(params[2].c_str()), nullptr};
            ret = execv(argv[0], argv);
        } else if (params.size() == PARAMS_SIZE_4) {
            char *const argv[] = {const_cast<char *>("/system/bin/sdc"), const_cast<char *>("filecrypt"),
                const_cast<char *>(params[0].c_str()), const_cast<char *>(params[1].c_str()),
                const_cast<char *>(params[2].c_str()), const_cast<char *>(params[3].c_str()), nullptr};
            ret = execv(argv[0], argv);
        }
        if (ret) {
            return -EINVAL;
        }
    }
    int status;
    pid_t ret = waitpid(pid, &status, 0);
    if (ret != pid) {
        return -EINVAL;
    }

    return 0;
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

    bool ret = HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_FALSE(ret);

    ret = HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterEncryptKey_0100 end";
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

    bool ret = HuksMaster::GetInstance().DecryptKey(ctx, auth, key, isNeedNewNonce);
    EXPECT_FALSE(ret);

    ret = HuksMaster::GetInstance().DecryptKeyEx(ctx, auth, rnd);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterDecryptKey_0100 end";
}

/**
 * @tc.name: Huks_Master_Hdi_Access_Finish_001
 * @tc.desc: Verify the HuksMaster HdiAccessFinish function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Hdi_Access_Finish_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterHdiAccessFinish_0100 start";
    const HksBlob handle {};
    struct HksParamSet *paramSet = nullptr;;
    const HksBlob inData {};
    HksBlob outData;

    int ret = HuksMaster::GetInstance().HdiAccessFinish(handle, paramSet, inData, outData);
    EXPECT_EQ(ret, HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterHdiAccessFinish_0100 end";
}

/**
 * @tc.name: Huks_Master_Hdi_Access_Upgrade_Key_001
 * @tc.desc: Verify the HuksMaster HdiAccessUpgradeKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Huks_Master_Hdi_Access_Upgrade_Key_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterHdiAccessUpgradeKey_0100 start";
    const HksBlob oldKey {};
    struct HksParamSet *paramSet = nullptr;
    HksBlob newKey;

    int ret = HuksMaster::GetInstance().HdiAccessUpgradeKey(oldKey, paramSet, newKey);
    EXPECT_NE(ret, HKS_SUCCESS);
    GTEST_LOG_(INFO) << "CryptoTest_HuksMasterHdiAccessUpgradeKey_0100 end";
}

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

    bool ret = OpensslCrypto::AESEncrypt(preKey, plainText, keyContext);
    EXPECT_TRUE(ret);

    ret = OpensslCrypto::AESDecrypt(preKey, keyContext, plainText);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "CryptoTest_OpensslCryptoAESEncrypt_0100 end";
}

/**
 * @tc.name: Fscrypt_SDC_Filecrypt_001
 * @tc.desc: Verify the SDC UnlockUserScreen and LockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Fscrypt_SDC_Filecrypt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_Fscrypt_SDC_Filecrypt_0100 start";
    std::vector<std::string> params;

    // test sdc unlock_user_screen
    params.clear();
    params.push_back("unlock_user_screen");
    params.push_back("id");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("unlock_user_screen");
    params.push_back("10");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("unlock_user_screen");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("unlock_user_screen");
    params.push_back("10");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();

    // test sdc lock_user_screen
    params.clear();
    params.push_back("lock_user_screen");
    params.push_back("id");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("lock_user_screen");
    params.push_back("10");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("lock_user_screen");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("lock_user_screen");
    params.push_back("10");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    GTEST_LOG_(INFO) << "CryptoTest_Fscrypt_SDC_Filecrypt_0100 end";
}

/**
 * @tc.name: Fscrypt_SDC_Filecrypt_002
 * @tc.desc: Verify the SDC GenerateAppKey and DeleteAppKey function.
 * @tc.type: FUNC
 * @tc.require: SR000H0CM9
 */
HWTEST_F(CryptoTest, Fscrypt_SDC_Filecrypt_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CryptoTest_Fscrypt_SDC_Filecrypt_0200 start";
    std::vector<std::string> params;

    // test sdc generate_app_key
    params.clear();
    params.push_back("generate_app_key");
    params.push_back("id");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("generate_app_key");
    params.push_back("10");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("generate_app_key");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("generate_app_key");
    params.push_back("10");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();

    // test sdc delete_app_key
    params.clear();
    params.push_back("delete_app_key");
    params.push_back("id");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("delete_app_key");
    params.push_back("10");
    params.push_back("01234567890abcd");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("delete_app_key");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    params.push_back("delete_app_key");
    params.push_back("10");
    params.push_back("01234567890abcd");
    EXPECT_EQ(0, CryptoTest::ExecSdcBinary(params, 1));
    params.clear();
    GTEST_LOG_(INFO) << "CryptoTest_Fscrypt_SDC_Filecrypt_0200 end";
}
}
}