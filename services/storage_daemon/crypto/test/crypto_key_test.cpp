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
    static const std::string testMnt;
    static const std::string testDirLegacy;
    static const std::string testDirV2;
    static const std::string testKeyPath;
    static const std::string testPolicy;
    OHOS::StorageDaemon::UserAuth emptyUserAuth { "" };
};

const std::string CryptoKeyTest::testMnt = "/data";
const std::string CryptoKeyTest::testDirLegacy = "/data/test/crypto_dir_legacy";
const std::string CryptoKeyTest::testDirV2 = "/data/test/crypto_dir";
const std::string CryptoKeyTest::testKeyPath = "/data/test/keypath";
const std::string CryptoKeyTest::testPolicy = "/data/test/policy";
OHOS::StorageDaemon::BaseKey testKey {CryptoKeyTest::testKeyPath};

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
    EXPECT_TRUE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V1));

    EXPECT_EQ(OHOS::StorageDaemon::FSCRYPT_V1, testKey.keyInfo_.version);
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_AES_256_XTS_KEY_SIZE, testKey.keyInfo_.key.size);
    EXPECT_NE(nullptr, testKey.keyInfo_.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_DESC_SIZE, testKey.keyInfo_.keyDesc.size);
    EXPECT_NE(nullptr, testKey.keyInfo_.keyDesc.data.get());
    testKey.keyInfo_.key.Clear();
    testKey.keyInfo_.keyDesc.Clear();

    // On kernel not support the v2, InitKey with v2 should fail.
    if (OHOS::StorageDaemon::KeyCtrl::GetFscryptVersion() == OHOS::StorageDaemon::FSCRYPT_V1) {
        EXPECT_FALSE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V2));
    } else {
        EXPECT_TRUE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V2));
        EXPECT_EQ(OHOS::StorageDaemon::FSCRYPT_V2, testKey.keyInfo_.version);
        testKey.keyInfo_.key.Clear();
        testKey.keyInfo_.keyDesc.Clear();
    }
}

