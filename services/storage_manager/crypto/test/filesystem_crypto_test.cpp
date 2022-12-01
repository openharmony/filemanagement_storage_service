/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "crypto/filesystem_crypto.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
namespace OHOS {
namespace StorageManager {
using namespace testing::ext;

class FileSystemCryptoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_GenerateUserKeys_0000
 * @tc.name: Storage_manager_crypto_GenerateUserKeys_0000
 * @tc.desc: Test function of GenerateUserKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_GenerateUserKeys_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_GenerateUserKeys_0000";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 108;
    uint32_t flags = 2; // UserKeys type
    uint32_t result = fileSystemCrypto_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(result, E_OK);

    fileSystemCrypto_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_GenerateUserKeys_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_GenerateUserKeys_0001
 * @tc.name: Storage_manager_crypto_GenerateUserKeys_0001
 * @tc.desc: Test function of GenerateUserKeys interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_GenerateUserKeys_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_GenerateUserKeys_0000";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 9999;
    uint32_t flags = 2; // UserKeys type
    uint32_t ret = fileSystemCrypto_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_GenerateUserKeys_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_DeleteUserKeys_0000
 * @tc.name: Storage_manager_crypto_DeleteUserKeys_0000
 * @tc.desc: Test function of DeleteUserKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_DeleteUserKeys_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_DeleteUserKeys_0000";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 109;
    uint32_t flags = 2; // UserKeys type
    uint32_t result = fileSystemCrypto_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(result, E_OK);

    fileSystemCrypto_->DeleteUserKeys(userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_DeleteUserKeys_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_DeleteUserKeys_0001
 * @tc.name: Storage_manager_crypto_DeleteUserKeys_0001
 * @tc.desc: Test function of DeleteUserKeys interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_DeleteUserKeys_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_DeleteUserKeys_0001";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 9999;
    uint32_t ret = fileSystemCrypto_->DeleteUserKeys(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_DeleteUserKeys_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_UpdateUserAuth_0000
 * @tc.name: Storage_manager_crypto_UpdateUserAuth_0000
 * @tc.desc: Test function of UpdateUserAuth interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_UpdateUserAuth_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_UpdateUserAuth_0000";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 110;
    uint32_t flags = 2; // UserKeys type
    uint32_t result = fileSystemCrypto_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(result, E_OK);
    result = fileSystemCrypto_->UpdateUserAuth(userId, {}, {}, {});
    EXPECT_EQ(result, E_OK);

    fileSystemCrypto_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_UpdateUserAuth_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_UpdateUserAuth_0001
 * @tc.name: Storage_manager_crypto_UpdateUserAuth_0001
 * @tc.desc: Test function of UpdateUserAuth interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_UpdateUserAuth_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_UpdateUserAuth_0001";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 9999;
    uint32_t ret = fileSystemCrypto_->UpdateUserAuth(userId, {}, {}, {});
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_UpdateUserAuth_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_ActiveUserKey_0000
 * @tc.name: Storage_manager_crypto_ActiveUserKey_0000
 * @tc.desc: Test function of ActiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_ActiveUserKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_ActiveUserKey_0000";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 111;
    uint32_t flags = 2; // UserKeys type
    uint32_t result = fileSystemCrypto_->GenerateUserKeys(userId, flags);
    EXPECT_EQ(result, E_OK);
    result = fileSystemCrypto_->ActiveUserKey(userId, {}, {});
    EXPECT_EQ(result, E_OK);

    fileSystemCrypto_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_ActiveUserKey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_ActiveUserKey_0001
 * @tc.name: Storage_manager_crypto_ActiveUserKey_0001
 * @tc.desc: Test function of ActiveUserKey interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_ActiveUserKey_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_ActiveUserKey_0001";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 9999;
    uint32_t ret = fileSystemCrypto_->ActiveUserKey(userId, {}, {});
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_ActiveUserKey_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_InactiveUserKey_0000
 * @tc.name: Storage_manager_crypto_InactiveUserKey_0000
 * @tc.desc: Test function of InactiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_InactiveUserKey_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_InactiveUserKey_0000";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t userId = 112;
    int32_t flags = 2; // UserKeys type
    int32_t ret = fileSystemCrypto_->GenerateUserKeys(userId, flags);
    ASSERT_EQ(ret, E_OK);

    ret = fileSystemCrypto_->ActiveUserKey(userId, {}, {});
    EXPECT_EQ(ret, E_OK);

    ret = fileSystemCrypto_->InactiveUserKey(userId);
    EXPECT_EQ(ret, E_OK);

    fileSystemCrypto_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_InactiveUserKey_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_InactiveUserKey_0001
 * @tc.name: Storage_manager_crypto_InactiveUserKey_0001
 * @tc.desc: Test function of InactiveUserKey interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_InactiveUserKey_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_InactiveUserKey_0001";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 9999;
    uint32_t ret = fileSystemCrypto_->InactiveUserKey(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_InactiveUserKey_0001";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_UpdateKeyContext_0000
 * @tc.name: Storage_manager_crypto_UpdateKeyContext_0000
 * @tc.desc: Test function of UpdateKeyContext interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_UpdateKeyContext_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_UpdateKeyContext_0000";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t userId = 113;
    int32_t flags = 2; // UserKeys type
    int32_t ret = fileSystemCrypto_->GenerateUserKeys(userId, flags);
    ASSERT_EQ(ret, E_OK);

    ret = fileSystemCrypto_->UpdateUserAuth(userId, {}, {}, {});
    EXPECT_EQ(ret, E_OK);

    ret = fileSystemCrypto_->UpdateKeyContext(userId);
    EXPECT_EQ(ret, E_OK);

    fileSystemCrypto_->DeleteUserKeys(userId);
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_UpdateKeyContext_0000";
}

/**
 * @tc.number: SUB_STORAGE_Storage_manager_crypto_UpdateKeyContext_0001
 * @tc.name: Storage_manager_crypto_UpdateKeyContext_0001
 * @tc.desc: Test function of UpdateKeyContext interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(FileSystemCryptoTest, Storage_manager_crypto_UpdateKeyContext_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSystemCryptoTest-start Storage_manager_crypto_UpdateKeyContext_0001";
    std::shared_ptr<FileSystemCrypto> fileSystemCrypto_ =
        DelayedSingleton<FileSystemCrypto>::GetInstance();
    uint32_t userId = 9999;
    uint32_t ret = fileSystemCrypto_->UpdateKeyContext(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "FileSystemCryptoTest-end Storage_manager_crypto_UpdateKeyContext_0001";
}
}
}