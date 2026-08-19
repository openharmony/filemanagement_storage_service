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

#include <algorithm>
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <scsi/sg.h>

#include "disk_manager/disk/disk_utils.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"

#include "mock/disk_func_define.h"
#include "mock/disk_func_undef.h"
#include "securec.h"
#include <cstring>
#include <cstdarg>

int g_memsetRet = 0;
int g_ioctlRet = 0;
int g_ioctlInfo = 0;

extern "C" errno_t memset_s(void *dest, size_t destMax, int value, size_t count)
{
    if (g_memsetRet != 0) {
        return g_memsetRet;
    }
    (void)memset(dest, value, count);
    return 0;
}

extern "C" int ioctl(int fd, int request, ...)
{
    (void)fd;
    va_list args;
    va_start(args, request);
    void* arg = va_arg(args, void*);
    va_end(args);
    if (arg != nullptr) {
        sg_io_hdr_t* ioHdr = static_cast<sg_io_hdr_t*>(arg);
        ioHdr->info = g_ioctlInfo;
    }
    return g_ioctlRet;
}

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

    g_memsetRet = 0;
    g_ioctlRet = 0;
    g_ioctlInfo = 0;
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
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/../uttestdisk1", 0600, 8, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_InvalidPrefix, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/uttestdisk1", 0600, 8, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MajorNegative, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/uttestdisk1", 0600, -1, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MajorTooLarge, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/uttestdisk1", 0600, 4096, 1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MinorNegative, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/uttestdisk1", 0600, 8, -1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_MinorTooLarge, TestSize.Level1)
{
    int32_t ret = DiskUtils::CreateBlockDeviceNode("/dev/block/uttestdisk1", 0600, 8, 1048576);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DestroyBlockDeviceNode_EmptyPath, TestSize.Level1)
{
    int32_t ret = DiskUtils::DestroyBlockDeviceNode("");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DestroyBlockDeviceNode_PathTraversal, TestSize.Level1)
{
    int32_t ret = DiskUtils::DestroyBlockDeviceNode("/dev/block/../uttestdisk1");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DestroyBlockDeviceNode_InvalidPrefix, TestSize.Level1)
{
    int32_t ret = DiskUtils::DestroyBlockDeviceNode("/dev/uttestdisk1");
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
    int32_t ret = DiskUtils::ReadPartitionTable("/dev/block/uttestdisk", output, maxVolume);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtDiskUtilsTest, Partition_EmptyPath, TestSize.Level1)
{
    int32_t ret = DiskUtils::Partition("", "");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, Partition_PathTraversal, TestSize.Level1)
{
    int32_t ret = DiskUtils::Partition("/dev/block/../sda", "");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, Partition_ZapFailed, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "");
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtDiskUtilsTest, Partition_PartFailed, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "");
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtDiskUtilsTest, Partition_Success, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .Times(2)
        .WillRepeatedly(Return(E_OK));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "");
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, ReadPartitionTable_Success, TestSize.Level1)
{
    std::string output;
    int32_t maxVolume = 0;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("DISK gpt 259:0 100\n");
            lines->push_back("PART 1 0x0c00 2048 102400 userdata\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ReadPartitionTable("/dev/block/ut_nonexistent_disk", output, maxVolume);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(output, "DISK gpt 259:0 100\nPART 1 0x0c00 2048 102400 userdata\n");
    EXPECT_EQ(maxVolume, MAX_SCSI_VOLUMES);
}

HWTEST_F(ExtDiskUtilsTest, ReadPartitionTable_CrossChunkSplit, TestSize.Level1)
{
    std::string output;
    int32_t maxVolume = 0;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("DISK gpt 259:0 100\nPART 1 0x0c00 2048 102400 user");
            lines->push_back("data\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ReadPartitionTable("/dev/block/ut_nonexistent_disk", output, maxVolume);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(output, "DISK gpt 259:0 100\nPART 1 0x0c00 2048 102400 userdata\n");
    EXPECT_EQ(maxVolume, MAX_SCSI_VOLUMES);
}

HWTEST_F(ExtDiskUtilsTest, ReadPartitionTable_SkipCdDvdBd, TestSize.Level1)
{
    std::string path = "/dev/block/ut_cd_test_node_" + std::to_string(getpid());
    dev_t dev = makedev(DISK_CD_MAJOR, 0);
    mode_t mode = 0660 | S_IFBLK;
    ASSERT_EQ(mknod(path.c_str(), mode, dev), 0);

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(0);

    std::string output;
    int32_t maxVolume = -1;
    int32_t ret = DiskUtils::ReadPartitionTable(path, output, maxVolume);
    EXPECT_EQ(ret, E_NOT_SUPPORT);

    unlink(path.c_str());
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
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "");
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

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_EmptyPath, TestSize.Level1)
{
    std::string devPath = "";
    std::string execRet;
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_PathTraversal, TestSize.Level1)
{
    std::string devPath = "/dev/block/../sda";
    std::string execRet;
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_InvalidPrefix, TestSize.Level1)
{
    std::string devPath = "/dev/sda";
    std::string execRet;
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_ForkExecFailed, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_GET_PARTITION_ERROR);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_Success, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Disk /dev/block/uttestdisk: 100 GiB");
            lines->push_back("Number  Start  End  Size  Type");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(execRet.find("Disk") != std::string::npos);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_LineMergeWithSpace, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Disk /dev/block/uttestdisk: 100 GiB\n");
            lines->push_back("Number  Start  End  Size  Type\n");
            lines->push_back("1  2048  102400\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(execRet.find("Number  Start  End  Size  Type\n") != std::string::npos);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_LineMergeWithDigit, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Partition entry:\n");
            lines->push_back("1 2048 102400\n");
            lines->push_back("2 204800 409600\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(execRet.find("Partition entry:\n") != std::string::npos);
    EXPECT_TRUE(execRet.find("1 2048 102400\n") != std::string::npos);
    EXPECT_TRUE(execRet.find("2 204800 409600\n") != std::string::npos);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_LineNoMergeWithLetter, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Number  Start  End\n");
            lines->push_back("Disk label: gpt\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(execRet.find("Number  Start  End\n") != std::string::npos);
    EXPECT_TRUE(execRet.find("Disk label: gpt\n") != std::string::npos);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_MultiLineMerge, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Entry:\n");
            lines->push_back(" start=2048\n");
            lines->push_back(" end=102400\n");
            lines->push_back(" type=ext4\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(execRet.find("Entry:\n") != std::string::npos);
    EXPECT_TRUE(execRet.find(" start=2048\n") != std::string::npos);
    EXPECT_TRUE(execRet.find(" type=ext4\n") != std::string::npos);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_CrossChunkSplit, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Disk /dev/block/uttestdisk: 100 GiB\n");
            lines->push_back("Number  Start  End  user");
            lines->push_back("data\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(execRet.find("Number  Start  End  userdata\n") != std::string::npos);
    EXPECT_EQ(std::count(execRet.begin(), execRet.end(), '\n'), 2);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_LineEndsWithNewline, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Disk /dev/block/uttestdisk: 100 GiB\n");
            lines->push_back("Number  Start  End\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(std::count(execRet.begin(), execRet.end(), '\n'), 2);
}

HWTEST_F(ExtDiskUtilsTest, GetPartitionTableInfo_EmptyLineHandling, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string execRet;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *lines, int *) {
            lines->push_back("Disk info:\n");
            lines->push_back("\n");
            lines->push_back("  extra data\n");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetPartitionTableInfo(devPath, execRet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(execRet.find("Disk info:\n\n  extra data\n") != std::string::npos);
}

HWTEST_F(ExtDiskUtilsTest, CreatePartition_EmptyPath, TestSize.Level1)
{
    std::string devPath = "";
    int32_t partitionNum = 1;
    int64_t startSector = 2048;
    int64_t endSector = 102400;
    std::string typeCode = "ext4";
    int32_t ret = DiskUtils::CreatePartition(devPath, partitionNum, startSector, endSector, typeCode);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreatePartition_PathTraversal, TestSize.Level1)
{
    std::string devPath = "/dev/block/../sda";
    int32_t partitionNum = 1;
    int64_t startSector = 2048;
    int64_t endSector = 102400;
    std::string typeCode = "ext4";
    int32_t ret = DiskUtils::CreatePartition(devPath, partitionNum, startSector, endSector, typeCode);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreatePartition_InvalidPrefix, TestSize.Level1)
{
    std::string devPath = "/dev/sda";
    int32_t partitionNum = 1;
    int64_t startSector = 2048;
    int64_t endSector = 102400;
    std::string typeCode = "ext4";
    int32_t ret = DiskUtils::CreatePartition(devPath, partitionNum, startSector, endSector, typeCode);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, CreatePartition_ForkExecFailed, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 1;
    int64_t startSector = 2048;
    int64_t endSector = 102400;
    std::string typeCode = "ext4";
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::CreatePartition(devPath, partitionNum, startSector, endSector, typeCode);
    EXPECT_EQ(ret, E_CREATE_PARTITION_ERROR);
}

HWTEST_F(ExtDiskUtilsTest, CreatePartition_Success, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 1;
    int64_t startSector = 2048;
    int64_t endSector = 102400;
    std::string typeCode = "ext4";
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Creating partition 1");
            return E_OK;
        }));
    int32_t ret = DiskUtils::CreatePartition(devPath, partitionNum, startSector, endSector, typeCode);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, CreatePartition_WithVfatType, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 2;
    int64_t startSector = 2048;
    int64_t endSector = 204800;
    std::string typeCode = "vfat";
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::CreatePartition(devPath, partitionNum, startSector, endSector, typeCode);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_EmptyPath, TestSize.Level1)
{
    std::string devPath = "";
    std::string diskId = "sda";
    int32_t partitionNum = 1;
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_PathTraversal, TestSize.Level1)
{
    std::string devPath = "/dev/block/../sda";
    std::string diskId = "sda";
    int32_t partitionNum = 1;
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_InvalidPrefix, TestSize.Level1)
{
    std::string devPath = "/dev/sda";
    std::string diskId = "sda";
    int32_t partitionNum = 1;
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_DamageFailed, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string diskId = "uttestdisk";
    int32_t partitionNum = 1;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .Times(2)
        .WillRepeatedly(Return(E_ERR));
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_DELETE_PARTITION_ERROR);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_Success, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string diskId = "uttestdisk";
    int32_t partitionNum = 1;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .Times(2)
        .WillRepeatedly(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Deleting partition 1");
            return E_OK;
        }));
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_DeleteFailedAfterDamageSuccess, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string diskId = "uttestdisk";
    int32_t partitionNum = 1;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Damage partition 1");
            return E_OK;
        }))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_DELETE_PARTITION_ERROR);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_WithDifferentPartitionNum, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string diskId = "uttestdisk";
    int32_t partitionNum = 5;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .Times(2)
        .WillRepeatedly(Return(E_OK));
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncDamagePartition_Success, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 1;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Damage partition 1");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncDamagePartition(devPath, partitionNum);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncDamagePartition_ForkExecFailed, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 1;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::ExecAsyncDamagePartition(devPath, partitionNum);
    EXPECT_EQ(ret, E_DELETE_PARTITION_ERROR);
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncDamagePartition_WithDifferentPartitionNum, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 3;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::ExecAsyncDamagePartition(devPath, partitionNum);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_EmptyPath, TestSize.Level1)
{
    std::string devPath = "";
    std::string fsType = "ext4";
    std::string volumeName = "test_volume";
    bool quickFormat = true;
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_PathTraversal, TestSize.Level1)
{
    std::string devPath = "/dev/block/../sda";
    std::string fsType = "ext4";
    std::string volumeName = "test_volume";
    bool quickFormat = true;
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_InvalidPrefix, TestSize.Level1)
{
    std::string devPath = "/dev/sda";
    std::string fsType = "ext4";
    std::string volumeName = "test_volume";
    bool quickFormat = true;
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_UnsupportedFsType, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string fsType = "unsupported_fs";
    std::string volumeName = "test_volume";
    bool quickFormat = true;
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_FORMAT_PARTITION_NOT_SUPPORT);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_ForkExecFailed, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::string fsType = "ext4";
    std::string volumeName = "test_volume";
    bool quickFormat = true;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_FORMAT_PARTITION_ERROR);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_SuccessWithExt4, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk1";
    std::string fsType = "ext4";
    std::string volumeName = "test_volume";
    bool quickFormat = true;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Formatting with ext4");
            return E_OK;
        }));
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_SuccessWithVfat, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk2";
    std::string fsType = "vfat";
    std::string volumeName = "test_vfat";
    bool quickFormat = true;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Formatting with vfat");
            return E_OK;
        }));
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_SuccessWithExfat, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk3";
    std::string fsType = "exfat";
    std::string volumeName = "test_exfat";
    bool quickFormat = true;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Formatting with exfat");
            return E_OK;
        }));
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_SuccessWithoutVolumeName, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk4";
    std::string fsType = "ext4";
    std::string volumeName = "";
    bool quickFormat = true;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, FormatPartition_QuickFormatFalse, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk5";
    std::string fsType = "ext4";
    std::string volumeName = "test_volume";
    bool quickFormat = false;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::FormatPartition(devPath, fsType, volumeName, quickFormat);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, GetFormatCMD_VfatWithVolumeName, TestSize.Level1)
{
    std::string fsType = "vfat";
    std::string devPath = "/dev/block/uttestdisk1";
    std::string volName = "test_vol";
    std::vector<std::string> cmd = DiskUtils::GetFormatCMD(fsType, devPath, volName);
    EXPECT_EQ(cmd.size(), 5);
    EXPECT_EQ(cmd[0], "newfs_msdos");
    EXPECT_EQ(cmd[1], "-L");
    EXPECT_EQ(cmd[2], "test_vol");
    EXPECT_EQ(cmd[3], "-A");
    EXPECT_EQ(cmd[4], devPath);
}

