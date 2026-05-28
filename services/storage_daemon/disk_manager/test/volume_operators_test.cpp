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
#include "disk_manager/volume/hmfs_operator.h"
#include "disk_manager/volume/ext4_operator.h"
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
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", 0, ""), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", 0, ""), E_EXFAT_MOUNT);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", MS_RDONLY, ""), E_OK);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_Format, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_NO_CHILD));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_ERR);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_Check, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_VOL_NEED_FIX);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_Repair, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_OK));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(0, E_OK));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_VOL_FIX_FAILED);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_ERR));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_VOL_FIX_FAILED);
}

HWTEST_F(ExtOperatorTest, ExfatOperator_SetLabel, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_ERR);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_DoMount, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", 0, ""), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", 0, ""), E_NTFS_MOUNT);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", MS_RDONLY, ""), E_OK);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_Check, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_OK));
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(0, E_OK));
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_VOL_NEED_FIX);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_Repair, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_VOL_FIX_FAILED);
}

HWTEST_F(ExtOperatorTest, NtfsOperator_SetLabel, TestSize.Level1)
{
    NtfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_ERR);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_ERR);
}

HWTEST_F(ExtOperatorTest, VfatOperator_DoMount, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", 0, ""), E_OK);
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", 0, ""), E_FAT_MOUNT);
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", MS_RDONLY, ""), E_OK);
}

HWTEST_F(ExtOperatorTest, VfatOperator_Format, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_NO_CHILD));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_ERR);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_DoMount_EmptyDevPath, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_EQ(op.DoMount("", "/mock/mnt/data", 0, ""), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_DoMount_EmptyMountPath, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "", 0, ""), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_DoMount_Success, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/data", 0, ""), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_DoMount_MountFailed, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/data", 0, ""), E_HMFS_MOUNT);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_DoMount_MigrationRO_Success, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, MS_RDONLY, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/data", 0x8000, ""), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_DoMount_MigrationRO_Failed, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, MS_RDONLY, _)).WillOnce(Return(-1));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/data", 0x8000, ""), E_HMFS_MOUNT);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Format_EmptyDevPath, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_EQ(op.Format(""), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Format_Success, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Format_ENoChild, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_NO_CHILD));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Format_Failed, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_ERR);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Check_EmptyDevPath, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_EQ(op.Check(""), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Check_Success, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Check_NeedFix, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_VOL_NEED_FIX);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Repair_EmptyDevPath, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_EQ(op.Repair(""), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Repair_Success, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_OK));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Repair_WrongExitStatus, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(0, E_OK));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_VOL_FIX_FAILED);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Repair_ForkExecFailed, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(1, E_ERR));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_VOL_FIX_FAILED);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_Repair_BothFailed, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(MakeForkExecWithExit(0, E_ERR));
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_VOL_FIX_FAILED);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_SetLabel_EmptyDevPath, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_EQ(op.SetLabel("", "MyLabel"), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_SetLabel_EmptyLabel, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", ""), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_SetLabel_Success, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_OK);
}

HWTEST_F(ExtOperatorTest, HmfsOperator_SetLabel_Failed, TestSize.Level1)
{
    HmfsOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "MyLabel"), E_ERR);
}

HWTEST_F(ExtOperatorTest, Ext4Operator_DoMount_EmptyDevPath, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_EQ(op.DoMount("", "/mock/mnt/data", 0, ""), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, Ext4Operator_DoMount_EmptyMountPath, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "", 0, ""), E_PARAMS_INVALID);
}

HWTEST_F(ExtOperatorTest, Ext4Operator_DoMount_Success, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/data", 0, ""), E_OK);
}

HWTEST_F(ExtOperatorTest, Ext4Operator_DoMount_MountFailed, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/data", 0, ""), E_EXT_MOUNT);
}

HWTEST_F(ExtOperatorTest, Ext4Operator_Format_Success, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
}

HWTEST_F(ExtOperatorTest, Ext4Operator_Format_ENoChild, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_NO_CHILD));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
}

HWTEST_F(ExtOperatorTest, Ext4Operator_Format_Failed, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_ERR);
}

/**
 * @tc.name: Ext4Operator_DoMount_ReadOnly
 * @tc.desc: Verify Ext4Operator::DoMount succeeds with MS_RDONLY flag.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, Ext4Operator_DoMount_ReadOnly, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/data", MS_RDONLY, ""), E_OK);
}

/**
 * @tc.name: VfatOperator_DoMount_ReadOnly
 * @tc.desc: Verify VfatOperator::DoMount succeeds with MS_RDONLY flag.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, VfatOperator_DoMount_ReadOnly, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(op.DoMount("/dev/block/mock_dev", "/mock/mnt/usb", MS_RDONLY, ""), E_OK);
}

/**
 * @tc.name: VfatOperator_Format_Failed
 * @tc.desc: Verify VfatOperator::Format returns error when ForkExec fails.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, VfatOperator_Format_Failed, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_ERR);
}

/**
 * @tc.name: VfatOperator_Check_BaseDefault
 * @tc.desc: Verify VfatOperator::Check returns E_OK from base class default implementation.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, VfatOperator_Check_BaseDefault, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_OK);
}

/**
 * @tc.name: VfatOperator_Repair_NotSupport
 * @tc.desc: Verify VfatOperator::Repair returns E_NOT_SUPPORT from base class default.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, VfatOperator_Repair_NotSupport, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_NOT_SUPPORT);
}

/**
 * @tc.name: VfatOperator_SetLabel_NotSupport
 * @tc.desc: Verify VfatOperator::SetLabel returns E_NOT_SUPPORT from base class default.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, VfatOperator_SetLabel_NotSupport, TestSize.Level1)
{
    VfatOperator op;
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "label"), E_NOT_SUPPORT);
}

/**
 * @tc.name: Ext4Operator_Check_BaseDefault
 * @tc.desc: Verify Ext4Operator::Check returns E_OK from base class default implementation.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, Ext4Operator_Check_BaseDefault, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_OK);
}

/**
 * @tc.name: Ext4Operator_Repair_NotSupport
 * @tc.desc: Verify Ext4Operator::Repair returns E_NOT_SUPPORT from base class default.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, Ext4Operator_Repair_NotSupport, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_EQ(op.Repair("/dev/block/mock_dev"), E_NOT_SUPPORT);
}

/**
 * @tc.name: Ext4Operator_SetLabel_NotSupport
 * @tc.desc: Verify Ext4Operator::SetLabel returns E_NOT_SUPPORT from base class default.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, Ext4Operator_SetLabel_NotSupport, TestSize.Level1)
{
    Ext4Operator op;
    EXPECT_EQ(op.SetLabel("/dev/block/mock_dev", "label"), E_NOT_SUPPORT);
}

/**
 * @tc.name: ExfatOperator_Format_Failed
 * @tc.desc: Verify ExfatOperator::Format returns error when ForkExec fails.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, ExfatOperator_Format_Failed, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_ERR);
}

/**
 * @tc.name: ExfatOperator_Format_NoChild
 * @tc.desc: Verify ExfatOperator::Format returns E_OK when ForkExec returns E_NO_CHILD.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(ExtOperatorTest, ExfatOperator_Format_NoChild, TestSize.Level1)
{
    ExfatOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_NO_CHILD));
    EXPECT_EQ(op.Format("/dev/block/mock_dev"), E_OK);
}

} // namespace StorageDaemon
} // namespace OHOS
