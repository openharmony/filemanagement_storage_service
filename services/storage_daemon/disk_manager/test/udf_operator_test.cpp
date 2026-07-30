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
#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "securec.h"
#include "disk_manager/volume/udf_operator.h"
#include "disk_manager/disk/disk_utils.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "mock/library_func_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

constexpr int32_t E_VERIFY_BURN_DATA_FAILED = 13600030;

int g_realpathRet = 0;
bool g_realpathOverride = false;
const char *g_realpathPath = "/dev/block/sr0";

extern "C" char *realpath(const char *path, char *resolvedPath)
{
    if (g_realpathRet != 0) {
        return nullptr;
    }
    if (resolvedPath == nullptr) {
        return nullptr;
    }
    const char *result = g_realpathOverride ? g_realpathPath : path;
    if (strcpy_s(resolvedPath, PATH_MAX, result) != 0) {
        return nullptr;
    }
    return resolvedPath;
}

class UdfOperatorTest : public testing::Test {
public:
    void SetUp() override
    {
        fileUtilMoc_ = std::make_shared<FileUtilMoc>();
        IFileUtilMoc::fileUtilMoc = fileUtilMoc_;
        diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
        IDiskUtilMoc::diskUtilMoc = diskUtilMoc_;
        libraryFuncMock_ = std::make_shared<LibraryFuncMock>();
        LibraryFunc::libraryFunc_ = libraryFuncMock_;
        g_realpathRet = 0;
        g_realpathOverride = false;
        g_realpathPath = "/dev/block/sr0";
    }
    void TearDown() override
    {
        g_realpathOverride = false;
        IFileUtilMoc::fileUtilMoc = nullptr;
        fileUtilMoc_ = nullptr;
        IDiskUtilMoc::diskUtilMoc = nullptr;
        diskUtilMoc_ = nullptr;
        LibraryFunc::libraryFunc_ = nullptr;
        libraryFuncMock_ = nullptr;
    }
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<LibraryFuncMock> libraryFuncMock_ = nullptr;
};

HWTEST_F(UdfOperatorTest, UdfOperator_DoMount_EmptyDisc, TestSize.Level1)
{
    UdfOperator op;
    int32_t status = 3;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(status), Return(E_OK)));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoMount_QueryFailedProceed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoMount_ForkExecFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_UDF_MOUNT);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoMount_ForkExecSuccess, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ReadMetadata_EmptyDevPath, TestSize.Level1)
{
    UdfOperator op;
    std::string uuid, type, label;
    EXPECT_EQ(op.ReadMetadata("", uuid, type, label), E_PARAMS_INVALID);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ReadMetadata_PathTooLong, TestSize.Level1)
{
    UdfOperator op;
    std::string uuid, type, label;
    std::string longPath(PATH_MAX, 'a');
    EXPECT_EQ(op.ReadMetadata(longPath, uuid, type, label), E_PARAMS_INVALID);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ReadMetadata_RealpathFailed, TestSize.Level1)
{
    UdfOperator op;
    std::string uuid, type, label;
    g_realpathRet = -1;
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_PARAMS_INVALID);
    g_realpathRet = 0;
}

HWTEST_F(UdfOperatorTest, UdfOperator_ReadMetadata_InvalidPrefix, TestSize.Level1)
{
    UdfOperator op;
    std::string uuid, type, label;
    g_realpathOverride = true;
    g_realpathPath = "/invalid/path";
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_PARAMS_INVALID);
    g_realpathOverride = false;
    g_realpathPath = "/dev/block/sr0";
}

HWTEST_F(UdfOperatorTest, UdfOperator_ReadMetadata_Success, TestSize.Level1)
{
    UdfOperator op;
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("udf-uuid")).WillOnce(Return("UDFLabel"));
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_OK);
    EXPECT_EQ(uuid, "udf-uuid");
    EXPECT_EQ(label, "UDFLabel");
}

