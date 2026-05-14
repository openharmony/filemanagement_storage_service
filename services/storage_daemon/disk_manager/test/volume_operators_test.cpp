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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "disk_manager/volume/exfat_operator.h"
#include "disk_manager/volume/ntfs_operator.h"
#include "disk_manager/volume/vfat_operator.h"
#include "library_func_mock.h"
#include "mock/file_utils_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

static auto MakeForkExecWithExit(int exitCode, int retCode)
{
    return Invoke([exitCode, retCode](std::vector<std::string> &, std::vector<std::string> *, int *exitStatus) {
        if (exitStatus != nullptr) { *exitStatus = exitCode; }
        return retCode;
    });
}

class ExtOperatorTest : public testing::Test {
public:
    void SetUp() override
    {
        fileUtilMoc_ = std::make_shared<FileUtilMoc>();
        FileUtilMoc::fileUtilMoc = fileUtilMoc_;
        libraryFuncMock_ = std::make_shared<LibraryFuncMock>();
        LibraryFunc::libraryFunc_ = libraryFuncMock_;
    }
    void TearDown() override
    {
        FileUtilMoc::fileUtilMoc = nullptr;
        fileUtilMoc_ = nullptr;
        LibraryFunc::libraryFunc_ = nullptr;
        libraryFuncMock_ = nullptr;
    }
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::shared_ptr<LibraryFuncMock> libraryFuncMock_ = nullptr;
};

HWTEST_F(ExtOperatorTest, ExfatOperator_DoMount, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", 0), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", 0), E_EXFAT_MOUNT);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", MS_RDONLY), E_OK);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_Format, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Format("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_NO_CHILD));
    EXPECT_EQ(op.Format("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/sda1"), E_ERR);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_Check, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Check("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Check("/dev/block/sda1"), E_VOL_NEED_FIX);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_Repair, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_OK));
    EXPECT_EQ(op.Repair("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(0, E_OK));
    EXPECT_EQ(op.Repair("/dev/block/sda1"), E_VOL_FIX_FAILED);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_ERR));
    EXPECT_EQ(op.Repair("/dev/block/sda1"), E_VOL_FIX_FAILED);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_SetLabel, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/sda1", "MyLabel"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.SetLabel("/dev/block/sda1", "MyLabel"), E_ERR);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_DoMount, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", 0), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", 0), E_NTFS_MOUNT);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", MS_RDONLY), E_OK);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_Check, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_OK));
    EXPECT_EQ(op.Check("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(0, E_OK));
    EXPECT_EQ(op.Check("/dev/block/sda1"), E_VOL_NEED_FIX);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_Repair, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Repair("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Repair("/dev/block/sda1"), E_VOL_FIX_FAILED);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_SetLabel, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/sda1", "MyLabel"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/sda1", "MyLabel"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.SetLabel("/dev/block/sda1", "MyLabel"), E_ERR);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.SetLabel("/dev/block/sda1", "MyLabel"), E_ERR);
}

HWTEST_F(ExtOperatorTest, VfatOperator_DoMount, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", 0), E_OK);
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", 0), E_FAT_MOUNT);
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/sda1", "/mnt/usb", MS_RDONLY), E_OK);
}

HWTEST_F(ExtOperatorTest, VfatOperator_Format, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Format("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_NO_CHILD));
    EXPECT_EQ(op.Format("/dev/block/sda1"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/sda1"), E_ERR);
}

} // namespace StorageDaemon
} // namespace OHOS