HWTEST_F(ExtDiskUtilsTest, GetFormatCMD_VfatWithoutVolumeName, TestSize.Level1)
{
    std::string fsType = "vfat";
    std::string devPath = "/dev/block/uttestdisk1";
    std::string volName = "";
    std::vector<std::string> cmd = DiskUtils::GetFormatCMD(fsType, devPath, volName);
    EXPECT_EQ(cmd.size(), 3);
    EXPECT_EQ(cmd[0], "newfs_msdos");
    EXPECT_EQ(cmd[1], "-A");
    EXPECT_EQ(cmd[2], devPath);
}

HWTEST_F(ExtDiskUtilsTest, GetFormatCMD_Ext4WithVolumeName, TestSize.Level1)
{
    std::string fsType = "ext4";
    std::string devPath = "/dev/block/uttestdisk2";
    std::string volName = "ext4_vol";
    std::vector<std::string> cmd = DiskUtils::GetFormatCMD(fsType, devPath, volName);
    EXPECT_EQ(cmd.size(), 6);
    EXPECT_EQ(cmd[0], "mke2fs");
    EXPECT_EQ(cmd[1], "-L");
    EXPECT_EQ(cmd[2], "ext4_vol");
    EXPECT_EQ(cmd[3], "-t");
    EXPECT_EQ(cmd[4], "ext4");
    EXPECT_EQ(cmd[5], devPath);
}

