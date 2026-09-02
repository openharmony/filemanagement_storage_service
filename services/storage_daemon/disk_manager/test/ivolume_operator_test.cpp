/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <chrono>
#include <climits>
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "disk_manager/volume/ivolume_operator.h"
#include "mock/file_utils_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class TestOperator : public IVolumeOperator {
public:
    MOCK_METHOD4(DoMount, int32_t(const std::string& devPath,
                                   const std::string& mountPath,
                                   unsigned long mountFlags,
                                   const std::string& mountData));
};

class ExtIVolumeOperatorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::string testDir_;
    std::shared_ptr<TestOperator> op_;
    std::shared_ptr<FileUtilMoc> fileUtilMoc_;
};

void ExtIVolumeOperatorTest::SetUpTestCase(void)
{
    testDir_ = "/mnt/data/external/ext_vol_op_test_" + std::to_string(getpid());
    mkdir(testDir_.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
}

void ExtIVolumeOperatorTest::TearDownTestCase(void)
{
    rmdir(testDir_.c_str());
}

void ExtIVolumeOperatorTest::SetUp(void)
{
    op_ = std::make_shared<TestOperator>();
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    IFileUtilMoc::fileUtilMoc = fileUtilMoc_;
    ON_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).WillByDefault(Return(false));
}

void ExtIVolumeOperatorTest::TearDown(void)
{
    op_ = nullptr;
    fileUtilMoc_ = nullptr;
    IFileUtilMoc::fileUtilMoc = nullptr;
}

HWTEST_F(ExtIVolumeOperatorTest, EnsureMountPath_EmptyPath, TestSize.Level1)
{
    std::string empty;
    TestOperator dummy;
    int32_t ret = dummy.EnsureMountPath(empty);
    EXPECT_EQ(ret, E_MKDIR_MOUNT);
}

HWTEST_F(ExtIVolumeOperatorTest, EnsureMountPath_PathTooLong, TestSize.Level1)
{
    TestOperator dummy;
    std::string longPath(PATH_MAX, 'a');
    int32_t ret = dummy.EnsureMountPath(longPath);
    EXPECT_EQ(ret, E_MKDIR_MOUNT);
}

HWTEST_F(ExtIVolumeOperatorTest, EnsureMountPath_PathTraversal, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/nonexistent_parent/child";
    int32_t ret = dummy.EnsureMountPath(path);
    EXPECT_EQ(ret, E_MKDIR_MOUNT);
}

