/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <vector>
#include <string>
#include <gtest/gtest.h>

#include "base_key.h"
#include "key_ctrl.h"
#include "file_ex.h"
#include "directory_ex.h"
#include "securec.h"

using namespace testing::ext;

class CryptoKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    OHOS::StorageDaemon::UserAuth emptyUserAuth { "" };
};

std::string toEncryptMnt("/data");
std::string toEncryptDirLegacy("/data/test/crypto_dir_legacy");
std::string toEncryptDir("/data/test/crypto_dir");
std::string testKeyPath("/data/test/sys_de");
std::string policyPath("/data/test/policy");
OHOS::StorageDaemon::BaseKey deKey {testKeyPath};
void CryptoKeyTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void CryptoKeyTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void CryptoKeyTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void CryptoKeyTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: basekey_init
 * @tc.desc: Verify the InitKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_init, TestSize.Level1)
{
    OHOS::StorageDaemon::BaseKey deKey(testKeyPath);

    EXPECT_EQ(true, deKey.InitKey());

    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_AES_256_XTS_KEY_SIZE, deKey.keyInfo_.key.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, deKey.keyInfo_.keyDesc.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.keyDesc.data.get());

    deKey.keyInfo_.key.Clear();
    deKey.keyInfo_.keyDesc.Clear();
}