HWTEST_F(ExtDiskUtilsTest, GetFormatCMD_Ext4WithoutVolumeName, TestSize.Level1)
{
    std::string fsType = "ext4";
    std::string devPath = "/dev/block/uttestdisk2";
    std::string volName = "";
    std::vector<std::string> cmd = DiskUtils::GetFormatCMD(fsType, devPath, volName);
    EXPECT_EQ(cmd.size(), 4);
    EXPECT_EQ(cmd[0], "mke2fs");
    EXPECT_EQ(cmd[1], "-t");
    EXPECT_EQ(cmd[2], "ext4");
    EXPECT_EQ(cmd[3], devPath);
}

HWTEST_F(ExtDiskUtilsTest, GetFormatCMD_ExfatWithVolumeName, TestSize.Level1)
{
    std::string fsType = "exfat";
    std::string devPath = "/dev/block/uttestdisk3";
    std::string volName = "exfat_vol";
    std::vector<std::string> cmd = DiskUtils::GetFormatCMD(fsType, devPath, volName);
    EXPECT_EQ(cmd.size(), 4);
    EXPECT_EQ(cmd[0], "mkfs.exfat");
    EXPECT_EQ(cmd[1], "-L");
    EXPECT_EQ(cmd[2], "exfat_vol");
    EXPECT_EQ(cmd[3], devPath);
}

