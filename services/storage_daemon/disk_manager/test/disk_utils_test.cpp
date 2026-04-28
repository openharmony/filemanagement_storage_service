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

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include "disk_manager/disk/disk_utils.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class ExtDiskUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::string testDevPath_;
};

void ExtDiskUtilsTest::SetUpTestCase(void)
{
    testDevPath_ = "/dev/block/disk_utils_test_" + std::to_string(getpid());
    int fd = creat(testDevPath_.c_str(), 0600);
    if (fd >= 0) {
        close(fd);
    }
}

void ExtDiskUtilsTest::TearDownTestCase(void)
{
    unlink(testDevPath_.c_str());
}

void ExtDiskUtilsTest::SetUp(void)
{
    diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
    DiskUtilMoc::diskUtilMoc = diskUtilMoc_;

    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
}

void ExtDiskUtilsTest::TearDown(void)
{
    DiskUtilMoc::diskUtilMoc = nullptr;
    diskUtilMoc_ = nullptr;

    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_EmptyPath, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("", 0600, 8, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_PathTooLong, TestSize.Level1)
{
    std::string longPath = "/dev/block/" + std::string(4100, 'a');
    int32_t ret = DiskUtils::CreateBlockDeviceNode(longPath, 0600, 8, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_PathTraversal, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/../sda1", 0600, 8, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_InvalidPrefix, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/sda1", 0600, 8, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MajorNegative, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/sda1", 0600, -1, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MajorTooLarge, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/sda1", 0600, 4096, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MinorNegative, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/sda1", 0600, 8, -1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MinorTooLarge, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/sda1", 0600, 8, 1048576);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DestroyBlockDeviceNode_EmptyPath, TestSize.Level1)
{
    int32_t ret = DiskUtils::DestroyBlockDeviceNode("");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DestroyBlockDeviceNode_PathTraversal, TestSize.Level1)
{
    int32_t ret = DiskUtils::DestroyBlockDeviceNode("/dev/block/../sda1");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DestroyBlockDeviceNode_InvalidPrefix, TestSize.Level1)
{
    int32_t ret = DiskUtils::DestroyBlockDeviceNode("/dev/sda1");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, ReadPartitionTable_EmptyPath, TestSize.Level1)
{
    std::string output;
    int32_t maxVolume = 0;
    int32_t ret = DiskUtils::ReadPartitionTable("", output, maxVolume);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, ReadPartitionTable_PathTraversal, TestSize.Level1)
{
    std::string output;
    int32_t maxVolume = 0;
    int32_t ret = DiskUtils::ReadPartitionTable("/dev/block/../sda", output, maxVolume);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, ReadPartitionTable_ForkExecFailed, TestSize.Level1)
{
    std::string output;
    int32_t maxVolume = 0;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::ReadPartitionTable("/dev/block/sda", output, maxVolume);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtDiskUtilsTest, Partition_EmptyPath, TestSize.Level1)
{
    int32_t ret = DiskUtils::Partition("", 0, 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, Partition_PathTraversal, TestSize.Level1)
{
    int32_t ret = DiskUtils::Partition("/dev/block/../sda", 0, 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, Partition_ZapFailed, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::Partition("/dev/block/sda", 0, 0);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtDiskUtilsTest, Partition_PartFailed, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::Partition("/dev/block/sda", 0, 0);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtDiskUtilsTest, Partition_Success, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .Times(2)
        .WillRepeatedly(Return(E_OK));
    int32_t ret = DiskUtils::Partition("/dev/block/sda", 0, 0);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, ReadPartitionTable_Success, TestSize.Level1)
{
    std::string output;
    int32_t maxVolume = 0;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("PART_ENTRY_NUMBER=1");
            lines->push_back("PART_ENTRY_TYPE=0c00");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ReadPartitionTable("/dev/block/ut_nonexistent_disk", output, maxVolume);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(output, "PART_ENTRY_NUMBER=1\nPART_ENTRY_TYPE=0c00\n");
    EXPECT_EQ(maxVolume, MAX_SCSI_VOLUMES);
}

HWTEST_F(ExtDiskUtilsTest, Partition_SuccessWithOutput, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("zap line 1");
            return E_OK;
        }))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("part line 1");
            return E_OK;
        }));
    int32_t ret = DiskUtils::Partition("/dev/block/sda", 0, 0);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_NotBlockDevice, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode(testDevPath_, 0600, 8, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_ParentDirResolved, TestSize.Level1)
{
    std::string path = "/dev/block/nonexistent_test_node";
    int32_t ret = DiskUtils::CreateBlockDeviceNode(path, 0600, 8, 1);
    EXPECT_EQ(ret, E_OK);
    unlink(path.c_str());
}

} // namespace StorageDaemon
} // namespace OHOS