/**
 * @tc.name: basekey_store
 * @tc.desc: Verify the StoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_store, TestSize.Level1)
{
    std::string buf {};
    OHOS::ForceRemoveDirectory(testKeyPath);

    EXPECT_TRUE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V1));
    EXPECT_TRUE(testKey.StoreKey(emptyUserAuth));

    EXPECT_TRUE(OHOS::FileExists(testKeyPath + "/alias"));
    OHOS::LoadStringFromFile(testKeyPath + "/alias", buf);
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, buf.size());

    EXPECT_TRUE(OHOS::FileExists(testKeyPath + "/sec_discard"));
    OHOS::LoadStringFromFile(testKeyPath + "/sec_discard", buf);
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_SECDISC_SIZE, buf.size());

    EXPECT_TRUE(OHOS::FileExists(testKeyPath + "/encrypted"));
    OHOS::LoadStringFromFile(testKeyPath + "/encrypted", buf);
    EXPECT_EQ(80U, buf.size()); // the plaintext of 64 bytes, encrypted to 80 bytes size.

    OHOS::LoadStringFromFile(testKeyPath + "/version", buf);
    EXPECT_EQ(1U, buf.length());
    EXPECT_EQ('1', buf[0]);
}

/**
 * @tc.name: basekey_restore
 * @tc.desc: Verify the RestoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_restore, TestSize.Level1)
{
    EXPECT_TRUE(testKey.RestoreKey(emptyUserAuth));

    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_AES_256_XTS_KEY_SIZE, testKey.keyInfo_.key.size);
    EXPECT_NE(nullptr, testKey.keyInfo_.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_DESC_SIZE, testKey.keyInfo_.keyDesc.size);
    EXPECT_NE(nullptr, testKey.keyInfo_.keyDesc.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::FSCRYPT_V1, testKey.keyInfo_.version);
}

/**
 * @tc.name: basekey_install_v1
 * @tc.desc: Verify the ActiveKey function of v1 key.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_install_v1, TestSize.Level1)
{
    EXPECT_TRUE(testKey.RestoreKey(emptyUserAuth));
    EXPECT_FALSE(testKey.keyInfo_.key.IsEmpty());

    EXPECT_TRUE(testKey.ActiveKey());
    // raw key should be erase after install to kernel.
    EXPECT_TRUE(testKey.keyInfo_.key.IsEmpty());

    EXPECT_EQ(OHOS::StorageDaemon::FSCRYPT_V1, testKey.keyInfo_.version);
    // v1 key installed, and keyDesc was saved on disk.
    EXPECT_TRUE(OHOS::FileExists(testKeyPath + "/key_desc"));
}

/**
 * @tc.name: basekey_clear
 * @tc.desc: Verify the ClearKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_clear, TestSize.Level1)
{
    EXPECT_TRUE(testKey.RestoreKey(emptyUserAuth));
    EXPECT_FALSE(testKey.keyInfo_.key.IsEmpty());

    EXPECT_TRUE(testKey.ClearKey());
    EXPECT_TRUE(testKey.keyInfo_.key.IsEmpty());
    EXPECT_TRUE(testKey.keyInfo_.keyDesc.IsEmpty());
}

/**
 * @tc.name: basekey_fscrypt_v1_policy_set
 * @tc.desc: Verify the fscrypt V1 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v1_policy_set, TestSize.Level1)
{
    EXPECT_TRUE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V1));
    EXPECT_TRUE(testKey.StoreKey(emptyUserAuth));
    EXPECT_TRUE(testKey.ActiveKey());

    OHOS::StorageDaemon::FscryptPolicy arg;
    arg.v1.version = FSCRYPT_POLICY_V1;
    memcpy_s(arg.v1.master_key_descriptor, FSCRYPT_KEY_DESCRIPTOR_SIZE, testKey.keyInfo_.keyDesc.data.get(),
        testKey.keyInfo_.keyDesc.size);
    arg.v1.contents_encryption_mode = OHOS::StorageDaemon::CONTENTS_MODES.at("aes-256-xts");
    arg.v1.filenames_encryption_mode = OHOS::StorageDaemon::FILENAME_MODES.at("aes-256-cts");
    arg.v1.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    OHOS::ForceRemoveDirectory(testDirLegacy);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(testDirLegacy));
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::SetPolicy(testDirLegacy, arg));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(testDirLegacy + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirLegacy + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirLegacy + "/test_file2", "AA"));
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
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::GetPolicy(testDirLegacy, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V1, arg.policy.version);

    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::GetPolicy(testDirLegacy + "/test_dir", arg));
    EXPECT_EQ(FSCRYPT_POLICY_V1, arg.policy.version);
}

/**
 * @tc.name: basekey_fscrypt_v1_policy_clear
 * @tc.desc: Verify the ClearKey key function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v1_policy_clear, TestSize.Level1)
{
    EXPECT_TRUE(testKey.ClearKey());

    // the encrypted dir was readonly now, and can be seen encrypted after reboot.
    EXPECT_FALSE(OHOS::ForceCreateDirectory(testDirLegacy + "/test_dir1"));
}

/**
 * @tc.name: basekey_fscrypt_v2_key
 * @tc.desc: Verify the fscrypt V2 active and clear function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_key, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (OHOS::StorageDaemon::KeyCtrl::GetFscryptVersion() == OHOS::StorageDaemon::FSCRYPT_V1) {
        return;
    }

    EXPECT_TRUE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V2));
    EXPECT_TRUE(testKey.StoreKey(emptyUserAuth));

    std::string buf {};
    OHOS::LoadStringFromFile(testKeyPath + "/version", buf);
    EXPECT_EQ(1U, buf.length());
    EXPECT_EQ('2', buf[0]);

    EXPECT_TRUE(testKey.ActiveKey(testMnt));
    EXPECT_EQ(static_cast<unsigned int>(FSCRYPT_KEY_IDENTIFIER_SIZE), testKey.keyInfo_.keyId.size);
    EXPECT_TRUE(OHOS::FileExists(testKeyPath + "/key_id"));

    EXPECT_TRUE(testKey.ClearKey(testMnt));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_set
 * @tc.desc: Verify the fscrypt V2 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_set, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (OHOS::StorageDaemon::KeyCtrl::GetFscryptVersion() == OHOS::StorageDaemon::FSCRYPT_V1) {
        return;
    }

    EXPECT_TRUE(testKey.RestoreKey(emptyUserAuth));
    EXPECT_EQ(OHOS::StorageDaemon::FSCRYPT_V2, testKey.keyInfo_.version);
    EXPECT_TRUE(testKey.ActiveKey(testMnt));

    // raw key should be erase after install to kernel.
    EXPECT_TRUE(testKey.keyInfo_.key.IsEmpty());
    OHOS::StorageDaemon::FscryptPolicy arg;
    arg.v2.version = FSCRYPT_POLICY_V2;
    memcpy_s(arg.v2.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, testKey.keyInfo_.keyId.data.get(),
        testKey.keyInfo_.keyId.size);
    arg.v2.contents_encryption_mode = OHOS::StorageDaemon::CONTENTS_MODES.at("aes-256-xts");
    arg.v2.filenames_encryption_mode = OHOS::StorageDaemon::FILENAME_MODES.at("aes-256-cts");
    arg.v2.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    OHOS::ForceRemoveDirectory(testDirV2);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(testDirV2));
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::SetPolicy(testDirV2, arg));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(testDirV2 + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirV2 + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirV2 + "/test_file2", "AA"));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_get
 * @tc.desc: Verify the fscrypt V2 getpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_get, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (OHOS::StorageDaemon::KeyCtrl::GetFscryptVersion() == OHOS::StorageDaemon::FSCRYPT_V1) {
        return;
    }

    struct fscrypt_get_policy_ex_arg arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::GetPolicy(testDirV2, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V2, arg.policy.version);

    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::GetPolicy(testDirV2 + "/test_dir", arg));
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
    // skipped when kernel not support v2
    if (OHOS::StorageDaemon::KeyCtrl::GetFscryptVersion() == OHOS::StorageDaemon::FSCRYPT_V1) {
        return;
    }

    EXPECT_TRUE(testKey.ClearKey(testMnt));
    // When the v2 policy removed, the files are encrypted.
    EXPECT_FALSE(OHOS::FileExists(testDirV2 + "/test_dir"));
    EXPECT_FALSE(OHOS::FileExists(testDirV2 + "/test_file1"));
    EXPECT_FALSE(OHOS::FileExists(testDirV2 + "/test_file2"));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_restore
 * @tc.desc: Verify the fscrypt V2 restore and decrypt.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_restore, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (OHOS::StorageDaemon::KeyCtrl::GetFscryptVersion() == OHOS::StorageDaemon::FSCRYPT_V1) {
        return;
    }

    EXPECT_TRUE(testKey.RestoreKey(emptyUserAuth));
    EXPECT_EQ(OHOS::StorageDaemon::FSCRYPT_V2, testKey.keyInfo_.version);
    EXPECT_TRUE(testKey.ActiveKey(testMnt));

    OHOS::StorageDaemon::FscryptPolicy arg;
    arg.v2.version = FSCRYPT_POLICY_V2;
    memcpy_s(arg.v2.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, testKey.keyInfo_.keyId.data.get(),
        testKey.keyInfo_.keyId.size);
    arg.v2.contents_encryption_mode = OHOS::StorageDaemon::CONTENTS_MODES.at("aes-256-xts");
    arg.v2.filenames_encryption_mode = OHOS::StorageDaemon::FILENAME_MODES.at("aes-256-cts");
    arg.v2.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::SetPolicy(testDirV2, arg));

    EXPECT_TRUE(OHOS::FileExists(testDirV2 + "/test_dir"));
    EXPECT_TRUE(OHOS::FileExists(testDirV2 + "/test_file1"));
    EXPECT_TRUE(OHOS::FileExists(testDirV2 + "/test_file2"));

    EXPECT_TRUE(testKey.ClearKey(testMnt));
}

/**
 * @tc.name: basekey_fscrypt_v2_load_and_set_policy_default
 * @tc.desc: Verify the KeyCtrl::LoadAndSetPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_load_and_set_policy_default, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (OHOS::StorageDaemon::KeyCtrl::GetFscryptVersion() == OHOS::StorageDaemon::FSCRYPT_V1) {
        return;
    }

    EXPECT_TRUE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V2));
    EXPECT_TRUE(testKey.StoreKey(emptyUserAuth));
    EXPECT_TRUE(testKey.ActiveKey(testMnt));

    OHOS::ForceRemoveDirectory(testDirV2);
    OHOS::ForceCreateDirectory(testDirV2);
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::LoadAndSetPolicy(testKey.GetDir(), "", testDirV2));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(testDirV2 + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirV2 + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirV2 + "/test_file2", "AA"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirV2 + "/1111111111111111111111111111111111111111111111111", "AA"));

    EXPECT_TRUE(testKey.ClearKey(testMnt));
}

/**
 * @tc.name: basekey_fscrypt_v1_load_and_set_policy_default
 * @tc.desc: Verify the fscrypt V1 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v1_load_and_set_policy_default, TestSize.Level1)
{
    EXPECT_TRUE(testKey.InitKey(OHOS::StorageDaemon::FSCRYPT_V1));
    // the ext4 disk with `mke2fs -O encrypt` mounted for test
    EXPECT_TRUE(testKey.StoreKey(emptyUserAuth));
    EXPECT_TRUE(testKey.ActiveKey());

    OHOS::ForceRemoveDirectory(testDirLegacy);
    OHOS::ForceCreateDirectory(testDirLegacy);
    EXPECT_TRUE(OHOS::StorageDaemon::KeyCtrl::LoadAndSetPolicy(testKey.GetDir(), "", testDirLegacy));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(testDirLegacy + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirLegacy + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirLegacy + "/test_file2", "AA"));
    EXPECT_TRUE(OHOS::SaveStringToFile(testDirLegacy + "/1111111111111111111111111111111111111111111111111", "AA"));

    EXPECT_TRUE(testKey.ClearKey(testMnt));
}