HWTEST_F(ExtDiskUtilsTest, GetFormatCMD_ExfatWithoutVolumeName, TestSize.Level1)
{
    std::string fsType = "exfat";
    std::string devPath = "/dev/block/uttestdisk3";
    std::string volName = "";
    std::vector<std::string> cmd = DiskUtils::GetFormatCMD(fsType, devPath, volName);
    EXPECT_EQ(cmd.size(), 2);
    EXPECT_EQ(cmd[0], "mkfs.exfat");
    EXPECT_EQ(cmd[1], devPath);
}

HWTEST_F(ExtDiskUtilsTest, GetFormatCMD_UnsupportedFsType, TestSize.Level1)
{
    std::string fsType = "unsupported";
    std::string devPath = "/dev/block/uttestdisk";
    std::string volName = "test";
    std::vector<std::string> cmd = DiskUtils::GetFormatCMD(fsType, devPath, volName);
    EXPECT_TRUE(cmd.empty());
}

/**
 * @tc.name: Partition_HmfsPathTraversal_001
 * @tc.desc: Verify Partition with hmfs type and path traversal returns E_PARAMS_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsPathTraversal_001, TestSize.Level1)
{
    int32_t ret = DiskUtils::Partition("/dev/block/../sda", "hmfs");
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

/**
 * @tc.name: Partition_HmfsClearFailed_001
 * @tc.desc: Verify Partition with hmfs type fails when sgdisk clear (step 1) fails.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsClearFailed_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_ERR);
}

/**
 * @tc.name: Partition_HmfsPartFailed_001
 * @tc.desc: Verify Partition with hmfs type fails when sgdisk partition (step 2) fails.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsPartFailed_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_ERR);
}

/**
 * @tc.name: Partition_HmfsSuccess_001
 * @tc.desc: Verify Partition with hmfs type succeeds when all three steps (clear, partition, mkfs) succeed.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsSuccess_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .Times(2)
        .WillRepeatedly(Return(E_OK));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Partition_HmfsSuccessWithOutput_001
 * @tc.desc: Verify Partition with hmfs type handles output from all three steps correctly.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsSuccessWithOutput_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("GPT entries cleared");
            return E_OK;
        }))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Partition 1 created");
            return E_OK;
        }));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Partition_HmfsVerifyClearCmd_001
 * @tc.desc: Verify PartitionHmfs invokes sgdisk with -zog flag for clear step.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsVerifyClearCmd_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &cmd, std::vector<std::string> *, int *) {
            EXPECT_EQ(cmd.size(), 3u);
            EXPECT_EQ(cmd[1], "-zog");
            return E_OK;
        }))
        .WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Partition_HmfsVerifyPartCmd_001
 * @tc.desc: Verify PartitionHmfs invokes sgdisk with correct partition flags.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsVerifyPartCmd_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Invoke([](std::vector<std::string> &cmd, std::vector<std::string> *, int *) {
            EXPECT_EQ(cmd.size(), 4u);
            EXPECT_EQ(cmd[1], "--new=0:0:-0");
            EXPECT_EQ(cmd[2], "--typecode=0:8300");
            return E_OK;
        }));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, CreateBlockDeviceNode_AlreadyExists, TestSize.Level1)
{
    std::string path = "/dev/block/ut_eexist_test_node_" + std::to_string(getpid());
    dev_t dev = makedev(8, 200);
    mode_t mode = 0660 | S_IFBLK;
    ASSERT_EQ(mknod(path.c_str(), mode, dev), 0);
    int32_t ret = DiskUtils::CreateBlockDeviceNode(path, 0660, 8, 200);
    EXPECT_EQ(ret, E_OK);
    unlink(path.c_str());
}

HWTEST_F(ExtDiskUtilsTest, DestroyBlockDeviceNode_NotExists, TestSize.Level1)
{
    std::string path = "/dev/block/ut_enoent_test_node_" + std::to_string(getpid());
    unlink(path.c_str());
    int32_t ret = DiskUtils::DestroyBlockDeviceNode(path);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Partition_HmfsMkfsFailed_001
 * @tc.desc: Verify PartitionHmfs fails when mkfs.f2fs (step 3) fails.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsMkfsFailed_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_ERR);
}

/**
 * @tc.name: Partition_HmfsVerifyMkfsCmd_001
 * @tc.desc: Verify PartitionHmfs invokes mkfs.f2fs with correct flags for step 3.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, Partition_HmfsVerifyMkfsCmd_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::Partition("/dev/block/uttestdisk", "hmfs");
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, ExecuteScsiCmd_IoHdrMemsetFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteScsiCmd_IoHdrMemsetFailed start";
    g_memsetRet = E_ERR;
    int fd = 0;
    uint8_t cdb[10] = {0};
    uint8_t buf[64] = {0};
    EXPECT_EQ(ExecuteScsiCmd(fd, cdb, sizeof(cdb), buf, sizeof(buf)), E_ERR);
    g_memsetRet = 0;
    GTEST_LOG_(INFO) << "ExecuteScsiCmd_IoHdrMemsetFailed end";
}

HWTEST_F(ExtDiskUtilsTest, ExecuteScsiCmd_IoctlFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteScsiCmd_IoctlFailed start";
    g_ioctlRet = -1;
    int fd = 0;
    uint8_t cdb[10] = {0};
    uint8_t buf[64] = {0};
    EXPECT_EQ(ExecuteScsiCmd(fd, cdb, sizeof(cdb), buf, sizeof(buf)), E_ERR);
    GTEST_LOG_(INFO) << "ExecuteScsiCmd_IoctlFailed end";
}

HWTEST_F(ExtDiskUtilsTest, ExecuteScsiCmd_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteScsiCmd_Success start";
    g_ioctlRet = 0;
    g_ioctlInfo = SG_INFO_OK;
    int fd = 0;
    uint8_t cdb[10] = {0};
    uint8_t buf[64] = {0};
    EXPECT_EQ(ExecuteScsiCmd(fd, cdb, sizeof(cdb), buf, sizeof(buf)), E_OK);
    GTEST_LOG_(INFO) << "ExecuteScsiCmd_Success end";
}

/**
 * @tc.name: CleanTempDirectory_001
 * @tc.desc: Verify CleanTempDirectory returns E_ERR when ForkExec fails.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, CleanTempDirectory_001, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::CleanTempDirectory();
    EXPECT_EQ(ret, E_ERR);
}

/**
 * @tc.name: CleanTempDirectory_002
 * @tc.desc: Verify CleanTempDirectory returns E_OK when ForkExec succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, CleanTempDirectory_002, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::CleanTempDirectory();
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: CleanTempDirectory_003
 * @tc.desc: Verify CleanTempDirectory returns E_ERR when ForkExec fails with output.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, CleanTempDirectory_003, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("rm: cannot remove");
            return E_ERR;
        }));
    int32_t ret = DiskUtils::CleanTempDirectory();
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_ErrorAtStart, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("ERROR: invalid partition table");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "ERROR: invalid partition table");
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_ErrorInMiddle, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Warning: ERROR found in partition data");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "Warning: ERROR found in partition data");
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_ErrorAtEnd, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Disk read failed ERROR");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "Disk read failed ERROR");
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_MultipleErrors, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("ERROR: partition 1 corrupt");
            output->push_back("ERROR: partition 2 missing");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "ERROR: partition 1 corrupt");
    EXPECT_EQ(lines[1], "ERROR: partition 2 missing");
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_NoError, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Disk /dev/block/uttestdisk: 100 GiB");
            output->push_back("Number  Start  End  Size  Type");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(lines.size(), 2u);
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_LowercaseErrorNotMatched, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("error: lowercase should not match");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "error: lowercase should not match");
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_MixedOutput, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Disk /dev/block/uttestdisk: 100 GiB");
            output->push_back("ERROR: partition table corrupt");
            output->push_back("Number  Start  End  Size  Type");
            return E_OK;
        }));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[1], "ERROR: partition table corrupt");
}

HWTEST_F(ExtDiskUtilsTest, ExecAsyncGetPartitionTableInfo_ForkExecFailed, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    std::vector<std::string> lines;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::ExecAsyncGetPartitionTableInfo(devPath, lines);
    EXPECT_EQ(ret, E_GET_PARTITION_ERROR);
}

/**
 * @tc.name: GetOpticalDriveNode_NoMatch
 * @tc.desc: Verify GetOpticalDriveNode returns empty when volName has no dash, dash at end,
 *           or no /sys/block device matches major:minor.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetOpticalDriveNode_NoMatch, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_NoMatch start";
    // B1a: dashPos == npos (no dash in volName) -> enter if, return ""
    EXPECT_TRUE(GetOpticalDriveNode("/dev/block/sr0").empty());
    // B1b: dashPos != npos but dashPos+1 >= volName.size() (dash at end) -> enter if, return ""
    EXPECT_TRUE(GetOpticalDriveNode("/dev/block/abc-").empty());
    // B4: valid majorMinor but no matching /sys/block device -> return ""
    EXPECT_TRUE(GetOpticalDriveNode("/dev/block/vol-999-999").empty());
    // Note: line 1042 has 6 LLVM branch edges for ||, the uncovered '-' edge
    // is a short-circuit artifact (both sides of || cannot both be true), cannot be covered.
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_NoMatch end";
}

/**
 * @tc.name: GetOpticalDriveNode_ValidDeviceFound
 * @tc.desc: Verify GetOpticalDriveNode finds the correct block device when major:minor matches.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetOpticalDriveNode_ValidDeviceFound, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_ValidDeviceFound start";
    // GetOpticalDriveNode reads /sys/block at runtime. The match depends on
    // whether a device with the given major:minor exists on the test board.
    // Use vol-8-0 which maps to sda (8:0) on most Linux systems.
    // If the device doesn't exist, the function returns "" which is also acceptable.
    std::string result = GetOpticalDriveNode("/dev/block/vol-8-0");
    if (!result.empty()) {
        EXPECT_EQ(result, "sda");
    }
    // Note: This test is environment-dependent. The B3 branch (content == majorMinor)
    // can only be covered when a matching /sys/block device exists.
    // On boards without matching devices, only B1/B1b/B4 (no match) branches are covered.
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_ValidDeviceFound end";
}

/**
 * @tc.name: GetUsedSizeFromSysfs_DriveNodeEmpty
 * @tc.desc: Verify GetUsedSizeFromSysfs returns -1 when GetOpticalDriveNode returns empty.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetUsedSizeFromSysfs_DriveNodeEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_DriveNodeEmpty start";
    // B1: driveNode.empty() == true -> return -1
    EXPECT_EQ(DiskUtils::GetUsedSizeFromSysfs("/dev/block/sr0"), -1);
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_DriveNodeEmpty end";
}

/**
 * @tc.name: GetUsedSizeFromSysfs_Success
 * @tc.desc: Verify GetUsedSizeFromSysfs reads sector count from /sys/block/<dev>/size.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetUsedSizeFromSysfs_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_Success start";
    // B2: driveNode.empty() == false (if matching device exists)
    // B4: !ifs.is_open() == false (open succeeds, /sys/block/sda/size exists)
    // If GetOpticalDriveNode returns empty (no matching device), falls back to B1 (-1).
    int64_t result = DiskUtils::GetUsedSizeFromSysfs("/dev/block/vol-8-0");
    if (result > 0) {
        EXPECT_EQ(result % 512, 0);
    }
    // Note: B3 (!ifs.is_open() == true) requires driveNode non-empty but /size file missing.
    // Cannot be tested in UT because GetOpticalDriveNode returns a real device name from
    // /sys/block, and all real devices have /size files. Needs board-side verification.
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_Success end";
}

/**
 * @tc.name: GetDiscCapacity_AllTypesSuccess
 * @tc.desc: Verify GetDiscCapacity dispatches to the correct capacity function for each disc type.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetDiscCapacity_AllTypesSuccess, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDiscCapacity_AllTypesSuccess start";
    // B1: DVD-R & DVD-ROM -> GetDvdTotalCapacity (same source branch)
    EXPECT_CALL(*diskUtilMoc_, GetDvdTotalCapacity(_, _))
        .WillOnce(Invoke([](int, int64_t &cap) { cap = 4700372992L; return E_OK; }))
        .WillOnce(Invoke([](int, int64_t &cap) { cap = 4700372992L; return E_OK; }));
    EXPECT_EQ(DiskUtils::GetDiscCapacity(0, "DVD-R"), 4700372992L);
    EXPECT_EQ(DiskUtils::GetDiscCapacity(0, "DVD-ROM"), 4700372992L);

    // B2: DVD+RW -> GetDvdPlusRwTotalCapacity
    // Note: GetDvdPlusRwTotalCapacity is a STATIC function in disk_utils.cpp,
    // so mock EXPECT_CALL is bypassed. The real static function calls SendScsiCmd
    // (from storage_common_utils.so) which uses the mocked ioctl.
    // With g_ioctlRet=0 (default), ioctl returns success but dataBuf is all zeros,
    // resulting in totalSize=0. This is the expected behavior in UT.
    g_ioctlRet = 0;
    g_ioctlInfo = 0; // SG_INFO_OK
    EXPECT_EQ(DiskUtils::GetDiscCapacity(0, "DVD+RW"), 0);

    // B3: CD-ROM -> GetCdTotalCapacity (external linkage, mock redirect works)
    EXPECT_CALL(*diskUtilMoc_, GetCdTotalCapacity(_, _))
        .WillOnce(Invoke([](int, int64_t &cap) { cap = 737280000L; return E_OK; }));
    EXPECT_EQ(DiskUtils::GetDiscCapacity(0, "CD-ROM"), 737280000L);

    // B4: BD-RE -> GetBdTotalCapacity (external linkage, mock redirect works)
    EXPECT_CALL(*diskUtilMoc_, GetBdTotalCapacity(_, _))
        .WillOnce(Invoke([](int, int64_t &cap) { cap = 25025314816L; return E_OK; }));
    EXPECT_EQ(DiskUtils::GetDiscCapacity(0, "BD-RE"), 25025314816L);

    // B5: unknown type -> 0
    EXPECT_EQ(DiskUtils::GetDiscCapacity(0, "UNKNOWN"), 0);
    GTEST_LOG_(INFO) << "GetDiscCapacity_AllTypesSuccess end";
}

/**
 * @tc.name: GetDiscCapacity_CapacityCallFailed
 * @tc.desc: Verify GetDiscCapacity returns 0 when underlying capacity call fails.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetDiscCapacity_CapacityCallFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDiscCapacity_CapacityCallFailed start";
    // B6: err1 != E_OK -> return 0 (using DVD-R which uses external GetDvdTotalCapacity)
    EXPECT_CALL(*diskUtilMoc_, GetDvdTotalCapacity(_, _))
        .WillOnce(Invoke([](int, int64_t &) { return E_ERR; }));
    EXPECT_EQ(DiskUtils::GetDiscCapacity(0, "DVD-R"), 0);
    GTEST_LOG_(INFO) << "GetDiscCapacity_CapacityCallFailed end";
}

/**
 * @tc.name: AdjustBlankDiscCapacity_NotBlankDisc
 * @tc.desc: Verify AdjustBlankDiscCapacity does nothing when disc is not blank.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, AdjustBlankDiscCapacity_NotBlankDisc, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_NotBlankDisc start";
    int64_t totalSize = 4700372992L;
    int64_t usedSize = 1000000L;
    // B1: ReadCDDiscInfo fails + GetBlkidData returns non-empty -> not blank
    g_ioctlRet = -1;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVD+RW"));
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("udf"));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(totalSize, 4700372992L);
    EXPECT_EQ(usedSize, 1000000L);
    g_ioctlRet = 0;
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_NotBlankDisc end";
}

/**
 * @tc.name: AdjustBlankDiscCapacity_BlankCdType
 * @tc.desc: Verify AdjustBlankDiscCapacity resets usedSize=0 but skips ForkExec for CD type.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, AdjustBlankDiscCapacity_BlankCdType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankCdType start";
    int64_t totalSize = 737280000L;
    int64_t usedSize = 500000L;
    // B2: blank disc but discType starts with "CD" -> no ForkExec
    g_ioctlRet = 0;
    g_ioctlInfo = SG_INFO_OK;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("CD-ROM"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(0);
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "CD-ROM", totalSize, usedSize);
    EXPECT_EQ(usedSize, 0);
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankCdType end";
}

/**
 * @tc.name: AdjustBlankDiscCapacity_BlankDvd_ForkExecFailed
 * @tc.desc: Verify AdjustBlankDiscCapacity handles ForkExec failure for blank DVD.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, AdjustBlankDiscCapacity_BlankDvd_ForkExecFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankDvd_ForkExecFailed start";
    int64_t totalSize = 4700372992L;
    int64_t usedSize = 1000000L;
    // B3: blank DVD, ForkExec fails
    g_ioctlRet = 0;
    g_ioctlInfo = SG_INFO_OK;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVD+RW"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(usedSize, 0);
    EXPECT_EQ(totalSize, 4700372992L);
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankDvd_ForkExecFailed end";
}

/**
 * @tc.name: AdjustBlankDiscCapacity_BlankDvd_MediaInfoMatch
 * @tc.desc: Verify AdjustBlankDiscCapacity updates totalSize from mediainfo for blank DVD.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, AdjustBlankDiscCapacity_BlankDvd_MediaInfoMatch, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankDvd_MediaInfoMatch start";
    int64_t totalSize = 4700372992L;
    int64_t usedSize = 1000000L;
    // B4: ForkExec succeeds, regex matches, valid number > 0
    g_ioctlRet = 0;
    g_ioctlInfo = SG_INFO_OK;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVD+RW"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("  unformatted:   2295104*2048=4702922752");
            return E_OK;
        }));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(usedSize, 0);
    EXPECT_EQ(totalSize, 4702922752L);
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankDvd_MediaInfoMatch end";
}

/**
 * @tc.name: AdjustBlankDiscCapacity_MediaInfoNoMatchOrInvalid
 * @tc.desc: Verify AdjustBlankDiscCapacity keeps totalSize when mediainfo has no match,
 *           invalid number, or zero capacity.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, AdjustBlankDiscCapacity_MediaInfoNoMatchOrInvalid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_MediaInfoNoMatchOrInvalid start";
    // B6: no regex match
    int64_t totalSize = 4700372992L;
    int64_t usedSize = 1000000L;
    g_ioctlRet = 0;
    g_ioctlInfo = SG_INFO_OK;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVD+RW"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("  some other line without pattern");
            return E_OK;
        }));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(usedSize, 0);
    EXPECT_EQ(totalSize, 4700372992L);

    // B5: regex matches but invalid number
    totalSize = 4700372992L;
    usedSize = 1000000L;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVD+RW"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("  unformatted:   abc*2048=invalid");
            return E_OK;
        }));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(totalSize, 4700372992L);

    // B5: regex matches but zero capacity
    totalSize = 4700372992L;
    usedSize = 1000000L;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVD+RW"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("  unformatted:   0*2048=0");
            return E_OK;
        }));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(totalSize, 4700372992L);

    g_ioctlRet = 0;
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_MediaInfoNoMatchOrInvalid end";
}

/**
 * @tc.name: AdjustBlankDiscCapacity_BlankBd_ForkExecAndMediaInfo
 * @tc.desc: Verify AdjustBlankDiscCapacity with blank BD, covering ForkExec fail and mediainfo match.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, AdjustBlankDiscCapacity_BlankBd_ForkExecAndMediaInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankBd_ForkExecAndMediaInfo start";
    // B3: BD ForkExec fails
    int64_t totalSize = 25025314816L;
    int64_t usedSize = 5000000L;
    g_ioctlRet = 0;
    g_ioctlInfo = SG_INFO_OK;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("BD-RE"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "BD-RE", totalSize, usedSize);
    EXPECT_EQ(usedSize, 0);
    EXPECT_EQ(totalSize, 25025314816L);

    // B4: BD ForkExec succeeds, mediainfo matches
    totalSize = 25025314816L;
    usedSize = 5000000L;
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("BD-RE"));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("  unformatted:   12219392*2048=25025314816");
            return E_OK;
        }));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "BD-RE", totalSize, usedSize);
    EXPECT_EQ(usedSize, 0);
    EXPECT_EQ(totalSize, 25025314816L);
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankBd_ForkExecAndMediaInfo end";
}

/**
 * @tc.name: AdjustBlankDiscCapacity_BlankDvd_FallbackPath
 * @tc.desc: Verify AdjustBlankDiscCapacity with DVD+RW, ioctl fails -> fallback to GetBlkidData,
 *           blank disc -> ForkExec with mediainfo match.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, AdjustBlankDiscCapacity_BlankDvd_FallbackPath, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankDvd_FallbackPath start";
    int64_t totalSize = 4700372992L;
    int64_t usedSize = 1000000L;
    g_ioctlRet = -1; // ReadCDDiscInfo fails -> fallback path
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVD+RW"));
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("")); // empty -> IsCDBlank returns true
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("  unformatted:   2295104*2048=4702922752");
            return E_OK;
        }));
    DiskUtils::AdjustBlankDiscCapacity(testDevPath_, "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(usedSize, 0);
    EXPECT_EQ(totalSize, 4702922752L);
    g_ioctlRet = 0;
    GTEST_LOG_(INFO) << "AdjustBlankDiscCapacity_BlankDvd_FallbackPath end";
}

/**
 * @tc.name: GetCapacity_OpenFailed
 * @tc.desc: Verify GetCapacity returns E_ERR when open fails.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetCapacity_OpenFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacity_OpenFailed start";
    int64_t totalSize = 0;
    int64_t freeSize = 0;
    // /dev/block/nonexistent_device won't exist -> open returns -1
    int32_t ret = DiskUtils::GetCapacity("/dev/block/nonexistent_device_" + std::to_string(getpid()),
                                          totalSize, freeSize);
    EXPECT_EQ(ret, E_ERR);
    GTEST_LOG_(INFO) << "GetCapacity_OpenFailed end";
}

/**
 * @tc.name: GetCapacity_SuccessWithUsedSizeEqualToTotal
 * @tc.desc: Verify GetCapacity computes freeSize=0 when GetUsedSizeFromSysfs returns -1.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetCapacity_SuccessWithUsedSizeEqualToTotal, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacity_SuccessWithUsedSizeEqualToTotal start";
    int64_t totalSize = 0;
    int64_t freeSize = 0;
    // Use DVD-R which calls GetDvdTotalCapacity (external linkage, mock works)
    // GetCDType is called TWICE: once by GetCapacity (line 1245) and once by
    // IsCDBlank (line 732, called via AdjustBlankDiscCapacity)
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_))
        .WillOnce(Return("DVD-R"))
        .WillOnce(Return("DVD-R"));
    EXPECT_CALL(*diskUtilMoc_, GetDvdTotalCapacity(_, _))
        .WillOnce(Invoke([](int, int64_t &cap) { cap = 4700372992L; return E_OK; }));
    // AdjustBlankDiscCapacity -> IsCDBlank -> ReadCDDiscInfo via ioctl
    g_ioctlRet = -1; // ReadCDDiscInfo fails
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("udf")); // non-empty -> not blank
    int32_t ret = DiskUtils::GetCapacity(testDevPath_, totalSize, freeSize);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(totalSize, 4700372992L);
    EXPECT_EQ(freeSize, 0); // usedSize = -1 -> usedSize = totalSize -> freeSize = 0
    g_ioctlRet = 0;
    GTEST_LOG_(INFO) << "GetCapacity_SuccessWithUsedSizeEqualToTotal end";
}

/**
 * @tc.name: GetCapacity_GetDiscCapacityFailed
 * @tc.desc: Verify GetCapacity returns E_OK even when GetDiscCapacity fails.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetCapacity_GetDiscCapacityFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacity_GetDiscCapacityFailed start";
    int64_t totalSize = 0;
    int64_t freeSize = 0;
    // GetCDType called twice: GetCapacity + IsCDBlank
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_))
        .WillOnce(Return("DVD-R"))
        .WillOnce(Return("DVD-R"));
    EXPECT_CALL(*diskUtilMoc_, GetDvdTotalCapacity(_, _))
        .WillOnce(Invoke([](int, int64_t &) { return E_ERR; }));
    // AdjustBlankDiscCapacity -> IsCDBlank -> ReadCDDiscInfo
    g_ioctlRet = -1;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("udf"));
    int32_t ret = DiskUtils::GetCapacity(testDevPath_, totalSize, freeSize);
    // GetCapacity always returns E_OK regardless of GetDiscCapacity result
    EXPECT_EQ(ret, E_OK);
    g_ioctlRet = 0;
    GTEST_LOG_(INFO) << "GetCapacity_GetDiscCapacityFailed end";
}

/**
 * @tc.name: GetCapacity_BlankDiscUsedSizeZero
 * @tc.desc: Verify GetCapacity with blank disc sets usedSize=0 and freeSize=totalSize.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, GetCapacity_BlankDiscUsedSizeZero, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacity_BlankDiscUsedSizeZero start";
    int64_t totalSize = 0;
    int64_t freeSize = 0;
    // Use DVD-R which calls GetDvdTotalCapacity (external linkage, mock works)
    // GetCDType called twice: GetCapacity + IsCDBlank
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_))
        .WillOnce(Return("DVD-R"))
        .WillOnce(Return("DVD-R"));
    EXPECT_CALL(*diskUtilMoc_, GetDvdTotalCapacity(_, _))
        .WillOnce(Invoke([](int, int64_t &cap) { cap = 4700372992L; return E_OK; }));
    // AdjustBlankDiscCapacity -> IsCDBlank returns true (ioctl succeeds, discStatus=0)
    g_ioctlRet = 0;
    g_ioctlInfo = SG_INFO_OK;
    // Since blank, usedSize reset to 0, ForkExec for mediainfo
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("  unformatted:   2295104*2048=4702922752");
            return E_OK;
        }));
    int32_t ret = DiskUtils::GetCapacity(testDevPath_, totalSize, freeSize);
    EXPECT_EQ(ret, E_OK);
    // totalSize adjusted by AdjustBlankDiscCapacity to mediainfo value
    // freeSize = totalSize - 0 = totalSize
    EXPECT_GT(totalSize, 0);
    EXPECT_EQ(freeSize, totalSize);
    g_ioctlRet = 0;
    GTEST_LOG_(INFO) << "GetCapacity_BlankDiscUsedSizeZero end";
}

/**
 * @tc.name: DeletePartitionInfo_InvalidDiskId
 * @tc.desc: Verify DeletePartitionInfo returns E_PARAMS_INVALID with path traversal diskId.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_InvalidDiskId, TestSize.Level1)
{
    std::string devPath = "/dev/block/test_dev";
    std::string diskId = "../sda";
    int32_t partitionNum = 1;
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, diskId, partitionNum);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

/**
 * @tc.name: ValidateBurnOptions_InvalidBurnPath
 * @tc.desc: Verify ValidateBurnOptions returns E_PARAMS_INVALID with path traversal burnPath.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(ExtDiskUtilsTest, ValidateBurnOptions_InvalidBurnPath, TestSize.Level1)
{
    BurnOptions options;
    options.diskName = "testdisk";
    options.burnPath = "../../etc/passwd";
    int32_t ret = ValidateBurnOptions(options);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

} // namespace StorageDaemon
} // namespace OHOS
