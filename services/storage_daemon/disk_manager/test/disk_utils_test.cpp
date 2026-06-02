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
    int32_t partitionNum = 1;
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, partitionNum);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_PathTraversal, TestSize.Level1)
{
    std::string devPath = "/dev/block/../sda";
    int32_t partitionNum = 1;
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, partitionNum);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_InvalidPrefix, TestSize.Level1)
{
    std::string devPath = "/dev/sda";
    int32_t partitionNum = 1;
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, partitionNum);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_ForkExecFailed, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 1;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, partitionNum);
    EXPECT_EQ(ret, E_DELETE_PARTITION_ERROR);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_Success, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 1;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(Invoke([](std::vector<std::string> &, std::vector<std::string> *output, int *) {
            output->push_back("Deleting partition 1");
            return E_OK;
        }));
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, partitionNum);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(ExtDiskUtilsTest, DeletePartitionInfo_WithDifferentPartitionNum, TestSize.Level1)
{
    std::string devPath = "/dev/block/uttestdisk";
    int32_t partitionNum = 5;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = DiskUtils::DeletePartitionInfo(devPath, partitionNum);
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
 * @tc.name: DiskPathToVolPath_001
 * @tc.desc: Verify normal disk path conversion (disk-X-Y → vol-X-Y+1).
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, DiskPathToVolPath_001, TestSize.Level1)
{
    EXPECT_EQ(DiskUtils::DiskPathToVolPath("/dev/block/disk-8-0"), "/dev/block/vol-8-1");
}

/**
 * @tc.name: DiskPathToVolPath_002
 * @tc.desc: Verify path without /disk- prefix is returned unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, DiskPathToVolPath_002, TestSize.Level1)
{
    EXPECT_EQ(DiskUtils::DiskPathToVolPath("/dev/block/sda"), "/dev/block/sda");
}

/**
 * @tc.name: DiskPathToVolPath_003
 * @tc.desc: Verify minor number increment with non-zero value.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, DiskPathToVolPath_003, TestSize.Level1)
{
    EXPECT_EQ(DiskUtils::DiskPathToVolPath("/dev/block/disk-8-5"), "/dev/block/vol-8-6");
}

/**
 * @tc.name: DiskPathToVolPath_004
 * @tc.desc: Verify large major/minor numbers are handled correctly.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, DiskPathToVolPath_004, TestSize.Level1)
{
    EXPECT_EQ(DiskUtils::DiskPathToVolPath("/dev/block/disk-259-10"), "/dev/block/vol-259-11");
}

/**
 * @tc.name: DiskPathToVolPath_005
 * @tc.desc: Verify empty string is returned unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, DiskPathToVolPath_005, TestSize.Level1)
{
    EXPECT_EQ(DiskUtils::DiskPathToVolPath(""), "");
}

/**
 * @tc.name: DiskPathToVolPath_006
 * @tc.desc: Verify disk path with disk- in wrong position (no leading /) is returned unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsTest, DiskPathToVolPath_006, TestSize.Level1)
{
    EXPECT_EQ(DiskUtils::DiskPathToVolPath("disk-8-0"), "disk-8-0");
}

} // namespace StorageDaemon
} // namespace OHOS
