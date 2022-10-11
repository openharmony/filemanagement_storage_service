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

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"
#include "common/help_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

namespace {
    const uint32_t ALL_PERMS = (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);
    const std::string PATH_CHMOD = "/data/storage_daemon_chmod_test_dir";
    const std::string PATH_CHOWN = "/data/storage_daemon_chown_test_dir";
    const std::string PATH_RMDIR = "/data/storage_daemon_rmdir_test_dir";
    const std::string PATH_MKDIR = "/data/storage_daemon_mkdir_test_dir";
    const std::string PATH_MOUNT = "/data/storage_daemon_mount_test_dir";
}

int32_t ChMod(const std::string &path, mode_t mode);
int32_t ChOwn(const std::string &path, uid_t uid, gid_t gid);
int32_t MkDir(const std::string &path, mode_t mode);
int32_t RmDir(const std::string &path);

class FileUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
};

void FileUtilsTest::SetUp(void)
{
    mode_t mode = 002;
    umask(mode);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_CHMOD);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_CHOWN);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_MKDIR);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_RMDIR);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_MOUNT);
}

void FileUtilsTest::TearDown(void)
{
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_CHMOD);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_CHOWN);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_MKDIR);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_RMDIR);
    StorageTest::StorageTestUtils::RmDirRecurse(PATH_MOUNT);
}

/**
 * @tc.name: FileUtilsTest_ChMod_001
 * @tc.desc: Verify the ChMod function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ChMod_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ChMod_001 start";

    mode_t mode = 0660;
    bool bRet = StorageTest::StorageTestUtils::MkDir(PATH_CHMOD, mode);
    ASSERT_TRUE(bRet);
    struct stat st;
    int32_t ret = lstat(PATH_CHMOD.c_str(), &st);
    ASSERT_TRUE(ret == 0);
    EXPECT_TRUE((st.st_mode & ALL_PERMS) == mode);

    mode = 0771;
    ret = ChMod(std::string(PATH_CHMOD), mode);
    ASSERT_TRUE(ret == E_OK);

    ret = lstat(PATH_CHMOD.c_str(), &st);
    ASSERT_TRUE(ret == 0);
    EXPECT_TRUE((st.st_mode & ALL_PERMS) == mode);

    GTEST_LOG_(INFO) << "FileUtilsTest_ChMod_001 end";
}

/**
 * @tc.name: FileUtilsTest_ChOwn_001
 * @tc.desc: Verify the ChOwn function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ChOwn_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ChOwn_001 start";

    mode_t mode = 0660;
    bool bRet = StorageTest::StorageTestUtils::MkDir(PATH_CHOWN, mode);
    ASSERT_TRUE(bRet);
    ASSERT_TRUE(StorageTest::StorageTestUtils::CheckDir(PATH_CHOWN));

    uid_t uid = 00;
    gid_t gid = 01;
    int32_t ret = ChOwn(PATH_CHOWN, uid, gid);
    ASSERT_TRUE(ret == E_OK);

    struct stat st;
    ret = lstat(PATH_CHOWN.c_str(), &st);
    ASSERT_TRUE(ret == 0);
    EXPECT_TRUE(st.st_uid == uid);
    EXPECT_TRUE(st.st_gid == gid);

    uid = 01;
    gid = 00;
    ret = ChOwn(PATH_CHOWN, uid, gid);
    ASSERT_TRUE(ret == E_OK);

    ret = lstat(PATH_CHOWN.c_str(), &st);
    ASSERT_TRUE(ret == 0);
    EXPECT_TRUE(st.st_uid == uid);
    EXPECT_TRUE(st.st_gid == gid);

    GTEST_LOG_(INFO) << "FileUtilsTest_ChOwn_001 end";
}

/**
 * @tc.name: FileUtilsTest_MkDir_001
 * @tc.desc: Verify the MkDir function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_MkDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_MkDir_001 start";

    mode_t mode = 0771;
    int32_t ret = MkDir(PATH_MKDIR.c_str(), mode);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(StorageTest::StorageTestUtils::CheckDir(PATH_MKDIR)) << "check the dir";

    struct stat st;
    ret = lstat(PATH_MKDIR.c_str(), &st);
    ASSERT_TRUE(ret == 0);
    EXPECT_TRUE((st.st_mode & ALL_PERMS) == mode);

    GTEST_LOG_(INFO) << "FileUtilsTest_MkDir_001 end";
}

/**
 * @tc.name: FileUtilsTest_RmDir_001
 * @tc.desc: Verify the RmDir function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_RmDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_RmDir_001 start";

    mode_t mode = 0771;
    bool bRet = StorageTest::StorageTestUtils::MkDir(PATH_RMDIR, mode);
    ASSERT_TRUE(bRet);
    ASSERT_TRUE(StorageTest::StorageTestUtils::CheckDir(PATH_RMDIR));

    int32_t ret = RmDir(PATH_RMDIR);
    ASSERT_TRUE(ret == E_OK);
    EXPECT_TRUE(false == StorageTest::StorageTestUtils::CheckDir(PATH_RMDIR));

    GTEST_LOG_(INFO) << "FileUtilsTest_RmDir_001 end";
}

/**
 * @tc.name: FileUtilsTest_PrepareDir_001
 * @tc.desc: Verify the PrepareDir function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_PrepareDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDir_001 start";

    mode_t mode = 0771;
    uid_t uid = 00;
    gid_t gid = 01;
    int fd = open(PATH_MKDIR.c_str(), O_RDWR | O_CREAT, mode);
    ASSERT_TRUE(fd > 0);

    bool ret = PrepareDir(PATH_MKDIR, mode, uid, gid);
    ASSERT_TRUE(ret != true) << "path is not a dir";

    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDir_001 end";
}

/**
 * @tc.name: FileUtilsTest_PrepareDir_002
 * @tc.desc: Verify the PrepareDir function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_PrepareDir_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDir_002 start";

    mode_t mode = 0664;
    StorageTest::StorageTestUtils::MkDir(PATH_MKDIR, mode);

    mode = 0771;
    uid_t uid = 00;
    gid_t gid = 01;
    bool bRet = PrepareDir(PATH_MKDIR, mode, uid, gid);
    ASSERT_TRUE(bRet) << "check the dir is exists but mode is incorrect";

    struct stat st;
    int ret = lstat(PATH_MKDIR.c_str(), &st);
    ASSERT_TRUE(ret == 0);
    EXPECT_TRUE((st.st_mode & ALL_PERMS) == mode);
    EXPECT_TRUE(st.st_uid == uid);
    EXPECT_TRUE(st.st_gid == gid);

    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDir_002 end";
}

/**
 * @tc.name: FileUtilsTest_PrepareDir_003
 * @tc.desc: Verify the PrepareDir function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_PrepareDir_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDir_003 start";

    mode_t mode = 0771;
    uid_t uid = 00;
    gid_t gid = 01;
    bool bRet = PrepareDir(PATH_MKDIR, mode, uid, gid);
    ASSERT_TRUE(bRet);

    struct stat st;
    int ret = lstat(PATH_MKDIR.c_str(), &st);
    ASSERT_TRUE(ret == 0);
    EXPECT_TRUE((st.st_mode & ALL_PERMS) == mode);
    EXPECT_TRUE(st.st_uid == uid);
    EXPECT_TRUE(st.st_gid == gid);

    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDir_003 end";
}
} // STORAGE_DAEMON
} // OHOS
