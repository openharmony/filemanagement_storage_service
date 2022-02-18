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

#include "file_ex.h"
#include "directory_ex.h"
#include "securec.h"
#include "fscrypt_key_v1.h"
#include "fscrypt_key_v2.h"
#include "key_ctrl.h"

using namespace testing::ext;
using namespace OHOS::StorageDaemon;

namespace {
const std::string TEST_MNT = "/data";
const std::string TEST_DIR_LEGACY = "/data/test/crypto_dir_legacy";
const std::string TEST_DIR_V2 = "/data/test/crypto_dir";
const std::string TEST_KEYPATH = "/data/test/keypath";
const std::string TEST_KEYDIR_VERSION0 = "/version_0";
const std::string TEST_KEYDIR_VERSION1 = "/version_1";
const std::string TEST_KEYDIR_VERSION2 = "/version_2";
const std::string TEST_KEYDIR_LATEST = "/latest";
const std::string TEST_KEYDIR_LATEST_BACKUP = "/latest_bak";
const std::string TEST_POLICY = "/data/test/policy";
FscryptKeyV1 g_testKeyV1 {TEST_KEYPATH};
FscryptKeyV2 g_testKeyV2 {TEST_KEYPATH};
}

class CryptoKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    UserAuth emptyUserAuth { "" };
};

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
 * @tc.name: fscrypt_key_v1_init
 * @tc.desc: Verify the InitKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_init, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV1.InitKey());

    EXPECT_EQ(FSCRYPT_V1, g_testKeyV1.keyInfo_.version);
    EXPECT_EQ(CRYPTO_AES_256_XTS_KEY_SIZE, g_testKeyV1.keyInfo_.key.size);
    EXPECT_NE(nullptr, g_testKeyV1.keyInfo_.key.data.get());
    g_testKeyV1.keyInfo_.key.Clear();
}

