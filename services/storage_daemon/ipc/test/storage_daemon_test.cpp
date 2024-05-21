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

#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <mntent.h>
#include <unordered_map>

#include "ipc/istorage_daemon.h"
#include "ipc/storage_daemon.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "test/common/help_utils.h"
#include "user/user_manager.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
class StorageDaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    StorageDaemon* storageDaemon_;
};

void StorageDaemonTest::SetUpTestCase(void)
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
}

void StorageDaemonTest::SetUp()
{
    storageDaemon_ = new StorageDaemon();
    StorageTest::StorageTestUtils::ClearTestResource();
}

void StorageDaemonTest::TearDown(void)
{
    StorageTest::StorageTestUtils::ClearTestResource();
    if (storageDaemon_ != nullptr) {
        delete storageDaemon_;
        storageDaemon_ = nullptr;
    }
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_PrepareUserDirs_001
 * @tc.desc: check PrepareUserDirs when the el1 path exists but is not dir.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_PrepareUserDirs_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string filePath(StorageTest::StorageTestUtils::gRootDirs[0].path);
    filePath.replace(filePath.find("%s"), 2, "el1");
    filePath.replace(filePath.find("%d"), 2, std::to_string(StorageTest::USER_ID1));
    auto bRet = StorageTest::StorageTestUtils::CreateFile(filePath);
    EXPECT_TRUE(bRet) << "check the file create";

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemon_->PrepareUserDirs(StorageTest::USER_ID1, flags);
    EXPECT_TRUE(ret == E_PREPARE_DIR) << "the path is not dir";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_PrepareUserDirs_002
 * @tc.desc: check PrepareUserDirs when the flags is incorrect.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_PrepareUserDirs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_PrepareUserDirs_002 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1;
    auto ret = storageDaemon_->PrepareUserDirs(StorageTest::USER_ID2, flags);
    EXPECT_TRUE(ret == E_OK) << "the flags is incorrect";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_PrepareUserDirs_002 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_PrepareUserDirs_003
 * @tc.desc: Verify the PrepareUserDirs function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_PrepareUserDirs_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_PrepareUserDirs_003 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = storageDaemon_->PrepareUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK);

    storageDaemon_->DestroyUserDirs(StorageTest::USER_ID3, flags);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_PrepareUserDirs_003 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_StartUser_001
 * @tc.desc: check StartUser when user's dirs are not prepare.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StartUser_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t ret = storageDaemon_->StartUser(StorageTest::USER_ID1);
    EXPECT_TRUE(ret == E_MOUNT) << "user's dirs are not prepare";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StartUser_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_StartUser_002
 * @tc.desc: check the StartUser function when args are normal
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_StartUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StartUser_002 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = storageDaemon_->PrepareUserDirs(StorageTest::USER_ID5, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = storageDaemon_->StartUser(StorageTest::USER_ID5);
    EXPECT_TRUE(ret == E_OK) << "check StartUser";

    storageDaemon_->StopUser(StorageTest::USER_ID5);
    storageDaemon_->DestroyUserDirs(StorageTest::USER_ID5, flags);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StartUser_002 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_DestroyUserDirs_001
 * @tc.desc: Verify the DestroyUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_DestroyUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_DestroyUserDirs_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = storageDaemon_->PrepareUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK);

    ret = storageDaemon_->DestroyUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_DestroyUserDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_StopUser_001
 * @tc.desc: check the StopUser function when dir does not exist.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_StopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StopUser_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    auto ret = storageDaemon_->StopUser(StorageTest::USER_ID1);
    EXPECT_TRUE(ret == E_OK) << "dir is not mount";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StopUser_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_StopUser_002
 * @tc.desc: check the StopUser function when dir is not mount.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_StopUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StopUser_002 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = storageDaemon_->PrepareUserDirs(StorageTest::USER_ID4, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = storageDaemon_->StopUser(StorageTest::USER_ID4);
    EXPECT_TRUE(ret == E_OK) << "stop user error";

    storageDaemon_->DestroyUserDirs(StorageTest::USER_ID4, flags);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StopUser_002 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_StopUser_003
 * @tc.desc: check the StopUser function normal
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_StopUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StopUser_003 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL1 | IStorageDaemon::CRYPTO_FLAG_EL2;
    auto ret = storageDaemon_->PrepareUserDirs(StorageTest::USER_ID3, flags);
    EXPECT_TRUE(ret == E_OK) << "PrepareUserDirs error";
    ret = storageDaemon_->StartUser(StorageTest::USER_ID3);
    EXPECT_TRUE(ret == E_OK) << "StartUser error";

    ret = storageDaemon_->StopUser(StorageTest::USER_ID3);
    EXPECT_TRUE(ret == E_OK) << "check StopUser error";

    storageDaemon_->DestroyUserDirs(StorageTest::USER_ID3, flags);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_StopUser_003 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_Shutdown_001
 * @tc.desc: check the StopUser function normal
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_Shutdown_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Shutdown_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    auto ret = storageDaemon_->Shutdown();
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Shutdown_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_MountDfsDocs_001
 * @tc.desc: check the StopUser function normal
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_MountDfsDocs_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    int32_t userId = 105;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    auto ret = storageDaemon_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_PREPARE_DIR);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_MountDfsDocs_001 end";
}