/**
 * @tc.name: basekey_store
 * @tc.desc: Verify the StoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_store, TestSize.Level1)
{
    std::vector<char> buf {};
    OHOS::ForceRemoveDirectory(testKeyPath);

    EXPECT_EQ(true, deKey.InitKey());
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));

    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/alias"));
    OHOS::LoadBufferFromFile(testKeyPath + "/alias", buf);
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, buf.size());

    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/sec_discard"));
    OHOS::LoadBufferFromFile(testKeyPath + "/sec_discard", buf);
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_SECDISC_SIZE, buf.size());

    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/encrypted"));
    OHOS::LoadBufferFromFile(testKeyPath + "/encrypted", buf);
    EXPECT_EQ(80U, buf.size());
}

/**
 * @tc.name: basekey_restore
 * @tc.desc: Verify the RestoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_restore, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.RestoreKey(emptyUserAuth));

    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_AES_256_XTS_KEY_SIZE, deKey.keyInfo_.key.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, deKey.keyInfo_.keyDesc.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.keyDesc.data.get());
}

/**
 * @tc.name: basekey_install
 * @tc.desc: Verify the ActiveKeyLegacy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_install, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.RestoreKey(emptyUserAuth));
    EXPECT_EQ(false, deKey.keyInfo_.key.IsEmpty());

    EXPECT_EQ(true, deKey.ActiveKeyLegacy());
    // raw key should be erase after install to kernel.
    EXPECT_EQ(true, deKey.keyInfo_.key.IsEmpty());
}

/**
 * @tc.name: basekey_clear
 * @tc.desc: Verify the ClearKeyLegacy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_clear, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.RestoreKey(emptyUserAuth));
    EXPECT_EQ(false, deKey.keyInfo_.key.IsEmpty());

    EXPECT_EQ(true, deKey.ClearKeyLegacy());
    EXPECT_EQ(true, deKey.keyInfo_.key.IsEmpty());
    EXPECT_EQ(true, deKey.keyInfo_.keyDesc.IsEmpty());
}

/**
 * @tc.name: basekey_fscrypt_v1_policy_set
 * @tc.desc: Verify the fscrypt V1 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v1_policy_set, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.InitKey());
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));
    EXPECT_EQ(true, deKey.ActiveKeyLegacy());

    struct fscrypt_policy_v1 arg = {.version = FSCRYPT_POLICY_V1};
    memcpy_s(arg.master_key_descriptor, FSCRYPT_KEY_DESCRIPTOR_SIZE, deKey.keyInfo_.keyDesc.data.get(), deKey.keyInfo_.keyDesc.size);
    arg.contents_encryption_mode = OHOS::StorageDaemon::CONTENTS_MODES.at("aes-256-xts");
    arg.filenames_encryption_mode = OHOS::StorageDaemon::FILENAME_MODES.at("aes-256-cts");
    arg.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    OHOS::ForceRemoveDirectory(toEncryptDirLegacy);
    EXPECT_EQ(true, OHOS::ForceCreateDirectory(toEncryptDirLegacy));
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::SetPolicy(toEncryptDirLegacy, arg));

    EXPECT_EQ(true, OHOS::ForceCreateDirectory(toEncryptDirLegacy + "/test_dir"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDirLegacy + "/test_file1", "hello, world!\n"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDirLegacy + "/test_file2", "AA"));
}

/**
 * @tc.name: basekey_fscrypt_v1_policy_get
 * @tc.desc: Verify the fscrypt V1 getpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v1_policy_get, TestSize.Level1)
{
    struct fscrypt_get_policy_ex_arg arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::GetPolicy(toEncryptDirLegacy, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V1, arg.policy.version);

    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::GetPolicy(toEncryptDirLegacy + "/test_dir", arg));
    EXPECT_EQ(FSCRYPT_POLICY_V1, arg.policy.version);
}

/**
 * @tc.name: basekey_fscrypt_v1_policy_clear
 * @tc.desc: Verify the ClearKeyLegacy key function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v1_policy_clear, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.ClearKeyLegacy());

    // the encrypted dir was readonly now, and can be seen encrypted after reboot.
    EXPECT_EQ(false, OHOS::ForceCreateDirectory(toEncryptDirLegacy + "/test_dir1"));
}

/**
 * @tc.name: basekey_fscrypt_v2_key
 * @tc.desc: Verify the fscrypt V2 active and clear function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_key, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.InitKey());
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));
    EXPECT_EQ(true, deKey.ActiveKey(toEncryptMnt));

    EXPECT_EQ(static_cast<unsigned int>(FSCRYPT_KEY_IDENTIFIER_SIZE), deKey.keyInfo_.keyId.size);
    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/kid"));

    EXPECT_EQ(true, deKey.ClearKey(toEncryptMnt));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_set
 * @tc.desc: Verify the fscrypt V2 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_set, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.InitKey());
    // the ext4 disk with `mke2fs -O encrypt` mounted for test
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));
    EXPECT_EQ(true, deKey.ActiveKey(toEncryptMnt));

    // raw key should be erase after install to kernel.
    EXPECT_EQ(true, deKey.keyInfo_.key.IsEmpty());

    struct fscrypt_policy_v2 arg = {.version = FSCRYPT_POLICY_V2};
    memcpy_s(arg.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, deKey.keyInfo_.keyId.data.get(), deKey.keyInfo_.keyId.size);
    arg.contents_encryption_mode = OHOS::StorageDaemon::CONTENTS_MODES.at("aes-256-xts");
    arg.filenames_encryption_mode = OHOS::StorageDaemon::FILENAME_MODES.at("aes-256-cts");
    arg.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    OHOS::ForceRemoveDirectory(toEncryptDir);
    EXPECT_EQ(true, OHOS::ForceCreateDirectory(toEncryptDir));
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::SetPolicy(toEncryptDir, arg));

    EXPECT_EQ(true, OHOS::ForceCreateDirectory(toEncryptDir + "/test_dir"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/test_file1", "hello, world!\n"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/test_file2", "AA"));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_get
 * @tc.desc: Verify the fscrypt V2 getpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_get, TestSize.Level1)
{
    struct fscrypt_get_policy_ex_arg arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::GetPolicy(toEncryptDir, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V2, arg.policy.version);

    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::GetPolicy(toEncryptDir + "/test_dir", arg));
    EXPECT_EQ(FSCRYPT_POLICY_V2, arg.policy.version);
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_clear
 * @tc.desc: Verify the fscrypt V2 clear key function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_clear, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.ClearKey(toEncryptMnt));
    // When the v2 policy removed, the files are encrypted.
    EXPECT_EQ(false, OHOS::FileExists(toEncryptDir + "/test_dir"));
    EXPECT_EQ(false, OHOS::FileExists(toEncryptDir + "/test_file1"));
    EXPECT_EQ(false, OHOS::FileExists(toEncryptDir + "/test_file2"));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_restore
 * @tc.desc: Verify the fscrypt V2 restore and decrypt.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_restore, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.RestoreKey(emptyUserAuth));
    // the ext4 disk with `mke2fs -O encrypt` mounted for test
    EXPECT_EQ(true, deKey.ActiveKey(toEncryptMnt));

    struct fscrypt_policy_v2 arg = {.version = FSCRYPT_POLICY_V2};
    memcpy_s(arg.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, deKey.keyInfo_.keyId.data.get(), deKey.keyInfo_.keyId.size);
    arg.contents_encryption_mode = OHOS::StorageDaemon::CONTENTS_MODES.at("aes-256-xts");
    arg.filenames_encryption_mode = OHOS::StorageDaemon::FILENAME_MODES.at("aes-256-cts");
    arg.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::SetPolicy(toEncryptDir, arg));

    EXPECT_EQ(true, OHOS::FileExists(toEncryptDir + "/test_dir"));
    EXPECT_EQ(true, OHOS::FileExists(toEncryptDir + "/test_file1"));
    EXPECT_EQ(true, OHOS::FileExists(toEncryptDir + "/test_file2"));

    EXPECT_EQ(true, deKey.ClearKey(toEncryptMnt));
}

/**
 * @tc.name: basekey_fscrypt_v2_load_and_set_policy_default
 * @tc.desc: Verify the KeyCtrl::LoadAndSetPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_load_and_set_policy_default, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.InitKey());
    // the ext4 disk with `mke2fs -O encrypt` mounted for test
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));
    EXPECT_EQ(true, deKey.ActiveKey(toEncryptMnt));

    OHOS::ForceRemoveDirectory(toEncryptDir);
    OHOS::ForceCreateDirectory(toEncryptDir);
    OHOS::ForceRemoveDirectory(policyPath);
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::LoadAndSetPolicy(testKeyPath + "/kid", policyPath, toEncryptDir));

    EXPECT_EQ(true, OHOS::ForceCreateDirectory(toEncryptDir + "/test_dir"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/test_file1", "hello, world!\n"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/test_file2", "AA"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/1111111111111111111111111111111111111111111111111", "AA"));

    EXPECT_EQ(true, deKey.ClearKey(toEncryptMnt));
}

/**
 * @tc.name: basekey_fscrypt_v2_load_and_set_policy_from_file
 * @tc.desc: Verify the fscrypt V2 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_load_and_set_policy_from_file, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.InitKey());
    // the ext4 disk with `mke2fs -O encrypt` mounted for test
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));
    EXPECT_EQ(true, deKey.ActiveKey(toEncryptMnt));

    OHOS::ForceRemoveDirectory(toEncryptDir);
    OHOS::ForceCreateDirectory(toEncryptDir);
    OHOS::ForceCreateDirectory(policyPath);
    // the `aes-128-cts/aes-128-cbc` need the CONFIG_CRYPTO_ESSIV, not enabled yet
    EXPECT_EQ(true, OHOS::SaveStringToFile(policyPath + "/filename", "aes-256-cts"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(policyPath + "/content", "aes-256-xts"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(policyPath + "/flags", "padding-8"));
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::LoadAndSetPolicy(testKeyPath + "/kid", policyPath, toEncryptDir));

    EXPECT_EQ(true, OHOS::ForceCreateDirectory(toEncryptDir + "/test_dir"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/test_file1", "hello, world!\n"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/test_file2", "AA"));
    EXPECT_EQ(true, OHOS::SaveStringToFile(toEncryptDir + "/1111111111111111111111111111111111111111111111111", "AA"));

    EXPECT_EQ(true, deKey.ClearKey(toEncryptMnt));
}
