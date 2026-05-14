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
#include <sys/stat.h>
#include <unistd.h>

#include "disk_manager/volume/volume_utils.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class ExtVolumeUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::string testDevPath_;
};

void ExtVolumeUtilsTest::SetUpTestCase(void)
{
    testDevPath_ = "/dev/block/vol_utils_test_" + std::to_string(getpid());
    int fd = creat(testDevPath_.c_str(), 0600);
    if (fd >= 0) {
        close(fd);
    }
}

void ExtVolumeUtilsTest::TearDownTestCase(void)
{
    unlink(testDevPath_.c_str());
}

void ExtVolumeUtilsTest::SetUp(void)
{
    diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
    DiskUtilMoc::diskUtilMoc = diskUtilMoc_;
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
}

void ExtVolumeUtilsTest::TearDown(void)
{
    DiskUtilMoc::diskUtilMoc = nullptr;
    diskUtilMoc_ = nullptr;
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_EmptyDevPath, TestSize.Level1)
{
    std::string uuid, type, label;
    int32_t ret = VolumeUtils::ReadMetadata("", uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_PathTooLong, TestSize.Level1)
{
    std::string uuid, type, label;
    std::string longPath(PATH_MAX, 'a');
    int32_t ret = VolumeUtils::ReadMetadata(longPath, uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_PathTraversal, TestSize.Level1)
{
    std::string uuid, type, label;
    int32_t ret = VolumeUtils::ReadMetadata("/dev/block/../sda1", uuid, type, label);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_TypeEmpty, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return(""));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_READMETADATA);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_InvalidUuid, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("vfat"))
        .WillOnce(Return(""))
        .WillOnce(Return("vfat"))
        .WillOnce(Return("testlabel"));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_READMETADATA);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsLabelWithQuestionMark, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("?????"));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(type, "ntfs");
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsLabelEmpty, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return(""));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(type, "ntfs");
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsLabelNormal, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("MyNTFSVolume"));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(type, "ntfs");
    EXPECT_EQ(label, "MyNTFSVolume");
}

HWTEST_F(ExtVolumeUtilsTest, MountFuseDevice_EmptyPath, TestSize.Level1)
{
    int fuseFd = -1;
    int32_t ret = VolumeUtils::MountFuseDevice("", fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, MountFuseDevice_PathTooLong, TestSize.Level1)
{
    int fuseFd = -1;
    std::string longPath(PATH_MAX, 'a');
    int32_t ret = VolumeUtils::MountFuseDevice(longPath, fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, MountFuseDevice_PathTraversal, TestSize.Level1)
{
    int fuseFd = -1;
    int32_t ret = VolumeUtils::MountFuseDevice("/data/../tmp/fuse", fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, MountFuseDevice_RealpathFailed, TestSize.Level1)
{
    int fuseFd = -1;
    int32_t ret = VolumeUtils::MountFuseDevice("/nonexistent_dir/some_path", fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, MountFuseDevice_InvalidParentPrefix, TestSize.Level1)
{
    int fuseFd = -1;
    int32_t ret = VolumeUtils::MountFuseDevice("/tmp/evil_mount/sub", fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, MountFuseDevice_ParentPathTraversal, TestSize.Level1)
{
    int fuseFd = -1;
    int32_t ret = VolumeUtils::MountFuseDevice("/mnt/data/external/../evil/sub", fuseFd);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsFallback_ForkExecFailed, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("?????"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_ERR));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(label, "");
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsFallback_LabelFound, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("?????"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("ntfslabel v1.0");
            output->push_back("Volume label: MyUSBDrive");
            return E_OK;
        }));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(label, "MyUSBDrive");
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsFallback_LabelNotFound, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return(""));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("ntfslabel v1.0");
            output->push_back("some other output line");
            return E_OK;
        }));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(label, "");
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_UuidContainsSlash, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("vfat"))
        .WillOnce(Return("abc/def"))
        .WillOnce(Return("vfat"))
        .WillOnce(Return("testlabel"));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_READMETADATA);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_UuidTooLong, TestSize.Level1)
{
    std::string uuid, type, label;
    std::string longUuid(41, 'a');
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("vfat"))
        .WillOnce(Return(longUuid))
        .WillOnce(Return("vfat"))
        .WillOnce(Return("testlabel"));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_READMETADATA);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_UuidIsDot, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("vfat"))
        .WillOnce(Return("."))
        .WillOnce(Return("vfat"))
        .WillOnce(Return("testlabel"));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_READMETADATA);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_UuidIsDotDot, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("vfat"))
        .WillOnce(Return(".."))
        .WillOnce(Return("vfat"))
        .WillOnce(Return("testlabel"));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_READMETADATA);
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsFallback_NoColonInOutput, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("?????"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Volume label information unavailable");
            return E_OK;
        }));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(label, "");
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsFallback_WhitespaceOnlyLabel, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("?????"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Volume label:   ");
            return E_OK;
        }));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(label, "   ");
}

HWTEST_F(ExtVolumeUtilsTest, ReadMetadata_NtfsFallback_EmptyAfterColon, TestSize.Level1)
{
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("ntfs-uuid-1234"))
        .WillOnce(Return("ntfs"))
        .WillOnce(Return("?????"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Volume label:");
            return E_OK;
        }));
    int32_t ret = VolumeUtils::ReadMetadata(testDevPath_, uuid, type, label);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(label, "");
}

} // namespace StorageDaemon
} // namespace OHOS