/**
 * @tc.name: fscrypt_key_v2_init
 * @tc.desc: Verify the InitKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_init, TestSize.Level1)
{
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        // On kernel donnot support fscrypt_v2, the InitKey() should fail.
        EXPECT_FALSE(g_testKeyV2.InitKey());
    } else {
        EXPECT_TRUE(g_testKeyV2.InitKey());
        EXPECT_EQ(FSCRYPT_V2, g_testKeyV2.keyInfo_.version);
        EXPECT_EQ(CRYPTO_AES_256_XTS_KEY_SIZE, g_testKeyV2.keyInfo_.key.size);
        EXPECT_NE(nullptr, g_testKeyV2.keyInfo_.key.data.get());
        g_testKeyV2.keyInfo_.key.Clear();
    }
}

/**
 * @tc.name: fscrypt_key_v1_store
 * @tc.desc: Verify the StoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_store, TestSize.Level1)
{
    std::string buf {};
    g_testKeyV1.ClearKey();

    EXPECT_TRUE(g_testKeyV1.InitKey());
    EXPECT_TRUE(g_testKeyV1.StoreKey(emptyUserAuth));

    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS));
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS, buf));
    EXPECT_EQ(CRYPTO_KEY_ALIAS_SIZE, buf.size());

    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_SECDISC));
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_SECDISC, buf));
    EXPECT_EQ(CRYPTO_KEY_SECDISC_SIZE, buf.size());

    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ENCRYPTED));
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ENCRYPTED, buf));
    // the plaintext of 64 bytes, encrypted to 80 bytes size by huks.
    EXPECT_EQ(80U, buf.size());

    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + PATH_FSCRYPT_VER));
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + PATH_FSCRYPT_VER, buf));
    EXPECT_EQ(1U, buf.length());
    EXPECT_EQ('1', buf[0]);
}

/**
 * @tc.name: fscrypt_key_v2_store
 * @tc.desc: Verify the StoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_store, TestSize.Level1)
{
    std::string buf {};
    g_testKeyV1.ClearKey();

    EXPECT_TRUE(g_testKeyV2.InitKey());
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));

    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_SECDISC));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ENCRYPTED));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_ALIAS));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_SECDISC));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_ENCRYPTED));

    OHOS::LoadStringFromFile(TEST_KEYPATH + PATH_FSCRYPT_VER, buf);
    EXPECT_EQ(1U, buf.length());
    EXPECT_EQ('2', buf[0]);
}

/**
 * @tc.name: fscrypt_key_v2_update
 * @tc.desc: Verify the UpdateKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_update, TestSize.Level1)
{
    std::string buf {};
    EXPECT_TRUE(g_testKeyV2.UpdateKey());

    EXPECT_FALSE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS));
    EXPECT_FALSE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_ALIAS));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_ALIAS));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_SECDISC));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_ENCRYPTED));
    OHOS::LoadStringFromFile(TEST_KEYPATH + PATH_FSCRYPT_VER, buf);
    EXPECT_EQ(1U, buf.length());
    EXPECT_EQ('2', buf[0]);
}

/**
 * @tc.name: fscrypt_key_v1_restore_fail_wrong_version
 * @tc.desc: Verify the RestoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_restore_fail_wrong_version, TestSize.Level1)
{
    g_testKeyV1.keyInfo_.key.Clear();
    // the version loaded is v2, not expected v1.
    EXPECT_FALSE(g_testKeyV1.RestoreKey(emptyUserAuth));
}

/**
 * @tc.name: fscrypt_key_v1_restore
 * @tc.desc: Verify the RestoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_restore, TestSize.Level1)
{
    g_testKeyV1.ClearKey();
    EXPECT_TRUE(g_testKeyV1.InitKey());
    EXPECT_TRUE(g_testKeyV1.StoreKey(emptyUserAuth));
    EXPECT_TRUE(g_testKeyV1.UpdateKey());
    EXPECT_TRUE(g_testKeyV1.RestoreKey(emptyUserAuth));

    EXPECT_EQ(CRYPTO_AES_256_XTS_KEY_SIZE, g_testKeyV1.keyInfo_.key.size);
    EXPECT_NE(nullptr, g_testKeyV1.keyInfo_.key.data.get());
    EXPECT_EQ(FSCRYPT_V1, g_testKeyV1.keyInfo_.version);
}

/**
 * @tc.name: fscrypt_key_v1_active
 * @tc.desc: Verify the ActiveKey function of v1 key.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_active, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV1.RestoreKey(emptyUserAuth));
    EXPECT_FALSE(g_testKeyV1.keyInfo_.key.IsEmpty());
    EXPECT_EQ(FSCRYPT_V1, g_testKeyV1.keyInfo_.version);

    EXPECT_TRUE(g_testKeyV1.ActiveKey());
    // raw key should be erase after install to kernel.
    EXPECT_TRUE(g_testKeyV1.keyInfo_.key.IsEmpty());
    EXPECT_TRUE(g_testKeyV1.keyInfo_.keyId.IsEmpty());
    // key desc saved in memory for later clear key.
    EXPECT_FALSE(g_testKeyV1.keyInfo_.keyDesc.IsEmpty());

    // v1 key installed, and key_desc was saved on disk.
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + PATH_KEYDESC));
}

/**
 * @tc.name: fscrypt_key_v1_clear
 * @tc.desc: Verify the ClearKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_clear, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV1.ClearKey());
    EXPECT_TRUE(g_testKeyV1.keyInfo_.key.IsEmpty());
    EXPECT_FALSE(OHOS::FileExists(TEST_KEYPATH + PATH_KEYDESC));
    EXPECT_FALSE(OHOS::FileExists(TEST_KEYPATH + PATH_FSCRYPT_VER));
    EXPECT_FALSE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_ALIAS));
}


/**
 * @tc.name: fscrypt_key_v1_policy_set
 * @tc.desc: Verify the fscrypt V1 KeyCtrl::SetPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_policy_set, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV1.InitKey());
    EXPECT_TRUE(g_testKeyV1.StoreKey(emptyUserAuth));
    EXPECT_TRUE(g_testKeyV1.ActiveKey());

    FscryptPolicy arg;
    arg.v1.version = FSCRYPT_POLICY_V1;
    memcpy_s(arg.v1.master_key_descriptor, FSCRYPT_KEY_DESCRIPTOR_SIZE, g_testKeyV1.keyInfo_.keyDesc.data.get(),
        g_testKeyV1.keyInfo_.keyDesc.size);
    arg.v1.contents_encryption_mode = CONTENTS_MODES.at("aes-256-xts");
    arg.v1.filenames_encryption_mode = FILENAME_MODES.at("aes-256-cts");
    arg.v1.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    OHOS::ForceRemoveDirectory(TEST_DIR_LEGACY);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_LEGACY));
    EXPECT_TRUE(KeyCtrl::SetPolicy(TEST_DIR_LEGACY, arg));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_LEGACY + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_LEGACY + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_LEGACY + "/test_file2", "AA"));
}

/**
 * @tc.name: fscrypt_key_v1_policy_get
 * @tc.desc: Verify the fscrypt V1 KeyCtrl::GetPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_policy_get, TestSize.Level1)
{
    struct fscrypt_policy arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    EXPECT_TRUE(KeyCtrl::GetPolicy(TEST_DIR_LEGACY, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V1, arg.version);

    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    EXPECT_TRUE(KeyCtrl::GetPolicy(TEST_DIR_LEGACY + "/test_dir", arg));
    EXPECT_EQ(FSCRYPT_POLICY_V1, arg.version);
}

/**
 * @tc.name: fscrypt_key_v1_key_inactive
 * @tc.desc: Verify the FscryptKeyV1 InactiveKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_key_inactive, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV1.InactiveKey());

    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V2) {
        EXPECT_FALSE(OHOS::ForceCreateDirectory(TEST_DIR_LEGACY + "/test_dir1"));
        EXPECT_FALSE(OHOS::SaveStringToFile(TEST_DIR_LEGACY + "/test_file3", "AAA"));
    }
    // earlier kernels may have different behaviour.
}

/**
 * @tc.name: fscrypt_key_v2_active
 * @tc.desc: Verify the fscrypt V2 active function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_active, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        return;
    }

    g_testKeyV2.ClearKey();
    EXPECT_TRUE(g_testKeyV2.InitKey());
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(g_testKeyV2.ActiveKey());

    // raw key should be erase after install to kernel.
    EXPECT_TRUE(g_testKeyV2.keyInfo_.key.IsEmpty());
    EXPECT_TRUE(g_testKeyV2.keyInfo_.keyDesc.IsEmpty());
    EXPECT_EQ(static_cast<unsigned int>(FSCRYPT_KEY_IDENTIFIER_SIZE), g_testKeyV2.keyInfo_.keyId.size);
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + PATH_KEYID));
}

/**
 * @tc.name: fscrypt_key_v2_policy_set
 * @tc.desc: Verify the fscrypt V2 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_policy_set, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        return;
    }

    EXPECT_EQ(FSCRYPT_V2, g_testKeyV2.keyInfo_.version);
    FscryptPolicy arg;
    arg.v2.version = FSCRYPT_POLICY_V2;
    memcpy_s(arg.v2.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, g_testKeyV2.keyInfo_.keyId.data.get(),
        g_testKeyV2.keyInfo_.keyId.size);
    arg.v2.contents_encryption_mode = CONTENTS_MODES.at("aes-256-xts");
    arg.v2.filenames_encryption_mode = FILENAME_MODES.at("aes-256-cts");
    arg.v2.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    OHOS::ForceRemoveDirectory(TEST_DIR_V2);
    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_V2));
    EXPECT_TRUE(KeyCtrl::SetPolicy(TEST_DIR_V2, arg));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_V2 + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/test_file2", "AA"));
}

/**
 * @tc.name: fscrypt_key_v2_policy_get
 * @tc.desc: Verify the fscrypt V2 getpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_policy_get, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        return;
    }

    struct fscrypt_get_policy_ex_arg arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_TRUE(KeyCtrl::GetPolicy(TEST_DIR_V2, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V2, arg.policy.version);

    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_TRUE(KeyCtrl::GetPolicy(TEST_DIR_V2 + "/test_dir", arg));
    EXPECT_EQ(FSCRYPT_POLICY_V2, arg.policy.version);
}

/**
 * @tc.name: fscrypt_key_v2_policy_inactive
 * @tc.desc: Verify the fscryptV2 InactiveKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_policy_inactive, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        return;
    }

    EXPECT_TRUE(g_testKeyV2.InactiveKey());
    // When the v2 policy removed, the files are encrypted.
    EXPECT_FALSE(OHOS::FileExists(TEST_DIR_V2 + "/test_dir"));
    EXPECT_FALSE(OHOS::FileExists(TEST_DIR_V2 + "/test_file1"));
    EXPECT_FALSE(OHOS::FileExists(TEST_DIR_V2 + "/test_file2"));
}

/**
 * @tc.name: fscrypt_key_v2_policy_restore
 * @tc.desc: Verify the fscrypt V2 restore and decrypt.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_policy_restore, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        return;
    }

    EXPECT_TRUE(g_testKeyV2.RestoreKey(emptyUserAuth));
    EXPECT_EQ(FSCRYPT_V2, g_testKeyV2.keyInfo_.version);
    EXPECT_TRUE(g_testKeyV2.ActiveKey());

    // the files is decrypted now
    EXPECT_TRUE(OHOS::FileExists(TEST_DIR_V2 + "/test_dir"));
    EXPECT_TRUE(OHOS::FileExists(TEST_DIR_V2 + "/test_file1"));
    EXPECT_TRUE(OHOS::FileExists(TEST_DIR_V2 + "/test_file2"));

    EXPECT_TRUE(g_testKeyV2.ClearKey());
}

/**
 * @tc.name: fscrypt_key_v2_load_and_set_policy_default
 * @tc.desc: Verify the KeyCtrl::LoadAndSetPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_load_and_set_policy_default, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        return;
    }

    g_testKeyV2.ClearKey();
    EXPECT_TRUE(g_testKeyV2.InitKey());
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(g_testKeyV2.ActiveKey());

    OHOS::ForceRemoveDirectory(TEST_DIR_V2);
    OHOS::ForceCreateDirectory(TEST_DIR_V2);
    EXPECT_TRUE(KeyCtrl::LoadAndSetPolicy(g_testKeyV2.GetDir(), TEST_DIR_V2));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_V2 + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/test_file2", "AA"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/111111111111111111111111111111111111111111111111", "AA"));

    EXPECT_TRUE(g_testKeyV2.ClearKey());
}

/**
 * @tc.name: fscrypt_key_v1_load_and_set_policy_default
 * @tc.desc: Verify the fscrypt V1 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v1_load_and_set_policy_default, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV1.InitKey());
    EXPECT_TRUE(g_testKeyV1.StoreKey(emptyUserAuth));
    EXPECT_TRUE(g_testKeyV1.ActiveKey());

    OHOS::ForceRemoveDirectory(TEST_DIR_LEGACY);
    OHOS::ForceCreateDirectory(TEST_DIR_LEGACY);
    EXPECT_TRUE(KeyCtrl::LoadAndSetPolicy(g_testKeyV1.GetDir(), TEST_DIR_LEGACY));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_LEGACY + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_LEGACY + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_LEGACY + "/test_file2", "AA"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_LEGACY + "/111111111111111111111111111111111111111111111111", "AA"));

    EXPECT_TRUE(g_testKeyV1.ClearKey());
}

/**
 * @tc.name: fscrypt_key_storekey_version_test_1
 * @tc.desc: Verify the fscrypt storekey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_storekey_version_test_1, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV2.InitKey());

    // storekey to version 0
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS));
    std::string aliasV0;
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS, aliasV0));

    // storekey to version 1
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_ALIAS));
    std::string aliasV1;
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_ALIAS, aliasV1));
    EXPECT_NE(aliasV0, aliasV1);

    // storekey to version 2
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION2 + PATH_ALIAS));
    std::string aliasV2;
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION2 + PATH_ALIAS, aliasV2));
    EXPECT_NE(aliasV1, aliasV2);

    // updatekey will rename version 2 to latest
    EXPECT_TRUE(g_testKeyV2.UpdateKey());
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_ALIAS));
    EXPECT_FALSE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST_BACKUP + PATH_ALIAS));
    std::string aliasLatest;
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_ALIAS, aliasLatest));
    EXPECT_EQ(aliasLatest, aliasV2);
}

/**
 * @tc.name: fscrypt_key_storekey_version_test_2
 * @tc.desc: Verify the fscrypt storekey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_storekey_version_test_2, TestSize.Level1)
{
    EXPECT_TRUE(g_testKeyV2.RestoreKey(emptyUserAuth));

    // storekey to version 0
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS));
    std::string aliasV0;
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION0 + PATH_ALIAS, aliasV0));

    // storekey to version 1
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_ALIAS));
    std::string aliasV1;
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_VERSION1 + PATH_ALIAS, aliasV1));

    // restorekey will decrypt from versions and rename first success one to latest
    EXPECT_TRUE(g_testKeyV2.RestoreKey(emptyUserAuth));
    EXPECT_TRUE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_ALIAS));
    EXPECT_FALSE(OHOS::FileExists(TEST_KEYPATH + TEST_KEYDIR_LATEST_BACKUP + PATH_ALIAS));
    std::string aliasLatest;
    EXPECT_TRUE(OHOS::LoadStringFromFile(TEST_KEYPATH + TEST_KEYDIR_LATEST + PATH_ALIAS, aliasLatest));
    EXPECT_EQ(aliasLatest, aliasV1);
}

/**
 * @tc.name: fscrypt_key_v2_load_and_set_policy_padding_4
 * @tc.desc: Verify the KeyCtrl::LoadAndSetPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BO
 */