HWTEST_F(UdfOperatorTest, UdfOperator_ReadMetadata_UuidEmpty, TestSize.Level1)
{
    UdfOperator op;
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("")).WillOnce(Return("Lbl"));
    EXPECT_CALL(*diskUtilMoc_, GenerateRandomUuid(_, _)).WillOnce(Return("rand-uuid"));
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_OK);
    EXPECT_EQ(uuid, "rand-uuid");
}

HWTEST_F(UdfOperatorTest, UdfOperator_ReadMetadata_LabelEmpty, TestSize.Level1)
{
    UdfOperator op;
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("uuid")).WillOnce(Return(""));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("CDROM"));
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_OK);
    EXPECT_EQ(label, "CDROM");
}

HWTEST_F(UdfOperatorTest, UdfOperator_CreateIsoImage_ForkExecFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.CreateIsoImage("/dev/sr0", "/tmp/image.iso", "/mnt/cd"), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_CreateIsoImage_Success, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.CreateIsoImage("/dev/sr0", "/tmp/image.iso", "/mnt/cd"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_CreateIsoImage_CleanTempFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.CreateIsoImage("/dev/sr0", "/tmp/image.iso", "/mnt/cd"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareIsoImage_MkDirFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareIsoImage_IsIsoImage, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareIsoImage_DiskEmptyGenSuccess, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareIsoImage_DiskNotEmptyGenSuccess, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, false, "0,0"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareIsoImage_GenFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoCDBurn_PrepareFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoCDBurn("/dev/sr0", opts, true, ""), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoCDBurn_NotIsoImageDiskEmpty, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoCDBurn("/dev/sr0", opts, true, ""), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoCDBurn_NotIsoImageDiskNotEmpty, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoCDBurn("/dev/sr0", opts, false, "0,0"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoCDBurn_IsIsoImage, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoCDBurn("/dev/sr0", opts, true, ""), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoCDBurn_WodimFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_EQ(op.DoCDBurn("/dev/sr0", opts, true, ""), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoDVDBurn_NotIsoImageDiskEmpty, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, true), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoDVDBurn_NotIsoImageDiskNotEmpty, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, false), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoDVDBurn_IsIsoImage, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, true), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoDVDBurn_ForkExecFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, true), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_Burn_BlankCD_CDType_VerifyFalse, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isVerifyBurn = false;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    bool blank = true;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_, _)).WillOnce(DoAll(SetArgReferee<1>(blank), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("CDROM"));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, EjectCD(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_Burn_NotBlankCD_GetIncBurnAddrFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    bool blank = false;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_, _)).WillOnce(DoAll(SetArgReferee<1>(blank), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("CDROM"));
    EXPECT_CALL(*diskUtilMoc_, GetIncBurnAddr(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_Burn_DVDType_Success, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isVerifyBurn = false;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    bool blank = true;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_, _)).WillOnce(DoAll(SetArgReferee<1>(blank), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVDROM"));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, EjectCD(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_Burn_VerifyTrueFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isVerifyBurn = true;
    opts.isIsoImage = true;
    opts.burnPath = "/data/burn";
    opts.burnSpeed = "1";
    bool blank = true;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_, _)).WillOnce(DoAll(SetArgReferee<1>(blank), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVDROM"));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*diskUtilMoc_, EjectCD(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_VERIFY_BURN_DATA_FAILED);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareVerifyMountPath_MkDirFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareVerifyMountPath(), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareVerifyMountPath_DirExists, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareVerifyMountPath(), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareVerifyMountPath_Success, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareVerifyMountPath(), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ExecuteIsoInfoList_ForkExecFailed, TestSize.Level1)
{
    UdfOperator op;
    std::vector<std::string> merged;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ExecuteIsoInfoList("/dev/sr0", merged), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ExecuteIsoInfoList_Success, TestSize.Level1)
{
    UdfOperator op;
    std::vector<std::string> merged;
    std::vector<std::string> rawOutput = {"raw line"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>{"line1"}));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(std::vector<std::string>{"merged1"}));
    EXPECT_EQ(op.ExecuteIsoInfoList("/dev/sr0", merged), E_OK);
    EXPECT_EQ(merged.size(), 1u);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ProcessMergedLine_DirectoryListing, TestSize.Level1)
{
    UdfOperator op;
    std::string currentPath;
    EXPECT_CALL(*diskUtilMoc_, ParseDirectoryPath(_)).WillOnce(Return("/root"));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "Directory listing of /root", currentPath), E_OK);
    EXPECT_EQ(currentPath, "/root");
}

