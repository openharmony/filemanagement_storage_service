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

#include <climits>
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#include "disk_manager/volume/ivolume_operator.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class TestOperator : public IVolumeOperator {
public:
    MOCK_METHOD3(DoMount, int32_t(const std::string& devPath,
                                   const std::string& mountPath,
                                   unsigned long mountFlags));
};

class ExtIVolumeOperatorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::string testDir_;
    std::shared_ptr<TestOperator> op_;
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
}

void ExtIVolumeOperatorTest::TearDown(void)
{
    op_ = nullptr;
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

HWTEST_F(ExtIVolumeOperatorTest, Mount_EmptyMountPath, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/sda1", "", 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_MountPathTraversal, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/sda1", "/data/../etc/passwd", 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_InvalidPrefix, TestSize.Level1)
{
    int32_t ret = op_->Mount("/dev/block/sda1", "/data/local/tmp/fake_mnt", 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_RealpathFailed, TestSize.Level1)
{
    std::string path = testDir_ + "/nonexistent_mount_path";
    int32_t ret = op_->Mount("/dev/block/sda1", path, 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_DoMountFailed_CleanupMountPath, TestSize.Level1)
{
    std::string path = testDir_ + "/mount_fail_test";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    EXPECT_CALL(*op_, DoMount(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = op_->Mount("/dev/block/sda1", path, 0);
    EXPECT_EQ(ret, E_ERR);
    EXPECT_NE(access(path.c_str(), F_OK), 0);
}

HWTEST_F(ExtIVolumeOperatorTest, Mount_Success, TestSize.Level1)
{
    std::string path = testDir_ + "/mount_ok_test";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    EXPECT_CALL(*op_, DoMount(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = op_->Mount("/dev/block/sda1", path, 0);
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

} // namespace StorageDaemon
} // namespace OHOS
