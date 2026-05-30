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
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <scsi/sg.h>
#include <linux/cdrom.h>

#include "disk_manager/disk/disk_utils.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"

#include "securec.h"
#include <cstring>
#include <cstdarg>

int g_memsetRet = 0;
int g_realpathRet = 0;
char g_realpathBuf[PATH_MAX] = {0};

extern "C" errno_t memset_s(void *dest, size_t destMax, int value, size_t count)
{
    if (g_memsetRet != 0) {
        return g_memsetRet;
    }
    (void)memset(dest, value, count);
    return 0;
}

extern "C" char* realpath(const char *path, char *resolvedPath)
{
    (void)path;
    if (g_realpathRet != 0) {
        return nullptr;
    }
    if (resolvedPath == nullptr) {
        return nullptr;
    }
    if (g_realpathBuf[0] != '\0') {
        if (strcpy_s(resolvedPath, PATH_MAX, g_realpathBuf) != 0) {
            return nullptr;
        }
    } else {
        if (strcpy_s(resolvedPath, PATH_MAX, "/dev/sr0") != 0) {
            return nullptr;
        }
    }
    return resolvedPath;
}

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class DiskUtilsForIOTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::string testDevPath_;
};

void DiskUtilsForIOTest::SetUpTestCase(void)
{
    testDevPath_ = "/dev/block/disk_utils_io_test_" + std::to_string(getpid());
    int fd = creat(testDevPath_.c_str(), 0600);
    if (fd >= 0) {
        close(fd);
    }
}

void DiskUtilsForIOTest::TearDownTestCase(void)
{
    unlink(testDevPath_.c_str());
}

void DiskUtilsForIOTest::SetUp(void)
{
    diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
    DiskUtilMoc::diskUtilMoc = diskUtilMoc_;

    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;

    g_memsetRet = 0;
    g_realpathRet = 0;
    memset(g_realpathBuf, 0, sizeof(g_realpathBuf));
}

void DiskUtilsForIOTest::TearDown(void)
{
    DiskUtilMoc::diskUtilMoc = nullptr;
    diskUtilMoc_ = nullptr;

    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
}

HWTEST_F(DiskUtilsForIOTest, ReadCDDiscInfo_RealpathFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCDDiscInfo_RealpathFailed start";
    g_realpathRet = -1;
    uint8_t buf[64] = {0};
    EXPECT_EQ(ReadCDDiscInfo("/dev/sr0", 0x51, buf, sizeof(buf)), E_ERR);
    g_realpathRet = 0;
    GTEST_LOG_(INFO) << "ReadCDDiscInfo_RealpathFailed end";
}

HWTEST_F(DiskUtilsForIOTest, GetCDDiskStatus_RealpathFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCDDiskStatus_RealpathFailed start";
    g_realpathRet = -1;
    int status = 0;
    EXPECT_EQ(GetCDDiskStatus("/dev/sr0", status), E_FILE_PATH_INVALID);
    g_realpathRet = 0;
    GTEST_LOG_(INFO) << "GetCDDiskStatus_RealpathFailed end";
}

HWTEST_F(DiskUtilsForIOTest, IsCDExist_GetCDDiskStatusFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCDExist_GetCDDiskStatusFailed start";
    g_realpathRet = -1;
    bool isCDExist = true;
    EXPECT_EQ(IsCDExist("/dev/sr0", isCDExist), E_ERR);
    g_realpathRet = 0;
    GTEST_LOG_(INFO) << "IsCDExist_GetCDDiskStatusFailed end";
}

HWTEST_F(DiskUtilsForIOTest, IsCDBlank_ReadCDDiscInfoFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCDBlank_ReadCDDiscInfoFailed start";
    g_realpathRet = -1;
    bool isCDBlank = true;
    EXPECT_EQ(IsCDBlank("/dev/sr0", isCDBlank), E_ERR);
    g_realpathRet = 0;
    GTEST_LOG_(INFO) << "IsCDBlank_ReadCDDiscInfoFailed end";
}

HWTEST_F(DiskUtilsForIOTest, QueryCDStatus_IsCDExistFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryCDStatus_IsCDExistFailed start";
    g_realpathRet = -1;
    int32_t status = 0;
    EXPECT_EQ(DiskUtils::QueryCDStatus("/dev/sr0", status), E_ERR);
    g_realpathRet = 0;
    GTEST_LOG_(INFO) << "QueryCDStatus_IsCDExistFailed end";
}

HWTEST_F(DiskUtilsForIOTest, EjectCD_ForkExecFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EjectCD_ForkExecFailed start";
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(DiskUtils::EjectCD("/dev/sr0"), E_ERR);
    GTEST_LOG_(INFO) << "EjectCD_ForkExecFailed end";
}

HWTEST_F(DiskUtilsForIOTest, EjectCD_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EjectCD_Success start";
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(DiskUtils::EjectCD("/dev/sr0"), E_OK);
    GTEST_LOG_(INFO) << "EjectCD_Success end";
}
} // namespace StorageDaemon
} // namespace OHOS
