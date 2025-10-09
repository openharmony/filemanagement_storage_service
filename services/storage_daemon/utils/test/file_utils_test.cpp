/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <fstream>
#include <filesystem>

#include "gtest/gtest.h"
#include "common/help_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "parameter.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

constexpr int NOT_EXIST_FD_1 = 45678;
constexpr int NOT_EXIST_FD_2 = 45679;
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
    EXPECT_TRUE(StorageTest::StorageTestUtils::CheckDir(PATH_RMDIR) == false);

    GTEST_LOG_(INFO) << "FileUtilsTest_RmDir_001 end";
}

/**
 * @tc.name: FileUtilsTest_PrepareDirSimple_001
 * @tc.desc: Verify the PrepareDirSimple function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_PrepareDirSimple_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDirSimple_001 start";

    std::string path = "/data/testDir";
    mode_t mode = 0771;
    uid_t uid = 0;
    gid_t gid = 0;

    int32_t ret = PrepareDirSimple(path + "/testDir", mode, uid, gid);
    EXPECT_NE(ret, E_OK);

    ret = PrepareDirSimple(path, mode, uid, gid);
    EXPECT_EQ(ret, E_OK);
    bool isPathEmpty = true;
    DestroyDir(path, isPathEmpty);

    GTEST_LOG_(INFO) << "FileUtilsTest_PrepareDirSimple_001 end";
}

/**
 * @tc.name: FileUtilsTest_DestroyDir_001
 * @tc.desc: Verify the DestroyDir function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_DestroyDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_DestroyDir_001 start";
    bool isPathEmpty = true;
    std::string path = "/data/testDir";
    mode_t mode = 0771;

    int32_t ret = DestroyDir(path + "/testDir", isPathEmpty);
    EXPECT_EQ(ret, E_DELETE_USER_DIR_NOEXIST);

    std::ofstream file("/data/testFile.txt");
    file.close();
    
    ret = DestroyDir("/data/testFile.txt", isPathEmpty);
    EXPECT_EQ(ret, E_OPENDIR_ERROR);
    std::filesystem::remove("/data/testFile.txt");

    PrepareDirSimple(path, mode, 0, 0);
    PrepareDirSimple(path + "/testDir", mode, 0, 0);
    std::ofstream file2(path + "/testDir/testFile.txt");
    file2.close();
    ret = DestroyDir(path, isPathEmpty);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "FileUtilsTest_DestroyDir_001 end";
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
    (void)close(fd);
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

/**
 * @tc.name: FileUtilsTest_Split_001
 * @tc.desc: Verify the Split function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_Split_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_Split_001 start";

    std::string test = "this is a good idea";
    std::string pattern = " ";
    std::vector<std::string> expectVec{ "this", "is", "a", "good", "idea" };
    auto ret = Split(test, pattern);
    EXPECT_EQ(ret, expectVec);
    GTEST_LOG_(INFO) << "FileUtilsTest_Split_001 end";
}

/**
 * @tc.name: FileUtilsTest_DelFolder_001
 * @tc.desc: Verify the DelFolder function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_DelFolder_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_DelFolder_001 start";

    std::string testPath = "/data/test/tdd";
    EXPECT_TRUE(CreateFolder(testPath));
    EXPECT_TRUE(DelFolder(testPath));
    EXPECT_FALSE(DelFolder(testPath));
    GTEST_LOG_(INFO) << "FileUtilsTest_DelFolder_001 end";
}

/**
 * @tc.name: FileUtilsTest_IsFile_001
 * @tc.desc: Verify the IsFile function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_IsFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_IsFile_001 start";

    std::string testPath = "/data/test/tdd";
    EXPECT_TRUE(CreateFolder(testPath));
    std::string fileName = testPath + "/test.txt";
    auto fd = open(fileName.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);
    close(fd);
    EXPECT_TRUE(IsFile(fileName));
    EXPECT_TRUE(DeleteFile(fileName) == 0);
    EXPECT_FALSE(IsFile(fileName));
    EXPECT_TRUE(DelFolder(testPath));
    GTEST_LOG_(INFO) << "FileUtilsTest_IsFile_001 end";
}

/**
 * @tc.name: FileUtilsTest_TravelChmod_001
 * @tc.desc: Verify the IsFile function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_TravelChmod_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_TravelChmod_001 start";
    std::string basePath = "/data/test/tdd";
    std::string testPath = basePath + "/fold";
    EXPECT_TRUE(CreateFolder(testPath));
    std::string fileName = basePath + "/test.txt";
    auto fd = open(fileName.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);
    close(fd);

    mode_t mode = 0777;
    TravelChmod(basePath, mode);
    TravelChmod(fileName, mode);
    EXPECT_TRUE(DeleteFile(basePath) == 0);
    EXPECT_TRUE(DelFolder(basePath));
    TravelChmod(basePath, mode);
    GTEST_LOG_(INFO) << "FileUtilsTest_TravelChmod_001 end";
}

/**
 * @tc.name: FileUtilsTest_IsTempFolder_001
 * @tc.desc: Verify the IsTempFolder function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_IsTempFolder_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_IsTempFolder_001 start";
    std::string basePath = "/data/test/tdd/fold";
    std::string sub = "fold";
    std::string sub2 = "temp";
    EXPECT_TRUE(CreateFolder(basePath));
    EXPECT_TRUE(IsTempFolder(basePath, sub));
    EXPECT_FALSE(IsTempFolder(basePath, sub2));
    EXPECT_TRUE(DeleteFile(basePath) == 0);
    EXPECT_TRUE(DelFolder(basePath));
    GTEST_LOG_(INFO) << "FileUtilsTest_IsTempFolder_001 end";
}

/**
 * @tc.name: FileUtilsTest_DelTemp_001
 * @tc.desc: Verify the DelTemp function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_DelTemp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_DelTemp_001 start";
    std::string basePath = "/data/test/tdd";
    std::string subPath1 = basePath + "/fold";
    std::string subPath2 = basePath + "/simple-mtpfs";
    EXPECT_TRUE(CreateFolder(subPath1));
    EXPECT_TRUE(CreateFolder(subPath2));
    DelTemp(basePath);
    EXPECT_TRUE(IsDir(subPath1));
    EXPECT_FALSE(IsDir(subPath2));
    EXPECT_TRUE(DeleteFile(basePath) == 0);
    EXPECT_TRUE(DelFolder(basePath));
    GTEST_LOG_(INFO) << "FileUtilsTest_DelTemp_001 end";
}

/**
 * @tc.name: FileUtilsTest_KillProcess_001
 * @tc.desc: Verify the KillProcess function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_KillProcess_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_KillProcess_001 start";
    std::vector<ProcessInfo> processList;
    std::vector<ProcessInfo> killFailList;
    KillProcess(processList, killFailList);
    
    ProcessInfo info1 {.name = "test1", .pid = 65300 };
    ProcessInfo info2 {.name = "test2", .pid = 65301 };
    processList.push_back(info1);
    processList.push_back(info2);

    KillProcess(processList, killFailList);
    EXPECT_EQ(killFailList.size(), 0);
    GTEST_LOG_(INFO) << "FileUtilsTest_KillProcess_001 end";
}

/**
 * @tc.name: FileUtilsTest_ForkExecWithExit_001
 * @tc.desc: Verify the ForkExecWithExit function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ForkExecWithExit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExecWithExit_001 start";

    std::vector<std::string> cmd = {
        "fsck.ntfs",
        "/dev/block/vol-1-7",
    };

    EXPECT_EQ(ForkExecWithExit(cmd), E_WEXITSTATUS);
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExecWithExit_001 end";
}

/**
 * @tc.name: FileUtilsTest_ForkExecWithExit_002
 * @tc.desc: Verify the ForkExecWithExit function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ForkExecWithExit_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExecWithExit_002 start";

    std::vector<std::string> cmd = {
        "fsck.ntfs",
        "/dev/block/vol-1-8",
    };
    int res = 0;
    EXPECT_EQ(ForkExecWithExit(cmd, &res), E_WEXITSTATUS);
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExecWithExit_002 end";
}

/**
 * @tc.name: FileUtilsTest_ForkExecWithExit_003
 * @tc.desc: Verify the ForkExecWithExit function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ForkExecWithExit_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExecWithExit_003 start";

    std::vector<std::string> cmd = {
        "fsck.ntfs",
        "/dev/block/vol-1-9",
    };
    int res = 1;
    EXPECT_EQ(ForkExecWithExit(cmd, &res), E_WEXITSTATUS);
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExecWithExit_003 end";
}

/**
 * @tc.name: FileUtilsTest_IsFuse_001
 * @tc.desc: Verify the IsUsbFuse function basic functionality.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_IsFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_IsFuse_001 start";

    // Test the IsUsbFuse function basic functionality
    bool result = IsUsbFuse();
    
    EXPECT_FALSE(result);
    
    GTEST_LOG_(INFO) << "FileUtilsTest_IsFuse_001 end";
}

/**
 * @tc.name: FileUtilsTest_ForkExec_001
 * @tc.desc: Verify the ForkExec function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ForkExec_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExec_001 start";
    std::vector<std::string> cmd = {
            "ls",
            "/dev/block/",
    };
    std::vector<std::string> output;
    EXPECT_EQ(ForkExec(cmd, &output), E_OK);
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExec_001 end";
}

/**
 * @tc.name: FileUtilsTest_ForkExec_002
 * @tc.desc: Verify the ForkExec function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ForkExec_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExec_002 start";
    std::vector<std::string> cmd = {
            "ls",
            "/dev/block/",
    };
    std::vector<std::string> output;
    int res = 0;
    EXPECT_EQ(ForkExec(cmd, &output, &res), E_OK);
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExec_002 end";
}

/**
 * @tc.name: FileUtilsTest_ForkExec_003
 * @tc.desc: Verify the ForkExec function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_ForkExec_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExec_003 start";
    std::vector<std::string> cmd = {
            "ls",
            "/dev/block/",
    };
    std::vector<std::string> output;
    int res = 1;
    EXPECT_EQ(ForkExec(cmd, &output, &res), E_OK);
    GTEST_LOG_(INFO) << "FileUtilsTest_ForkExec_003 end";
}

/**
 * @tc.name: FileUtilsTest_RedirectStdToPipe_001
 * @tc.desc: Verify the RedirectStdToPipe function.
 * @tc.type: FUNC
 * @tc.require: IBDKKD
 */
HWTEST_F(FileUtilsTest, FileUtilsTest_RedirectStdToPipe_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileUtilsTest_RedirectStdToPipe_001 start";
    int logpipe[2] = {};
    size_t len = 0;
    int res = RedirectStdToPipe(logpipe, len);
    EXPECT_EQ(res, E_ERR);

    logpipe[0] = NOT_EXIST_FD_1;
    logpipe[1] = NOT_EXIST_FD_2;
    res = RedirectStdToPipe(logpipe, len);
    EXPECT_EQ(res, E_ERR);

    len = sizeof(logpipe);
    res = RedirectStdToPipe(logpipe, len);
    EXPECT_EQ(res, E_ERR);
    GTEST_LOG_(INFO) << "FileUtilsTest_RedirectStdToPipe_001 end";
}
} // STORAGE_DAEMON
} // OHOS
