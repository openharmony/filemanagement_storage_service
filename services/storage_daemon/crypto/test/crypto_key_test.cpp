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
#include "file_ex.h"
#include "directory_ex.h"

using namespace testing::ext;

class CryptoKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::string testKeyPath { "/data/test/sys_de" };
    OHOS::StorageDaemon::UserAuth emptyUserAuth { "", "" };
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
 * @tc.name: basekey_init
 * @tc.desc: Verify the InitKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_init, TestSize.Level1)
{
    OHOS::StorageDaemon::BaseKey deKey(testKeyPath, emptyUserAuth);

    EXPECT_EQ(true, deKey.InitKey());

    EXPECT_EQ(OHOS::StorageDaemon::FS_AES_256_XTS_KEY_SIZE, deKey.keyInfo.key.size);
    EXPECT_NE(nullptr, deKey.keyInfo.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, deKey.keyInfo.keyDesc.size);
    EXPECT_NE(nullptr, deKey.keyInfo.keyDesc.data.get());
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
    OHOS::StorageDaemon::BaseKey deKey(testKeyPath, emptyUserAuth);

    EXPECT_EQ(true, deKey.InitKey());
    EXPECT_EQ(true, deKey.StoreKey());

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
    OHOS::StorageDaemon::BaseKey deKey(testKeyPath, emptyUserAuth);

    EXPECT_EQ(true, deKey.RestoreKey());

    EXPECT_EQ(OHOS::StorageDaemon::FS_AES_256_XTS_KEY_SIZE, deKey.keyInfo.key.size);
    EXPECT_NE(nullptr, deKey.keyInfo.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, deKey.keyInfo.keyDesc.size);
    EXPECT_NE(nullptr, deKey.keyInfo.keyDesc.data.get());
}

/**
 * @tc.name: basekey_install
 * @tc.desc: Verify the ActiveKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_install, TestSize.Level1)
{
    OHOS::StorageDaemon::BaseKey deKey(testKeyPath, emptyUserAuth);

    EXPECT_EQ(true, deKey.RestoreKey());
    EXPECT_EQ(false, deKey.keyInfo.key.IsEmpty());

    EXPECT_EQ(true, deKey.ActiveKey());
}

/**
 * @tc.name: basekey_clear
 * @tc.desc: Verify the ClearKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_clear, TestSize.Level1)
{
    OHOS::StorageDaemon::BaseKey deKey(testKeyPath, emptyUserAuth);

    EXPECT_EQ(true, deKey.RestoreKey());
    EXPECT_EQ(false, deKey.keyInfo.key.IsEmpty());

    EXPECT_EQ(true, deKey.ClearKey());
    EXPECT_EQ(true, deKey.keyInfo.key.IsEmpty());
    EXPECT_EQ(false, OHOS::FileExists(testKeyPath));
}