HWTEST_F(ExtIVolumeOperatorTest, EnsureMountPath_Success, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/mnt_test";
    int32_t ret = dummy.EnsureMountPath(path);
    EXPECT_EQ(ret, E_OK);
    struct stat st;
    EXPECT_EQ(stat(path.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, RemoveMountPath_EmptyPath, TestSize.Level1)
{
    TestOperator dummy;
    int32_t ret = dummy.RemoveMountPath("");
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtIVolumeOperatorTest, RemoveMountPath_PathTooLong, TestSize.Level1)
{
    TestOperator dummy;
    std::string longPath(PATH_MAX, 'a');
    int32_t ret = dummy.RemoveMountPath(longPath);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtIVolumeOperatorTest, RemoveMountPath_PathTraversal, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/traversal_file";
    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    close(fd);
    int32_t ret = dummy.RemoveMountPath(path);
    EXPECT_EQ(ret, E_ERR);
    remove(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, RemoveMountPath_Success, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/rmdir_test";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    int32_t ret = dummy.RemoveMountPath(path);
    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(access(path.c_str(), F_OK), 0);
}

HWTEST_F(ExtIVolumeOperatorTest, RemoveMountPath_NotExist, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/non_exist_dir";
    int32_t ret = dummy.RemoveMountPath(path);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_InvalidDevPrefix, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/sda1", testDir_ + "/invalid_dev_prefix", 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_DevMapperPrefix_Success, TestSize.Level1)
{
    std::string path = testDir_ + "/mapper_mount_ok";
    EXPECT_CALL(*op_, DoMount(_, _, _, _)).WillOnce(Return(E_OK));
    int32_t ret = op_->Mount("/dev/mapper/mock_dev", path, 0);
    EXPECT_EQ(ret, E_OK);
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_EmptyMountPath, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/mock_dev", "", 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountPathTraversal, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/mock_dev", "/data/../etc/passwd", 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_InvalidPrefix, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/mock_dev", "/data/local/tmp/fake_mnt", 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountPathNotExist_DoMountFail, TestSize.Level1)
{
    std::string path = testDir_ + "/nonexistent_mount_path";
    EXPECT_CALL(*op_, DoMount(_, _, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0);
    EXPECT_EQ(ret, E_ERR);
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_DoMountFailed_CleanupMountPath, TestSize.Level1)
{
    std::string path = testDir_ + "/mount_fail_test";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    EXPECT_CALL(*op_, DoMount(_, _, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0);
    EXPECT_EQ(ret, E_ERR);
    EXPECT_NE(access(path.c_str(), F_OK), 0);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_Success, TestSize.Level1)
{
    std::string path = testDir_ + "/mount_ok_test";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    EXPECT_CALL(*op_, DoMount(_, _, _, _)).WillOnce(Return(E_OK));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0);
    EXPECT_EQ(ret, E_OK);
    struct stat st;
    EXPECT_EQ(stat(path.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Unmount_EmptyPath, TestSize.Level1)
{
    int32_t ret = op_->Unmount("", "", false);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Unmount_PathTraversal, TestSize.Level1)
{
    int32_t ret = op_->Unmount("/data/../etc", "", false);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Unmount_InvalidPrefix, TestSize.Level1)
{
    int32_t ret = op_->Unmount("/data/local/tmp/fake", "", false);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Unmount_RealpathFailed, TestSize.Level1)
{
    std::string path = testDir_ + "/nonexistent_unmount_path";
    int32_t ret = op_->Unmount(path, "", false);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Unmount_NotMountedPath, TestSize.Level1)
{
    std::string path = testDir_ + "/not_mounted";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    int32_t ret = op_->Unmount(path, "", false);
    EXPECT_EQ(ret, E_VOL_UMOUNT_ERR);
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, EnsureMountPath_FileExists_RemoveAndMkdir, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/file_to_remove";
    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    close(fd);
    struct stat st;
    ASSERT_EQ(stat(path.c_str(), &st), 0);
    ASSERT_FALSE(S_ISDIR(st.st_mode));

    int32_t ret = dummy.EnsureMountPath(path);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(stat(path.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, EnsureMountPath_DirAlreadyExists, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/dir_already_exists";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);

    int32_t ret = dummy.EnsureMountPath(path);
    EXPECT_EQ(ret, E_OK);
    struct stat st;
    EXPECT_EQ(stat(path.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, RemoveMountPath_NonEmptyDir, TestSize.Level1)
{
    TestOperator dummy;
    std::string path = testDir_ + "/nonempty_dir";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    std::string innerFile = path + "/inner";
    int fd = open(innerFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    close(fd);

    int32_t ret = dummy.RemoveMountPath(path);
    EXPECT_EQ(ret, E_ERR);

    remove(innerFile.c_str());
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Unmount_Success, TestSize.Level1)
{
    TestOperator dummy;
    std::string mntPath = testDir_ + "/unmount_mnt";
    mkdir(mntPath.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    int mret = mount("tmpfs", mntPath.c_str(), "tmpfs", 0, "size=1M");
    if (mret != 0) {
        rmdir(mntPath.c_str());
        GTEST_SKIP() << "mount tmpfs failed, skipping Unmount success test";
    }
    int32_t ret = dummy.Unmount(mntPath, "", false);
    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(access(mntPath.c_str(), F_OK), 0);
}

HWTEST_F(ExtIVolumeOperatorTest, Unmount_ForceSuccess, TestSize.Level1)
{
    TestOperator dummy;
    std::string mntPath = testDir_ + "/unmount_force_mnt";
    mkdir(mntPath.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    int mret = mount("tmpfs", mntPath.c_str(), "tmpfs", 0, "size=1M");
    if (mret != 0) {
        rmdir(mntPath.c_str());
        GTEST_SKIP() << "mount tmpfs failed, skipping Unmount force test";
    }
    int32_t ret = dummy.Unmount(mntPath, "", true);
    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(access(mntPath.c_str(), F_OK), 0);
}

HWTEST_F(ExtIVolumeOperatorTest, ReadMetadata_EmptyDevPath, TestSize.Level1)
{
    TestOperator dummy;
    std::string uuid, type, label;
    int32_t ret = dummy.ReadMetadata("", uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, ReadMetadata_PathTooLong, TestSize.Level1)
{
    TestOperator dummy;
    std::string longPath(PATH_MAX, 'a');
    std::string uuid, type, label;
    int32_t ret = dummy.ReadMetadata(longPath, uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, ReadMetadata_PathTraversal, TestSize.Level1)
{
    TestOperator dummy;
    std::string uuid, type, label;
    int32_t ret = dummy.ReadMetadata("/dev/../etc/passwd", uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, ReadMetadata_RealpathFailed, TestSize.Level1)
{
    TestOperator dummy;
    std::string uuid, type, label;
    int32_t ret = dummy.ReadMetadata("/dev/block/nonexistent_device", uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, ReadMetadata_InvalidDevPrefix, TestSize.Level1)
{
    TestOperator dummy;
    std::string uuid, type, label;
    std::string fakeDev = testDir_ + "/fake_dev_node";
    int fd = open(fakeDev.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);
    ASSERT_GE(fd, 0);
    close(fd);
    int32_t ret = dummy.ReadMetadata(fakeDev, uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    unlink(fakeDev.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_AsyncDoMountSuccess, TestSize.Level1)
{
    std::string path = testDir_ + "/async_mount_ok";
    EXPECT_CALL(*op_, DoMount(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::string &, unsigned long,
                            const std::string &) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return E_OK;
        }));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0);
    EXPECT_EQ(ret, E_OK);
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_AsyncDoMountFail_CleanupMountPath, TestSize.Level1)
{
    std::string path = testDir_ + "/async_mount_fail";
    EXPECT_CALL(*op_, DoMount(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::string &, unsigned long,
                            const std::string &) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return E_ERR;
        }));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0);
    EXPECT_EQ(ret, E_ERR);
    EXPECT_NE(access(path.c_str(), F_OK), 0);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_AsyncDoMountImmediateReturn, TestSize.Level1)
{
    std::string path = testDir_ + "/async_mount_immediate";
    EXPECT_CALL(*op_, DoMount(_, _, _, _)).WillOnce(Return(E_OK));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0);
    EXPECT_EQ(ret, E_OK);
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountDataContainsUid0, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/mock_dev", testDir_ + "/uid0_test", 0, "uid=0,other");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountDataContainsGid0, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/mock_dev", testDir_ + "/gid0_test", 0, "gid=0,other");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountDataContainsSuid, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/mock_dev", testDir_ + "/suid_test", 0, "suid,other");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountDataValid, TestSize.Level1)
{
    std::string path = testDir_ + "/valid_data_test";
    EXPECT_CALL(*op_, DoMount(_, _, _, _)).WillOnce(Return(E_OK));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0, "uid=1006,gid=1006");
    EXPECT_EQ(ret, E_OK);
    rmdir(path.c_str());
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountDataEmptyValid, TestSize.Level1)
{
    std::string path = testDir_ + "/empty_data_test";
    EXPECT_CALL(*op_, DoMount(_, _, _, _)).WillOnce(Return(E_OK));
    int32_t ret = op_->Mount("/dev/block/mock_dev", path, 0, "");
    EXPECT_EQ(ret, E_OK);
    rmdir(path.c_str());
}

/**
 * @tc.name: Unmount_VoldataPath_InUseFailed
 * @tc.desc: Verify Unmount returns E_VOL_UMOUNT_ERR when IsUsbInUse fails on /mnt/data/voldata/ path.
 * @tc.type: FUNC
 */
HWTEST_F(ExtIVolumeOperatorTest, Unmount_VoldataPath_InUseFailed, TestSize.Level1)
{
    TestOperator dummy;
    std::string voldataBase = "/mnt/data/voldata";
    std::string mntPath = voldataBase + "/ut_voldata_test_" + std::to_string(getpid());
    mkdir(voldataBase.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    mkdir(mntPath.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    int mret = mount("tmpfs", mntPath.c_str(), "tmpfs", 0, "size=1M");
    if (mret != 0) {
        rmdir(mntPath.c_str());
        rmdir(voldataBase.c_str());
        GTEST_SKIP() << "mount tmpfs on voldata path failed, skipping test";
    }
    // Open fd to hold the mount busy
    int holdFd = open(mntPath.c_str(), O_RDONLY);
    ASSERT_GE(holdFd, 0) << "Failed to open voldata mount path";
    // IsUsbInUse will return E_USB_IN_USE because of occupation,
    // and the path starts with /mnt/data/voldata/, so Unmount should return E_VOL_UMOUNT_ERR
    int32_t ret = dummy.Unmount(mntPath, "", false);
    EXPECT_EQ(ret, E_VOL_UMOUNT_ERR);
    close(holdFd);
    // Clean up: force unmount
    umount2(mntPath.c_str(), MNT_DETACH);
    rmdir(mntPath.c_str());
    rmdir(voldataBase.c_str());
}
 
/**
 * @tc.name: Unmount_NonVoldataPath_UInUseFailed_NoExtraError
 * @tc.desc: Verify Unmount does NOT return E_VOL_UMOUNT_ERR for non-voldata path when IsUsbInUse fails.
 *           The original behavior (success or E_VOL_UMOUNT_ERR from umount2) should be preserved.
 * @tc.type: FUNC
 */
HWTEST_F(ExtIVolumeOperatorTest, Unmount_NonVoldataPath_InUseFailed_NoExtraError, TestSize.Level1)
{
    TestOperator dummy;
    std::string mntPath = testDir_ + "/non_voldata_unmount";
    mkdir(mntPath.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    int mret = mount("tmpfs", mntPath.c_str(), "tmpfs", 0, "size=1M");
    if (mret != 0) {
        rmdir(mntPath.c_str());
        GTEST_SKIP() << "mount tmpfs failed, skipping test";
    }
    // Path is /mnt/data/external/..., not /mnt/data/voldata/,
    // so even if IsUsbInUse fails, the voldata check should not trigger.
    // Unmount should succeed based on umount2 result.
    int32_t ret = dummy.Unmount(mntPath, "", false);
    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(access(mntPath.c_str(), F_OK), 0);
}

} // namespace StorageDaemon
} // namespace OHOS
