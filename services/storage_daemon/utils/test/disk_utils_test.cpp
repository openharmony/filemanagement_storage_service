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

#include <gtest/gtest.h>
#include <sys/sysmacros.h>

#include "mock/file_utils_mock.h"
#include "mock/disk_func_mock.h"
#include "utils/disk_utils.h"
#include "storage_service_errno.h"
#include "securec.h"

#include "disk_func_define.h"
#include "disk_utils.cpp"
#include "disk_func_undef.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
using namespace testing;

class DiskUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::shared_ptr<DiskFuncMock> diskFuncMock_ = nullptr;
};

void DiskUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    diskFuncMock_ = std::make_shared<DiskFuncMock>();
    DiskFuncMock::diskFunc_ = diskFuncMock_;
}

void DiskUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    DiskFuncMock::diskFunc_ = nullptr;
    diskFuncMock_ = nullptr;
}

void DiskUtilsTest::SetUp(void)
{
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    diskFuncMock_ = std::make_shared<DiskFuncMock>();
    DiskFuncMock::diskFunc_ = diskFuncMock_;
}

void DiskUtilsTest::TearDown(void)
{
    if (fileUtilMoc_) {
        Mock::VerifyAndClearExpectations(fileUtilMoc_.get());
    }
    if (diskFuncMock_) {
        Mock::VerifyAndClearExpectations(diskFuncMock_.get());
    }
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    DiskFuncMock::diskFunc_ = nullptr;
    diskFuncMock_ = nullptr;
}