#ifdef EXTERNAL_STORAGE_MANAGER
/**
 * @tc.name: Storage_Manager_StorageDaemonTest_Mount_001
 * @tc.desc: check the Mount function when volume not exist
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_Mount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Mount_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string volId = "vol-1-1";
    uint32_t flag = 0;
    auto ret = storageDaemon_->Mount(volId, flag);
    EXPECT_TRUE(ret == E_NON_EXIST);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Mount_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_UMount_001
 * @tc.desc: check the UMount function when volume not exist
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_UMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_UMount_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string volId = "vol-1-2";
    auto ret = storageDaemon_->UMount(volId);
    EXPECT_TRUE(ret == E_NON_EXIST);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_UMount_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_Check_001
 * @tc.desc: check the Check function when volume not exist
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_Check_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Check_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string volId = "vol-1-3";
    auto ret = storageDaemon_->Check(volId);
    EXPECT_TRUE(ret == E_NON_EXIST);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Check_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_Format_001
 * @tc.desc: check the Format function when volume not exist
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_Format_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Format_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string volId = "vol-1-4";
    std::string fsType = "exfat";
    auto ret = storageDaemon_->Format(volId, fsType);
    EXPECT_TRUE(ret == E_NON_EXIST);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Format_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_Partition_001
 * @tc.desc: check the Partition function when disk not exist
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_Partition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Partition_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string diskId = "disk-1-0";
    int32_t type = 0;
    auto ret = storageDaemon_->Partition(diskId, type);
    EXPECT_TRUE(ret == E_NON_EXIST);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_Partition_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_SetVolumeDescription_001
 * @tc.desc: check the SetVolumeDescription function when volume not exist
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_SetVolumeDescription_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_SetVolumeDescription_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string volId = "vol-1-5";
    std::string des = "des";
    auto ret = storageDaemon_->SetVolumeDescription(volId, des);
    EXPECT_TRUE(ret == E_NON_EXIST);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_SetVolumeDescription_001 end";
}
#endif

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_SetBundleQuota_001
 * @tc.desc: check the SetBundleQuota function
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_SetBundleQuota_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_SetBundleQuota_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::string bundleName = "com.ohos.bundleName-0-1";
    std::string bundleDataDirPath = "/data/app/el2/100/base/" + bundleName;
    int32_t uid = 20000000;
    int32_t limitSizeMb = 1000;
    auto ret = storageDaemon_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_TRUE(ret == E_SYS_CALL);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_SetBundleQuota_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_GetBundleStatsForIncrease_001
 * @tc.desc: check the GetBundleStatsForIncrease function
 * @tc.type: FUNC
 * @tc.require: AR000IGCR7
 */
HWTEST_F(StorageDaemonTest, Storage_Manager_StorageDaemonTest_GetBundleStatsForIncrease_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_GetBundleStatsForIncrease_001 start";

    ASSERT_TRUE(storageDaemon_ != nullptr);
    // create parameter
    uint32_t userId = 100;
    std::vector<std::string> bundleNames;
    std::string bundleName = "com.ohos.UserFile.ExternalFileManager";
    bundleNames.emplace_back(bundleName);
    std::vector<int64_t> incrementalBackTimes;
    int64_t lastIncrementalTime = 0;
    incrementalBackTimes.emplace_back(lastIncrementalTime);
    std::vector<int64_t> pkgFileSizes;
    // prepare config file
    mode_t mode = 0771;
    // backup_sa bundle path
    std::string backupSaBundleDir = BACKUP_PATH_PREFIX + std::to_string(userId) + BACKUP_PATH_SURFFIX + bundleName +
        FILE_SEPARATOR_CHAR;
    if (!StorageTest::StorageTestUtils::CheckDir(backupSaBundleDir)) {
        StorageTest::StorageTestUtils::MkDir(backupSaBundleDir, mode);
    }
    // backup_sa include/exclude
    std::string incExFilePath = backupSaBundleDir + BACKUP_INCEXC_SYMBOL + std::to_string(lastIncrementalTime);
    std::ofstream incExcFile;
    incExcFile.open(incExFilePath.data(), std::ios::out | std::ios::trunc);
    if (!incExcFile.is_open()) {
        EXPECT_TRUE(false) << "Failed to create file for include-exclude config";
    }
    incExcFile << BACKUP_INCLUDE << std::endl;
    std::string include = "/storage/Users/currentUser/";
    incExcFile << include << std::endl;

    incExcFile << BACKUP_EXCLUDE << std::endl;
    std::string exclude = "/storage/Users/currentUser/.Trash/";
    incExcFile << exclude << std::endl;
    incExcFile.close();

    // backup_sa stat
    std::string statFilePath = backupSaBundleDir + BACKUP_STAT_SYMBOL + std::to_string(lastIncrementalTime);
    std::ofstream statFile;
    statFile.open(statFilePath.data(), std::ios::out | std::ios::trunc);
    if (!statFile.is_open()) {
        EXPECT_TRUE(false) << "Failed to create file for bundle stat";
    }
    statFile.close();
    // invoke GetBundleStatsForIncrease
    auto ret = storageDaemon_->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes);
    EXPECT_TRUE(ret == E_OK);
    EXPECT_GE(pkgFileSizes[0], 0);

    if (StorageTest::StorageTestUtils::CheckDir(backupSaBundleDir)) {
        StorageTest::StorageTestUtils::RmDirRecurse(backupSaBundleDir);
    }

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_GetBundleStatsForIncrease_001 end";
}
} // STORAGE_DAEMON
} // OHOS