HWTEST_F(CryptoKeyTest, fscrypt_key_v2_load_and_set_policy_padding_4, TestSize.Level1)
{
    // skipped when kernel not support v2
    if (KeyCtrl::GetFscryptVersion(TEST_MNT) == FSCRYPT_V1) {
        return;
    }

    g_testKeyV2.ClearKey();
    EXPECT_TRUE(g_testKeyV2.InitKey());
    EXPECT_TRUE(g_testKeyV2.StoreKey(emptyUserAuth));
    EXPECT_TRUE(g_testKeyV2.ActiveKey());

    EXPECT_EQ(0, KeyCtrl::SetFscryptSyspara("2:aes-256-cts:aes-256-xts:padding-4"));
    EXPECT_EQ(0, KeyCtrl::InitFscryptPolicy());

    OHOS::ForceRemoveDirectory(TEST_DIR_V2);
    OHOS::ForceCreateDirectory(TEST_DIR_V2);
    EXPECT_TRUE(KeyCtrl::LoadAndSetPolicy(g_testKeyV2.GetDir(), TEST_DIR_V2));

    EXPECT_TRUE(OHOS::ForceCreateDirectory(TEST_DIR_V2 + "/test_dir"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/test_file1", "hello, world!\n"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/test_file2", "AA"));
    EXPECT_TRUE(OHOS::SaveStringToFile(TEST_DIR_V2 + "/111111111111111111111111111111111111111111111111", "AA"));

    struct fscrypt_get_policy_ex_arg arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    EXPECT_TRUE(KeyCtrl::GetPolicy(TEST_DIR_V2, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V2, arg.policy.version);
    EXPECT_EQ(FILENAME_MODES.at("aes-256-cts"), arg.policy.v2.filenames_encryption_mode);
    EXPECT_EQ(CONTENTS_MODES.at("aes-256-xts"), arg.policy.v2.contents_encryption_mode);
    EXPECT_EQ(POLICY_FLAGS.at("padding-4"), arg.policy.v2.flags);

    EXPECT_TRUE(g_testKeyV2.ClearKey());
}