HWTEST_F(UdfOperatorTest, UdfOperator_ProcessMergedLine_NotFileEntry, TestSize.Level1)
{
    UdfOperator op;
    std::string currentPath;
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(Return(false));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "some line", currentPath), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ProcessMergedLine_EmptyName, TestSize.Level1)
{
    UdfOperator op;
    std::string currentPath;
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return(""));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  file line", currentPath), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ProcessMergedLine_DirEntry, TestSize.Level1)
{
    UdfOperator op;
    std::string currentPath;
    char et = 'd';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("mydir"));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  d mydir", currentPath), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ProcessMergedLine_FileExtractSuccess, TestSize.Level1)
{
    UdfOperator op;
    std::string currentPath = "/root";
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("myfile"));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  f myfile", currentPath), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ProcessMergedLine_IntermediateMkDirFailed, TestSize.Level1)
{
    UdfOperator op;
    std::string currentPath = "/root";
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("myfile"));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  f myfile", currentPath), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ProcessMergedLine_ExtractFailed, TestSize.Level1)
{
    UdfOperator op;
    std::string currentPath = "/root";
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("myfile"));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  f myfile", currentPath), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ExtractIsoFiles_InvalidIsoPath, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ExtractIsoFiles_ShellMetachar, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0|evil", "/data/local/tmp"), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ExtractIsoFiles_ExecuteFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_ExtractIsoFiles_Success, TestSize.Level1)
{
    UdfOperator op;
    std::vector<std::string> mergedLines;
    std::vector<std::string> rawOutput = {"raw"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(mergedLines));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareSourceDirectory_IsIsoImage, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    std::vector<std::string> rawOutput = {"raw"};
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareSourceDirectory_MkDirFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_PrepareSourceDirectory_NotIsoImage, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn_dir";
    std::string sourceDir;
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_OK);
    EXPECT_EQ(sourceDir, "/data/burn_dir");
}

HWTEST_F(UdfOperatorTest, UdfOperator_GenerateAndCompareChecksums_SourceGenFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_GenerateAndCompareChecksums_DiscGenFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_GenerateAndCompareChecksums_UnmountFailed, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_GenerateAndCompareChecksums_SourceChecksumEmpty, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_GenerateAndCompareChecksums_DiscChecksumEmpty, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_GenerateAndCompareChecksums_Success, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoVerifyBurnData_PrepareMountPathFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoVerifyBurnData("/dev/block/sr0", opts, true), E_ERR);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoVerifyBurnData_MountFailed, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillRepeatedly(Return(true));
    EXPECT_NE(op.DoVerifyBurnData("/dev/block/sr0", opts, true), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_DoVerifyBurnData_Success, TestSize.Level1)
{
    UdfOperator op;
    BurnOptions opts;
    opts.isIsoImage = false;
    opts.burnPath = "/data/burn";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillRepeatedly(Return(true));
    EXPECT_NE(op.DoVerifyBurnData("/dev/block/sr0", opts, true), E_OK);
}

HWTEST_F(UdfOperatorTest, UdfOperator_Check_BaseDefault, TestSize.Level1)
{
    UdfOperator op;
    EXPECT_EQ(op.Check("/dev/block/mock_dev"), E_OK);
}
} // namespace StorageDaemon
} // namespace OHOS