/**
 * @tc.name: DiskUtilsTest_IsBlankCD_01
 * @tc.desc: Verify IsBlankCD function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsBlankCD_01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_01 start";
    std::string diskBlock = "/dev/test/sr01";
    FILE * tmpFile = tmpfile();
    bool isBlankCD = false;
    char realPath = '\0';
    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(IsBlankCD(diskBlock, isBlankCD), E_ERR);
    EXPECT_EQ(isBlankCD, false);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(CDS_NO_DISC));
    EXPECT_EQ(IsBlankCD(diskBlock, isBlankCD), E_ERR);
    EXPECT_EQ(isBlankCD, false);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillRepeatedly(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillRepeatedly(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillRepeatedly(Return(CDS_DISC_OK));
    EXPECT_EQ(IsBlankCD(diskBlock, isBlankCD), E_OK);
    EXPECT_EQ(isBlankCD, true);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath)).WillOnce(Return(nullptr));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillRepeatedly(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillRepeatedly(Return(CDS_DISC_OK));
    EXPECT_EQ(IsBlankCD(diskBlock, isBlankCD), E_ERR);
    EXPECT_EQ(isBlankCD, false);
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_01 end";
}

/**
 * @tc.name: DiskUtilsTest_DiskType2Str_01
 * @tc.desc: the DiskType2Str function to convert code for various CD types has been verified.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_DiskType2Str_01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_DiskType2Str_01 start";

    std::string str = DiskType2Str(0x08); // 0x08 is CD-ROM
    EXPECT_EQ(str, "CD-ROM");

    str = DiskType2Str(0x09); // 0x09 is CD-R
    EXPECT_EQ(str, "CD-R");

    str = DiskType2Str(0x0A); // 0x0A is CD-RW
    EXPECT_EQ(str, "CD-RW");

    str = DiskType2Str(0x10); // 0x10 is DVD-ROM
    EXPECT_EQ(str, "DVD-ROM");

    str = DiskType2Str(0x11); // 0x11 is DVD-R
    EXPECT_EQ(str, "DVD-R");

    str = DiskType2Str(0x12); // 0x12 is DVD-RAM
    EXPECT_EQ(str, "DVD-RAM");

    str = DiskType2Str(0x13); // 0x13 is DVD-RW
    EXPECT_EQ(str, "DVD-RW");

    str = DiskType2Str(0x14); // 0x14 is DVD-RW
    EXPECT_EQ(str, "DVD-RW");

    str = DiskType2Str(0x1A); // 0x1A is DVD+RW
    EXPECT_EQ(str, "DVD+RW");

    str = DiskType2Str(0x1B); // 0x1B is DVD+R
    EXPECT_EQ(str, "DVD+R");

    str = DiskType2Str(0x1C); // 0x1C is DVD+R
    EXPECT_EQ(str, "DVD+R");

    str = DiskType2Str(0x1D); // 0x1D is DVD+RW
    EXPECT_EQ(str, "DVD+RW");

    str = DiskType2Str(0x01); // 0x01 is default
    EXPECT_EQ(str, "");

    GTEST_LOG_(INFO) << "DiskUtilsTest_DiskType2Str_01 end";
}

/**
 * @tc.name: DiskUtilsTest_GetCDType_01
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetCDType_01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDType_01 start";

    std::string diskPath = "/dev/test/getcdtype01";
    char realPath = '\0';
    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(nullptr));

    std::string str = GetCDType(diskPath);
    EXPECT_EQ(str, "");
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDType_01 end";
}

/**
 * @tc.name: DiskUtilsTest_GetCDType_02
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetCDType_02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDType_02 start";

    std::string diskPath = "/dev/test/getcdtype02";
    FILE * tmpFile = tmpfile();
    char realPath = '\0';
    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(0));

    std::string str = GetCDType(diskPath);
    EXPECT_EQ(str, "");
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDType_02 end";
}

/**
 * @tc.name: DiskUtilsTest_GetCDType_03
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetCDType_03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDType_03 start";

    std::string diskPath = "/dev/test/getcdtype02";
    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(nullptr));

    std::string str = GetCDType(diskPath);
    EXPECT_EQ(str, "");
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDType_03 end";
}

/**
 * @tc.name: DiskUtilsTest_GetCDStatus_01
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetCDStatus_01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDStatus_01 start";

    std::string diskPath = "/dev/test/test01";
    FILE * tmpFile = tmpfile();
    char realPath = '\0';
    int status = 0;
    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(nullptr));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_FILE_PATH_INVALID);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(nullptr));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_SYS_KERNEL_ERR);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(-1));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_SYS_KERNEL_ERR);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(-1));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_ERR);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_OK);
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDStatus_01 end";
}

/**
 * @tc.name: DiskUtilsTest_GetCDStatus_02
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetCDStatus_02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDStatus_02 start";

    std::string diskPath = "/dev/test/test02";
    FILE * tmpFile = tmpfile();
    char realPath = '\0';
    int status = 0;

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(1));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_OK);
    EXPECT_EQ(status, 1);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(2));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_OK);
    EXPECT_EQ(status, 2);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(3));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_OK);
    EXPECT_EQ(status, 3);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(4));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_OK);
    EXPECT_EQ(status, 4);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(0));
    EXPECT_EQ(GetCDStatus(diskPath.c_str(), status), E_OK);
    EXPECT_EQ(status, 0);
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetCDStatus_02 end";
}

/**
 * @tc.name: DiskUtilsTest_IsExistCD_01
 * @tc.desc: test cd interface.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsExistCD_01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsExistCD_01 start";
    std::string diskBlock = "/dev/test/sr01";
    FILE * tmpFile = tmpfile();
    bool isExistCD = false;
    char realPath = '\0';
    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(nullptr));
    EXPECT_EQ(IsExistCD(diskBlock, isExistCD), E_ERR);
    EXPECT_EQ(isExistCD, false);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(CDS_DISC_OK));
    EXPECT_EQ(IsExistCD(diskBlock, isExistCD), E_OK);
    EXPECT_EQ(isExistCD, true);

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(&realPath));
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, fclose(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(CDS_DRIVE_NOT_READY));
    EXPECT_EQ(IsExistCD(diskBlock, isExistCD), E_OK);
    EXPECT_EQ(isExistCD, true);
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsExistCD_01 end";
}

/**
 * @tc.name: DiskUtilsTest_SendScsiCmdByPath_001
 * @tc.desc: Verify SendScsiCmdByPath with valid path and successful ioctl.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_SendScsiCmdByPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_SendScsiCmdByPath_001 start";
    
    std::string diskPath = "/dev/sr0";
    uint8_t cdb[10] = {0};
    uint8_t buf[64] = {0};

    int ret = SendScsiCmdByPath(diskPath, cdb, sizeof(cdb), buf, sizeof(buf));
    EXPECT_EQ(ret, E_ERR);

    GTEST_LOG_(INFO) << "DiskUtilsTest_SendScsiCmdByPath_001 end";
}

/**
 * @tc.name: DiskUtilsTest_SendScsiCmdByPath_002
 * @tc.desc: Verify SendScsiCmdByPath with invalid path (realpath fails).
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_SendScsiCmdByPath_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_SendScsiCmdByPath_002 start";
    
    std::string diskPath = "/invalid/path";
    uint8_t cdb[10] = {0};
    uint8_t buf[64] = {0};

    EXPECT_CALL(*diskFuncMock_, realpath(_, _)).WillOnce(Return(nullptr));

    int ret = SendScsiCmdByPath(diskPath, cdb, sizeof(cdb), buf, sizeof(buf));
    EXPECT_EQ(ret, E_ERR);

    GTEST_LOG_(INFO) << "DiskUtilsTest_SendScsiCmdByPath_002 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadConfiguration_001
 * @tc.desc: Verify ReadConfiguration with successful SCSI command.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadConfiguration_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_001 start";
    
    std::string diskPath = "/dev/sr0";
    uint8_t buf[64] = {0};

    int ret = ReadConfiguration(diskPath, buf, sizeof(buf));
    EXPECT_EQ(ret, E_ERR);

    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_001 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadConfiguration_002
 * @tc.desc: Verify ReadConfiguration with nullptr buf returns E_PARAMS_INVALID.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadConfiguration_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_002 start";

    std::string diskPath = "/dev/sr0";
    int ret = ReadConfiguration(diskPath, nullptr, 64);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_002 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadConfiguration_003
 * @tc.desc: Verify ReadConfiguration with zero len returns E_PARAMS_INVALID.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadConfiguration_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_003 start";

    std::string diskPath = "/dev/sr0";
    uint8_t buf[64] = {0};
    int ret = ReadConfiguration(diskPath, buf, 0);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_003 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadConfiguration_004
 * @tc.desc: Verify ReadConfiguration with negative len returns E_PARAMS_INVALID.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadConfiguration_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_004 start";

    std::string diskPath = "/dev/sr0";
    uint8_t buf[64] = {0};
    int ret = ReadConfiguration(diskPath, buf, -1);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadConfiguration_004 end";
}

/**
 * @tc.name: DiskUtilsTest_GetOpticalDriveMaxWriteSpeed_002
 * @tc.desc: Verify GetOpticalDriveMaxWriteSpeed fails when Mode Sense fails.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetOpticalDriveMaxWriteSpeed_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOpticalDriveMaxWriteSpeed_001 start";
    
    std::string diskPath = "/dev/sr0";
    int32_t maxWriteSpeed = 0;

    int ret = GetOpticalDriveMaxWriteSpeed(diskPath, maxWriteSpeed);
    EXPECT_EQ(ret, E_ERR);

    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOpticalDriveMaxWriteSpeed_001 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetScsiBusNum_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_001 start";
    std::string sysPath = "/sys/block/sda";
    char linkTarget[] = "../../../0:0:0:0";
    
    EXPECT_CALL(*diskFuncMock_, readlink(_, _, _)).WillOnce([&](const char* path, char* buf, size_t bufsiz) {
        strncpy_s(buf, bufsiz, linkTarget, bufsiz - 1);
        return strlen(linkTarget);
    });
    
    std::string result = GetScsiBusNum(sysPath);
    EXPECT_EQ(result, "0:0:0:0");
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_001 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetScsiBusNum_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_002 start";
    std::string sysPath = "/sys/block/sda";
    
    EXPECT_CALL(*diskFuncMock_, readlink(_, _, _)).WillOnce(Return(-1));
    
    std::string result = GetScsiBusNum(sysPath);
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_002 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetScsiBusNum_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_003 start";
    std::string sysPath = "/sys/block/sda";
    char linkTarget[] = "nodivider";
    
    EXPECT_CALL(*diskFuncMock_, readlink(_, _, _)).WillOnce([&](const char* path, char* buf, size_t bufsiz) {
        strncpy_s(buf, bufsiz, linkTarget, bufsiz - 1);
        return strlen(linkTarget);
    });
    
    std::string result = GetScsiBusNum(sysPath);
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_003 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetScsiBusNum_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_004 start";
    std::string sysPath = "/sys/block/sda";
    char linkTarget[] = "path/without/colon";
    
    EXPECT_CALL(*diskFuncMock_, readlink(_, _, _)).WillOnce([&](const char* path, char* buf, size_t bufsiz) {
        strncpy_s(buf, bufsiz, linkTarget, bufsiz - 1);
        return strlen(linkTarget);
    });
    
    std::string result = GetScsiBusNum(sysPath);
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetScsiBusNum_004 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetOddDriverType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_001 start";
    std::string sysPath = "/sys/block/sr0/usb-device";
    
    std::string result = GetOddDriverType(sysPath);
    EXPECT_EQ(result, "usb-storage");
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_001 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetOddDriverType_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_002 start";
    std::string sysPath = "/sys/block/sr0/sata-link";
    
    std::string result = GetOddDriverType(sysPath);
    EXPECT_EQ(result, "AHCI");
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_002 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetOddDriverType_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_003 start";
    std::string sysPath = "/sys/block/sr0/other-type";
    
    std::string result = GetOddDriverType(sysPath);
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_003 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetOddDriverType_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_004 start";
    std::string sysPath = "/sys/block/sr0";
    
    std::string result = GetOddDriverType(sysPath);
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DiskUtilsTest_GetOddDriverType_004 end";
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_SendScsiCmdByPath_NullCdb, TestSize.Level1)
{
    std::string diskPath = "/dev/sr0";
    uint8_t buf[64] = {0};
    int ret = SendScsiCmdByPath(diskPath, nullptr, 10, buf, sizeof(buf));
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_SendScsiCmdByPath_NullBuf, TestSize.Level1)
{
    std::string diskPath = "/dev/sr0";
    uint8_t cdb[10] = {0};
    int ret = SendScsiCmdByPath(diskPath, cdb, sizeof(cdb), nullptr, 64);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_SendScsiCmdByPath_ZeroCdbLen, TestSize.Level1)
{
    std::string diskPath = "/dev/sr0";
    uint8_t cdb[10] = {0};
    uint8_t buf[64] = {0};
    int ret = SendScsiCmdByPath(diskPath, cdb, 0, buf, sizeof(buf));
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_SendScsiCmdByPath_CdbLenTooLarge, TestSize.Level1)
{
    std::string diskPath = "/dev/sr0";
    uint8_t cdb[20] = {0};
    uint8_t buf[64] = {0};
    int ret = SendScsiCmdByPath(diskPath, cdb, 20, buf, sizeof(buf));
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_SendScsiCmdByPath_NegativeLen, TestSize.Level1)
{
    std::string diskPath = "/dev/sr0";
    uint8_t cdb[10] = {0};
    uint8_t buf[64] = {0};
    int ret = SendScsiCmdByPath(diskPath, cdb, sizeof(cdb), buf, -1);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetBlkidData_DashPath, TestSize.Level1)
{
    std::string result = GetBlkidData("-evil", "UUID");
    EXPECT_EQ(result, "");
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetBlkidData_NormalPath, TestSize.Level1)
{
    std::string result = GetBlkidData("/dev/block/sda1", "UUID");
    EXPECT_NE(result, "-evil");
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetMaxVolume_ConvertStringToInt32Fail, TestSize.Level1)
{
    dev_t mmcDev = makedev(179, 0);
    EXPECT_CALL(*fileUtilMoc_, ReadFile(_, _)).WillOnce(Invoke([](const std::string&, std::string* str) {
        *str = "not_a_number";
        return true;
    }));
    EXPECT_EQ(GetMaxVolume(mmcDev), E_ERR);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetMaxVolume_ConvertStringToInt32Success, TestSize.Level1)
{
    dev_t mmcDev = makedev(179, 0);
    EXPECT_CALL(*fileUtilMoc_, ReadFile(_, _)).WillOnce(Invoke([](const std::string&, std::string* str) {
        *str = "8";
        return true;
    }));
    EXPECT_EQ(GetMaxVolume(mmcDev), 8);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetMaxVolume_ScsiDevice, TestSize.Level1)
{
    dev_t scsiDev = makedev(8, 0);
    EXPECT_EQ(GetMaxVolume(scsiDev), MAX_SCSI_VOLUMES);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_GetMaxVolume_ReadFileFail, TestSize.Level1)
{
    dev_t mmcDev = makedev(179, 0);
    EXPECT_CALL(*fileUtilMoc_, ReadFile(_, _)).WillOnce(Return(false));
    EXPECT_EQ(GetMaxVolume(mmcDev), E_ERR);
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsAcceptableUuid_IsFilePathInvalid, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).WillOnce(Return(true));
    EXPECT_FALSE(IsAcceptableUuid("valid-uuid-1234"));
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsAcceptableUuid_IsFilePathInvalidFalse, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).WillOnce(Return(false));
    EXPECT_TRUE(IsAcceptableUuid("valid-uuid-1234"));
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsAcceptableUuid_Empty, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).Times(0);
    EXPECT_FALSE(IsAcceptableUuid(""));
}

HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsAcceptableUuid_TooLong, TestSize.Level1)
{
    EXPECT_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).Times(0);
    std::string longUuid(41, 'a');
    EXPECT_FALSE(IsAcceptableUuid(longUuid));
}
} // STORAGE_DAEMON
} // OHOS
