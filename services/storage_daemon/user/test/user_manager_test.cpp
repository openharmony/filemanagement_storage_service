/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "directory_ex.h"
#include "istorage_daemon.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "test/common/help_utils.h"
#include "user/mount_manager.h"
#include "user/user_manager.h"
#include "utils/file_utils.h"
#include "crypto/key_manager.h"
#include "user/system_mount_manager.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {

using namespace testing;
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
 * @tc.name: Storage_Manager_UserManagerTest_GetInstance_001
 * @tc.desc: Verify the GetInstance function.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_GetInstance_001 start";

    UserManager &userManager1 = UserManager::GetInstance();
    UserManager &userManager2 = UserManager::GetInstance();
    ASSERT_TRUE(&userManager1 == &userManager2);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_GetInstance_001 end";
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

    std::string filePath(StorageTest::StorageTestUtils::gRootDirs[0].path);
    filePath.replace(filePath.find("%s"), 2, "el1");
    filePath.replace(filePath.find("%d"), 2, std::to_string(StorageTest::USER_ID1));
    auto bRet = StorageTest::StorageTestUtils::CreateFile(filePath);
    EXPECT_TRUE(bRet) << "check the file create";

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2 |
                    IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4;
    int32_t ret = UserManager::GetInstance().PrepareUserDirs(StorageTest::USER_ID1, flags);
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

    std::string filePath(StorageTest::StorageTestUtils::gRootDirs[0].path);
    filePath.replace(filePath.find("%s"), 2, "el1");
    filePath.replace(filePath.find("%d"), 2, std::to_string(StorageTest::USER_ID1));
    auto bRet = StorageTest::StorageTestUtils::CreateFile(filePath);
    EXPECT_TRUE(bRet) << "check the file create";

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1;
    int32_t ret = UserManager::GetInstance().PrepareUserDirs(StorageTest::USER_ID1, flags);
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

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2 |
                    IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4;
    auto ret = KeyManager::GetInstance().GenerateUserKeys(StorageTest::USER_ID5, flags);
    EXPECT_EQ(ret, E_OK);

    ret = UserManager::GetInstance().PrepareUserDirs(StorageTest::USER_ID5, flags);
    EXPECT_TRUE(ret == E_OK);
    UserManager::GetInstance().DestroyUserDirs(StorageTest::USER_ID5, flags);
    KeyManager::GetInstance().DeleteUserKeys(StorageTest::USER_ID5);
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

    int32_t userId = -1;
    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2 |
                    IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4;
    auto ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
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

    UserManager::GetInstance().StopUser(StorageTest::USER_ID3);

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance().GenerateUserKeys(StorageTest::USER_ID3, flags);
    EXPECT_EQ(ret, E_OK);
    ret = UserManager::GetInstance().PrepareUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = UserManager::GetInstance().StartUser(StorageTest::USER_ID3);

    UserManager::GetInstance().StopUser(StorageTest::USER_ID3);
    KeyManager::GetInstance().DeleteUserKeys(StorageTest::USER_ID3);
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

    int32_t ret = UserManager::GetInstance().StartUser(StorageService::START_USER_ID - 1);
    EXPECT_TRUE(ret == E_USERID_RANGE) << "user's dirs are not prepare";

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

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance().GenerateUserKeys(StorageTest::USER_ID4, flags);
    EXPECT_EQ(ret, E_OK);
    ret = UserManager::GetInstance().PrepareUserDirs(StorageTest::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK);

    ret = UserManager::GetInstance().DestroyUserDirs(StorageTest::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK);
    KeyManager::GetInstance().DeleteUserKeys(StorageTest::USER_ID4);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_DestroyUserDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_CheckDirsFromVec_001
 * @tc.desc: Verify the CheckDirsFromVec function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_CheckDirsFromVec_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_CheckDirsFromVec_001 start";

    int32_t userId = 405;
    UserManager::GetInstance().CheckDirsFromVec(userId);
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_CheckDirsFromVec_001 end";
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

    auto ret = UserManager::GetInstance().StopUser(StorageTest::USER_ID2);
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

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance().GenerateUserKeys(StorageTest::USER_ID4, flags);
    EXPECT_EQ(ret, E_OK);
    ret = UserManager::GetInstance().PrepareUserDirs(StorageTest::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = UserManager::GetInstance().StopUser(StorageTest::USER_ID4);
    EXPECT_TRUE(ret == E_OK) << "dir mount success";

    UserManager::GetInstance().DestroyUserDirs(StorageTest::USER_ID4, flags);
    KeyManager::GetInstance().DeleteUserKeys(StorageTest::USER_ID4);
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

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    auto ret = KeyManager::GetInstance().GenerateUserKeys(StorageTest::USER_ID3, flags);
    EXPECT_EQ(ret, E_OK);
    ret = UserManager::GetInstance().PrepareUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";
    ret = UserManager::GetInstance().StartUser(StorageTest::USER_ID3);

    ret = UserManager::GetInstance().StopUser(StorageTest::USER_ID3);

    UserManager::GetInstance().DestroyUserDirs(StorageTest::USER_ID3, flags);
    KeyManager::GetInstance().DeleteUserKeys(StorageTest::USER_ID3);
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

    int32_t userId = StorageService::START_USER_ID - 1;
    auto ret = UserManager::GetInstance().CheckUserIdRange(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);

    userId = StorageService::MAX_USER_ID + 1;
    ret = UserManager::GetInstance().CheckUserIdRange(userId);
    EXPECT_EQ(ret, E_USERID_RANGE);

    userId = StorageService::ZERO_USER;
    ret = UserManager::GetInstance().CheckUserIdRange(userId);
    EXPECT_EQ(ret, E_OK);

    userId = 101;
    ret = UserManager::GetInstance().CheckUserIdRange(userId);
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

    ProcessInfo info;
    std::string filename = "";

    bool ret = MountManager::GetInstance().GetProcessInfo(filename, info);
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

    ProcessInfo info;
    std::string filename = "/data";

    bool ret = MountManager::GetInstance().GetProcessInfo(filename, info);
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

    uint32_t userId = 100;

    int32_t ret = MountManager::GetInstance().MountCryptoPathAgain(userId);
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

    SystemMountManager::GetInstance().SetCloudState(true);
    SystemMountManager::GetInstance().SetCloudState(false);
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

    int32_t userId = 100;
    auto ret = MountManager::GetInstance().LocalUMount(userId);
    EXPECT_EQ(ret, E_UMOUNT_LOCAL_CLOUD);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_LocalMmount_001 end";
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

    int32_t userId = 100;
    int32_t ret = UserManager::GetInstance().RestoreconSystemServiceDirs(userId);
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

    int32_t userId = -1;
    std::list<std::string> unMountFailList;
    unMountFailList.push_back("test1");
    unMountFailList.push_back("test2");
    int32_t ret = MountManager::GetInstance().FindAndKillProcess(userId, unMountFailList, E_USER_UMOUNT_ERR);
    EXPECT_EQ(ret, E_UMOUNT_NO_PROCESS_FIND);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_FindAndKillProcess_000 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_CheckMaps_001
 * @tc.desc: Verify the CheckMaps function.
 * @tc.type: FUNC
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_CheckMaps_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_CheckMaps_000 start";

    std::string path = "";
    std::list<std::string> unMountFailList;
    unMountFailList.push_back("test1");
    MountManager::GetInstance().CheckMaps(path, unMountFailList);
    MountManager::GetInstance().CheckSymlink(path, unMountFailList);

    path = "/data/file";
    MountManager::GetInstance().CheckMaps(path, unMountFailList);
    MountManager::GetInstance().CheckSymlink(path, unMountFailList);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_CheckMaps_000 end";
}
#ifdef STORAGE_SERVICE_MEDIA_FUSE
/**
 * @tc.name: Storage_Manager_MountManagerTest_MountMediaFuse_001
 * @tc.desc: Verify the MountMediaFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_MountMediaFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountMediaFuse_001 start";

    int32_t userId = 101;
    int32_t devFd = -1;
    int32_t ret = MountManager::GetInstance().MountMediaFuse(userId, devFd);
    EXPECT_EQ(ret, E_USER_MOUNT_ERR);

    ret = MountManager::GetInstance().UMountMediaFuse(userId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountMediaFuse_001 end";
}
#endif

/**
 * @tc.name: Storage_Manager_MountManagerTest_CreateUserDir_001
 * @tc.desc: Verify the CreateUserDir function.
 * @tc.type: FUNC
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_CreateUserDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_CreateUserDir_001 start";
    std::string path;
    mode_t mode = 771;
    uid_t uid = 0;
    gid_t gid = 0;

    int32_t ret = UserManager::GetInstance().CreateUserDir(path, mode, uid, gid);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    path = "/data/virt_service/rgm_hmos/anco_hmos_data";
    std::error_code errCode;
    if (!std::filesystem::exists(path, errCode)) {
        std::filesystem::create_directories(path, errCode);
    }
    
    ret = UserManager::GetInstance().CreateUserDir(path + "/testDir", mode, uid, gid);
    EXPECT_EQ(ret, E_OK);
    ret = UserManager::GetInstance().CreateUserDir(path  + "/testDir", mode, uid, gid);
    EXPECT_EQ(ret, E_CREATE_USER_DIR_EXIST);

    ret = UserManager::GetInstance().CreateUserDir(path + "/testDir/testDir/testDir", mode, uid, gid);
    EXPECT_NE(ret, E_OK);

    OHOS::ForceRemoveDirectory(path + "/testDir");
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_CreateUserDir_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_CreateElxBundleDataDir_001
 * @tc.desc: Verify the CreateElxBundleDataDir function.
 * @tc.type: FUNC
 */
HWTEST_F(UserManagerTest, Storage_Manager_MountManagerTest_CreateElxBundleDataDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_CreateElxBundleDataDir_001 start";
    int32_t userId = 100;
    int32_t elx = 1;
    UserManager::GetInstance().CreateElxBundleDataDir(userId, elx);
    elx = 2;
    UserManager::GetInstance().CreateElxBundleDataDir(userId, elx);
    userId = -1;
    UserManager::GetInstance().CreateElxBundleDataDir(userId, elx);

    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_CreateElxBundleDataDir_001 end";
}
} // STORAGE_DAEMON
} // OHOS
