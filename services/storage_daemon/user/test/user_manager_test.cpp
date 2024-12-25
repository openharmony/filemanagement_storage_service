/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "user/mount_manager.h"
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

    std::string filePath(StorageTest::StorageTestUtils::gRootDirs[0].path);
    filePath.replace(filePath.find("%s"), 2, "el1");
    filePath.replace(filePath.find("%d"), 2, std::to_string(StorageTest::USER_ID1));
    auto bRet = StorageTest::StorageTestUtils::CreateFile(filePath);
    EXPECT_TRUE(bRet) << "check the file create";

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2 |
                    IStorageDaemon::CRYPTO_FLAG_EL3 | IStorageDaemon::CRYPTO_FLAG_EL4;
    int32_t ret = userManager->PrepareUserDirs(StorageTest::USER_ID1, flags);
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

    std::string filePath(StorageTest::StorageTestUtils::gRootDirs[0].path);
    filePath.replace(filePath.find("%s"), 2, "el1");
    filePath.replace(filePath.find("%d"), 2, std::to_string(StorageTest::USER_ID1));
    auto bRet = StorageTest::StorageTestUtils::CreateFile(filePath);
    EXPECT_TRUE(bRet) << "check the file create";

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1;
    int32_t ret = userManager->PrepareUserDirs(StorageTest::USER_ID1, flags);
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
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_003 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2 |
                    IStorageDaemon::CRYPTO_FLAG_EL3 | IStorageDaemon::CRYPTO_FLAG_EL4;
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::USER_ID5, flags);
    EXPECT_EQ(ret, E_OK);

    ret = userManager->PrepareUserDirs(StorageTest::USER_ID5, flags);
    EXPECT_TRUE(ret == E_OK);
    userManager->DestroyUserDirs(StorageTest::USER_ID5, flags);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::USER_ID5);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_003 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_PrepareUserDirs_004
 * @tc.desc: check PrepareUserDirs when user id is invalid
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_PrepareUserDirs_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_004 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t userId = -1;
    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2 |
                    IStorageDaemon::CRYPTO_FLAG_EL3 | IStorageDaemon::CRYPTO_FLAG_EL4;
    auto ret = userManager->PrepareUserDirs(userId, flags);
    EXPECT_FALSE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_PrepareUserDirs_004 end";
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
    userManager->StopUser(StorageTest::USER_ID3);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::USER_ID3, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = userManager->StartUser(StorageTest::USER_ID3);

    userManager->StopUser(StorageTest::USER_ID3);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::USER_ID3);
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

    int32_t ret = userManager->StartUser(StorageTest::USER_ID1);
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
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::USER_ID4, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK);

    ret = userManager->DestroyUserDirs(StorageTest::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::USER_ID4);
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

    auto ret = userManager->StopUser(StorageTest::USER_ID2);
    EXPECT_TRUE(ret == E_OK) << "dir mount success";

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
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::USER_ID4, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = userManager->StopUser(StorageTest::USER_ID4);
    EXPECT_TRUE(ret == E_OK) << "dir mount success";

    userManager->DestroyUserDirs(StorageTest::USER_ID4, flags);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::USER_ID4);
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
    auto ret = KeyManager::GetInstance()->GenerateUserKeys(StorageTest::USER_ID3, flags);
    EXPECT_EQ(ret, E_OK);
    ret = userManager->PrepareUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";
    ret = userManager->StartUser(StorageTest::USER_ID3);

    ret = userManager->StopUser(StorageTest::USER_ID3);

    userManager->DestroyUserDirs(StorageTest::USER_ID3, flags);
    KeyManager::GetInstance()->DeleteUserKeys(StorageTest::USER_ID3);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_StopUser_003 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_RestoreconSystemServiceDirs_001
 * @tc.desc: Verify the CheckUserIdRange function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_CheckUserIdRange_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_CheckUserIdRange_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::GetInstance();
    ASSERT_TRUE(userManager != nullptr);

    int32_t userId = StorageService::START_USER_ID - 1;
    auto ret = userManager->CheckUserIdRange(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);

    userId = StorageService::MAX_USER_ID + 1;
    ret = userManager->CheckUserIdRange(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);

    userId = StorageService::ZERO_USER;
    ret = userManager->CheckUserIdRange(userId);
    EXPECT_EQ(ret, E_OK);

    userId = 101;
    ret = userManager->CheckUserIdRange(userId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_CheckUserIdRange_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_Instance_001
 * @tc.desc: Verify the MountManager function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_GetProcessInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_GetProcessInfo_001 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    ProcessInfo info;
    std::string filename = "";

    bool ret = mountManager->GetProcessInfo(filename, info);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_GetProcessInfo_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_GetProcessInfo_002
 * @tc.desc: Verify the MountManager function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_GetProcessInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_GetProcessInfo_002 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);
    ProcessInfo info;
    std::string filename = "/data";

    bool ret = mountManager->GetProcessInfo(filename, info);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_GetProcessInfo_002 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountCryptoPathAgain_001
 * @tc.desc: Verify the MountManager function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_MountCryptoPathAgain_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountCryptoPathAgain_001 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);
    uint32_t userId = 100;

    int32_t ret = mountManager->MountCryptoPathAgain(userId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountCryptoPathAgain_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountCloudForUsers_001
 * @tc.desc: Verify the MountManager function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_SetCloudState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_SetCloudState_001 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    mountManager->SetCloudState(true);
    mountManager->SetCloudState(false);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_SetCloudState_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountCloudForUsers_001
 * @tc.desc: Verify the MountManager function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_LocalMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_LocalMount_001 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    int32_t userId = 100;
    int32_t ret = mountManager->LocalMount(userId);
    EXPECT_EQ(ret, E_OK);

    ret = mountManager->LocalUMount(userId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_LocalMmount_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountDfsDocs_001
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_001 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    int32_t userId = 100;
    std::string relativePath = "/data";
    std::string deviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    int32_t ret = mountManager->MountDfsDocs(userId, relativePath, deviceId, deviceId);
    EXPECT_EQ(ret, E_MOUNT);

    ret = mountManager->UMountDfsDocs(userId, relativePath, deviceId, deviceId);
    EXPECT_EQ(ret, E_UMOUNT);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_RestoreconSystemServiceDirs_001
 * @tc.desc: Verify the RestoreconSystemServiceDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_RestoreconSystemServiceDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_RestoreconSystemServiceDirs_001 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    int32_t userId = 100;
    int32_t ret = mountManager->RestoreconSystemServiceDirs(userId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_RestoreconSystemServiceDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_FindAndKillProcess_001
 * @tc.desc: Verify the FindAndKillProcess function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_FindAndKillProcess_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_FindAndKillProcess_000 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    int32_t userId = -1;
    std::list<std::string> unMountFailList;
    unMountFailList.push_back("test1");
    unMountFailList.push_back("test2");
    int32_t ret = mountManager->FindAndKillProcess(userId, unMountFailList, E_UMOUNT);
    EXPECT_EQ(ret, E_UMOUNT_NO_PROCESS_FIND);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_FindAndKillProcess_000 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_UmountFailRadar_001
 * @tc.desc: Verify the UmountFailRadar function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_UmountFailRadar_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UmountFailRadar_000 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    ProcessInfo processInfo;
    processInfo.pid = 1234;
    processInfo.name = "test";
    int32_t radar = E_UMOUNT;
    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(processInfo);
    mountManager->UmountFailRadar(processInfos, E_UMOUNT);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UmountFailRadar_000 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_CheckMaps_001
 * @tc.desc: Verify the CheckMaps function.
 * @tc.type: FUNC
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_CheckMaps_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UmountFailRadar_000 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    std::string path = "";
    std::list<std::string> unMountFailList;
    unMountFailList.push_back("test1");
    mountManager->CheckMaps(path, unMountFailList);
    mountManager->CheckSymlink(path, unMountFailList);

    path = "/data/file";
    mountManager->CheckMaps(path, unMountFailList);
    mountManager->CheckSymlink(path, unMountFailList);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UmountFailRadar_000 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountMediaFuse_001
 * @tc.desc: Verify the MountMediaFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_MountMediaFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountMediaFuse_001 start";

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    int32_t userId = 101;
    int32_t devFd = -1;
    int32_t ret = mountManager->MountMediaFuse(userId, devFd);
    EXPECT_EQ(ret, E_MOUNT);

    ret = mountManager->UMountMediaFuse(userId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountMediaFuse_001 end";
}
} // STORAGE_DAEMON
} // OHOS
