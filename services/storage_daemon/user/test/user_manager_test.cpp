/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ipc/istorage_daemon.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "test/common/help_utils.h"
#include "user/user_manager.h"
#include "utils/file_utils.h"
#include "crypto/key_manager.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class UserManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown();
};

void UserManagerTest::SetUpTestCase(void)
{
    std::vector<std::string> paths = {
        "/data/app",
        "/data/app/el1",
        "/data/app/el2",

        "/data/service",
        "/data/service/el1",
        "/data/service/el2",

        "/data/chipset",
        "/data/chipset/el1",
        "/data/chipset/el2",

        "/storage",
        "/storage/media"
    };

    mode_t mode = 0711;
    for (auto path : paths) {
        if (!StorageTest::StorageTestUtils::CheckDir(path)) {
            StorageTest::StorageTestUtils::MkDir(path, mode);
        }
    }
    StorageTest::StorageTestUtils::ClearTestResource();
}

void UserManagerTest::TearDown()
{
    StorageTest::StorageTestUtils::ClearTestResource();
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_Instance_001
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_Instance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_Instance_002
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_Instance_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_002 start";

    std::shared_ptr<UserManager> userManagerFirst = UserManager::GetInstance();
    ASSERT_TRUE(userManagerFirst != nullptr);
    std::shared_ptr<UserManager> userManagerSecond = UserManager::GetInstance();
    ASSERT_TRUE(userManagerSecond != nullptr);

    EXPECT_TRUE(userManagerFirst == userManagerSecond);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_002 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_PrepareUserDirs_001
 * @tc.desc: func PrepareUserDirs when the el1 path exist but is not dir.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    std::string filePath(StorageTest::StorageTestUtils::g_rootDirs[0].path);
    filePath.replace(filePath.find("%s"), 2, "el1");
    filePath.replace(filePath.find("%d"), 2, std::to_string(StorageTest::StorageTestUtils::USER_ID1));
    auto bRet = StorageTest::StorageTestUtils::CreateFile(filePath);
    EXPECT_TRUE(bRet) << "check the file create";

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = userManager->PrepareUserDirs(StorageTest::StorageTestUtils::USER_ID1, flags);
    EXPECT_TRUE(ret == E_PREPARE_DIR) << "the path is not dir";

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_PrepareUserDirs_002
 * @tc.desc: func PrepareUserDirs when the flags is incorrect.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_PrepareUserDirs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_002 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    std::string filePath(StorageTest::StorageTestUtils::g_rootDirs[0].path);
    filePath.replace(filePath.find("%s"), 2, "el1");
    filePath.replace(filePath.find("%d"), 2, std::to_string(StorageTest::StorageTestUtils::USER_ID1));
    auto bRet = StorageTest::StorageTestUtils::CreateFile(filePath);
    EXPECT_TRUE(bRet) << "check the file create";

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1;
    int32_t ret = userManager->PrepareUserDirs(StorageTest::StorageTestUtils::USER_ID1, flags);
    EXPECT_TRUE(ret == E_PREPARE_DIR) << "the flags is incorrect";

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_002 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_PrepareUserDirs_003
 * @tc.desc: check PrepareUserDirs when args are normal
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_PrepareUserDirs_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_002 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::StorageTestUtils::USER_ID5, flags);
    EXPECT_EQ(ret, E_OK);

    ret = userManager->PrepareUserDirs(StorageTest::StorageTestUtils::USER_ID5, flags);
    EXPECT_TRUE(ret == E_OK);
    userManager->DestroyUserDirs(StorageTest::StorageTestUtils::USER_ID5, flags);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::StorageTestUtils::USER_ID5);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_003 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_StartUser_001
 * @tc.desc: check the StartUser function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StartUser_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);
    userManager->StopUser(StorageTest::StorageTestUtils::USER_ID3);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::StorageTestUtils::USER_ID3, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::StorageTestUtils::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = userManager->StartUser(StorageTest::StorageTestUtils::USER_ID3);
    EXPECT_TRUE(ret == E_OK) << "start user error";

    userManager->StopUser(StorageTest::StorageTestUtils::USER_ID3);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::StorageTestUtils::USER_ID3);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StartUser_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_StartUser_002
 * @tc.desc: check StartUser function when user's dirs are not prepare.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_StartUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StartUser_002 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t ret = userManager->StartUser(StorageTest::StorageTestUtils::USER_ID1);
    EXPECT_TRUE(ret == E_MOUNT) << "user's dirs are not prepare";

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StartUser_002 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_DestroyUserDirs_001
 * @tc.desc: check DestroyUserDirs function
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_DestroyUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_DestroyUserDirs_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::StorageTestUtils::USER_ID4, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::StorageTestUtils::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK);

    ret = userManager->DestroyUserDirs(StorageTest::StorageTestUtils::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::StorageTestUtils::USER_ID4);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_DestroyUserDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_StopUser_001
 * @tc.desc: check the StopUser function when dir does not exist.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_StopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StopUser_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    auto ret = userManager->StopUser(StorageTest::StorageTestUtils::USER_ID2);
    EXPECT_TRUE(ret == E_UMOUNT) << "dir is not mount";

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StopUser_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_StopUser_002
 * @tc.desc: check the StopUser function when dir is not mount.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_StopUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StopUser_002 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::StorageTestUtils::USER_ID4, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::StorageTestUtils::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = userManager->StopUser(StorageTest::StorageTestUtils::USER_ID4);
    EXPECT_TRUE(ret == E_UMOUNT) << "dir is not mount";

    userManager->DestroyUserDirs(StorageTest::StorageTestUtils::USER_ID4, flags);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::StorageTestUtils::USER_ID4);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StopUser_002 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_StopUser_003
 * @tc.desc: check the StopUser function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_StopUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StopUser_003 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::StorageTestUtils::USER_ID3, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::StorageTestUtils::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";
    ret = userManager->StartUser(StorageTest::StorageTestUtils::USER_ID3);
    EXPECT_TRUE(ret == E_OK) << "start user error";

    ret = userManager->StopUser(StorageTest::StorageTestUtils::USER_ID3);
    EXPECT_TRUE(ret == E_OK) << "stop user error";

    userManager->DestroyUserDirs(StorageTest::StorageTestUtils::USER_ID3, flags);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::StorageTestUtils::USER_ID3);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StopUser_003 end";
}
} // STORAGE_DAEMON
} // OHOS
