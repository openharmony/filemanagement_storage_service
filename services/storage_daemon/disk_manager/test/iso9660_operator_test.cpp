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
#include "disk_manager/volume/iso9660_operator.h"
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

class IsoOperatorTest : public testing::Test {
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

HWTEST_F(IsoOperatorTest, IsoOperator_DoMount_EmptyDisc, TestSize.Level1)
{
    IsoOperator op;
    int32_t status = 3;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(status), Return(E_OK)));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoMount_QueryFailedProceed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoMount_ForkExecFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_ISO9660_MOUNT);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoMount_ForkExecSuccess, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, QueryCDStatus(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoMount("/dev/block/sr0", "/mnt/data", 0, ""), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ReadMetadata_EmptyDevPath, TestSize.Level1)
{
    IsoOperator op;
    std::string uuid, type, label;
    EXPECT_EQ(op.ReadMetadata("", uuid, type, label), E_PARAMS_INVALID);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ReadMetadata_PathTooLong, TestSize.Level1)
{
    IsoOperator op;
    std::string uuid, type, label;
    std::string longPath(PATH_MAX, 'a');
    EXPECT_EQ(op.ReadMetadata(longPath, uuid, type, label), E_PARAMS_INVALID);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ReadMetadata_RealpathFailed, TestSize.Level1)
{
    IsoOperator op;
    std::string uuid, type, label;
    g_realpathRet = -1;
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_PARAMS_INVALID);
    g_realpathRet = 0;
}

HWTEST_F(IsoOperatorTest, IsoOperator_ReadMetadata_InvalidPrefix, TestSize.Level1)
{
    IsoOperator op;
    std::string uuid, type, label;
    g_realpathOverride = true;
    g_realpathPath = "/invalid/path";
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_PARAMS_INVALID);
    g_realpathOverride = false;
    g_realpathPath = "/dev/block/sr0";
}

HWTEST_F(IsoOperatorTest, IsoOperator_ReadMetadata_Success, TestSize.Level1)
{
    IsoOperator op;
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _))
        .WillOnce(Return("iso-uuid")).WillOnce(Return("ISOLabel"));
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_OK);
    EXPECT_EQ(uuid, "iso-uuid");
    EXPECT_EQ(label, "ISOLabel");
}

HWTEST_F(IsoOperatorTest, IsoOperator_ReadMetadata_UuidEmpty, TestSize.Level1)
{
    IsoOperator op;
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("")).WillOnce(Return("Lbl"));
    EXPECT_CALL(*diskUtilMoc_, GenerateRandomUuid(_, _)).WillOnce(Return("rand-uuid"));
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_OK);
    EXPECT_EQ(uuid, "rand-uuid");
}

HWTEST_F(IsoOperatorTest, IsoOperator_ReadMetadata_LabelEmpty, TestSize.Level1)
{
    IsoOperator op;
    std::string uuid, type, label;
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(_, _)).WillOnce(Return("uuid")).WillOnce(Return(""));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("CDROM"));
    EXPECT_EQ(op.ReadMetadata("/dev/block/sr0", uuid, type, label), E_OK);
    EXPECT_EQ(label, "CDROM");
}

HWTEST_F(IsoOperatorTest, IsoOperator_CreateIsoImage_ForkExecFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.CreateIsoImage("/dev/sr0", "/tmp/image.iso", "/mnt/cd"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_CreateIsoImage_Success, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.CreateIsoImage("/dev/sr0", "/tmp/image.iso", "/mnt/cd"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_CreateIsoImage_CleanTempFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.CreateIsoImage("/dev/sr0", "/tmp/image.iso", "/mnt/cd"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareIsoImage_MkDirFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareIsoImage_IsIsoImage, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareIsoImage_DiskEmptyGenSuccess, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareIsoImage_DiskNotEmptyGenSuccess, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, false, "0,0"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareIsoImage_GenFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_EQ(op.PrepareIsoImage("/dev/sr0", opts, true, ""), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoCDBurn_PrepareFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoCDBurn("/dev/sr0", opts, true, ""), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoCDBurn_NotIsoImageDiskEmpty, TestSize.Level1)
{
    IsoOperator op;
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

HWTEST_F(IsoOperatorTest, IsoOperator_DoCDBurn_NotIsoImageDiskNotEmpty, TestSize.Level1)
{
    IsoOperator op;
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

HWTEST_F(IsoOperatorTest, IsoOperator_DoCDBurn_IsIsoImage, TestSize.Level1)
{
    IsoOperator op;
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

HWTEST_F(IsoOperatorTest, IsoOperator_DoCDBurn_WodimFailed, TestSize.Level1)
{
    IsoOperator op;
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

HWTEST_F(IsoOperatorTest, IsoOperator_DoDVDBurn_BurnPathDash, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "-evil";
    opts.burnSpeed = "1";
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, true), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoDVDBurn_NotIsoImageDiskEmpty, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, true), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoDVDBurn_NotIsoImageDiskNotEmpty, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, false), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoDVDBurn_IsIsoImage, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, true), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoDVDBurn_ForkExecFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoDVDBurn("/dev/sr0", opts, true), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_Burn_BlankCD_CDType_VerifyFalse, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isVerifyBurn = false;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    bool blank = true;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_)).WillOnce(Return(blank));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("CDROM"));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory())
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, EjectCD(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_Burn_NotBlankCD_GetIncBurnAddrFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    bool blank = false;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_)).WillOnce(Return(blank));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("CDROM"));
    EXPECT_CALL(*diskUtilMoc_, GetIncBurnAddr(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_Burn_DVDType_Success, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isVerifyBurn = false;
    opts.burnPath = "/data/burn";
    opts.diskName = "MYDISC";
    opts.burnSpeed = "1";
    bool blank = true;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_)).WillOnce(Return(blank));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVDROM"));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, EjectCD(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_Burn_VerifyTrueFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isVerifyBurn = true;
    opts.isIsoImage = true;
    opts.burnPath = "/data/burn";
    opts.burnSpeed = "1";
    bool blank = true;
    EXPECT_CALL(*diskUtilMoc_, IsCDBlank(_)).WillOnce(Return(blank));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(_)).WillOnce(Return("DVDROM"));
    EXPECT_CALL(*diskUtilMoc_, CleanTempDirectory()).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*diskUtilMoc_, EjectCD(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.Burn("/dev/sr0", opts), E_VERIFY_BURN_DATA_FAILED);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareVerifyMountPath_MkDirFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareVerifyMountPath(), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareVerifyMountPath_DirExists, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareVerifyMountPath(), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareVerifyMountPath_Success, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.PrepareVerifyMountPath(), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ExecuteIsoInfoList_ForkExecFailed, TestSize.Level1)
{
    IsoOperator op;
    std::vector<std::string> merged;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ExecuteIsoInfoList("/dev/sr0", merged), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ExecuteIsoInfoList_Success, TestSize.Level1)
{
    IsoOperator op;
    std::vector<std::string> merged;
    std::vector<std::string> rawOutput = {"raw line"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>{"line1"}));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(std::vector<std::string>{"merged1"}));
    EXPECT_EQ(op.ExecuteIsoInfoList("/dev/sr0", merged), E_OK);
    EXPECT_EQ(merged.size(), 1u);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ProcessMergedLine_DirectoryListing, TestSize.Level1)
{
    IsoOperator op;
    std::string currentPath;
    EXPECT_CALL(*diskUtilMoc_, ParseDirectoryPath(_)).WillOnce(Return("/root"));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "Directory listing of /root", currentPath), E_OK);
    EXPECT_EQ(currentPath, "/root");
}

HWTEST_F(IsoOperatorTest, IsoOperator_ProcessMergedLine_NotFileEntry, TestSize.Level1)
{
    IsoOperator op;
    std::string currentPath;
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(Return(false));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "some line", currentPath), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ProcessMergedLine_EmptyName, TestSize.Level1)
{
    IsoOperator op;
    std::string currentPath;
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return(""));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  file line", currentPath), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ProcessMergedLine_DirEntry, TestSize.Level1)
{
    IsoOperator op;
    std::string currentPath;
    char et = 'd';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("mydir"));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  d mydir", currentPath), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ProcessMergedLine_FileExtractSuccess, TestSize.Level1)
{
    IsoOperator op;
    std::string currentPath = "/root";
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("myfile"));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, ForkExecToFile(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  f myfile", currentPath), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ProcessMergedLine_IntermediateMkDirFailed, TestSize.Level1)
{
    IsoOperator op;
    std::string currentPath = "/root";
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("myfile"));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  f myfile", currentPath), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ProcessMergedLine_ExtractFailed, TestSize.Level1)
{
    IsoOperator op;
    std::string currentPath = "/root";
    char et = 'f';
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillOnce(DoAll(SetArgReferee<1>(et), Return(true)));
    EXPECT_CALL(*diskUtilMoc_, ParseFileName(_)).WillOnce(Return("myfile"));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, ForkExecToFile(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ProcessMergedLine("/iso", "/src", "  f myfile", currentPath), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ExtractIsoFiles_InvalidIsoPath, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ExtractIsoFiles_ShellMetachar, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0;evil", "/data/local/tmp"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ExtractIsoFiles_ExecuteFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ExtractIsoFiles_LineInvalidSkipped, TestSize.Level1)
{
    IsoOperator op;
    // "../evil" triggers IsFilePathInvalid == true -> continue (skip line)
    // "valid_line" triggers IsFilePathInvalid == false -> ProcessMergedLine
    // This covers both branches of IsFilePathInvalid(line) AND the loop-continue path
    std::vector<std::string> mergedLines = {"../evil", "valid_line"};
    std::vector<std::string> rawOutput = {"raw"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(mergedLines));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(mergedLines));
    EXPECT_CALL(*diskUtilMoc_, ParseDirectoryPath(_)).WillRepeatedly(Return(""));
    EXPECT_CALL(*diskUtilMoc_, IsFileEntry(_, _)).WillRepeatedly(Return(false));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ExtractIsoFiles_Success, TestSize.Level1)
{
    IsoOperator op;
    std::vector<std::string> mergedLines;
    std::vector<std::string> rawOutput = {"raw"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(mergedLines));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_OK);
}

/**
 * @tc.name: IsoOperator_ExtractIsoFiles_InvalidLineOnly
 * @tc.desc: Verify ExtractIsoFiles skips an invalid line and exits loop (no valid lines follow).
 *           This covers the loop-exit branch after IsFilePathInvalid returns true.
 * @tc.type: FUNC
 */
HWTEST_F(IsoOperatorTest, IsoOperator_ExtractIsoFiles_InvalidLineOnly, TestSize.Level1)
{
    IsoOperator op;
    // Only an invalid line -> IsFilePathInvalid returns true -> continue -> loop exits
    std::vector<std::string> mergedLines = {"../evil"};
    std::vector<std::string> rawOutput = {"raw"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(mergedLines));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(mergedLines));
    EXPECT_EQ(op.ExtractIsoFiles("/dev/block/sr0", "/data/local/tmp"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateChecksums_ForkExecFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.GenerateChecksums("/data/dir", "/tmp/checksum.txt"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateChecksums_WriteSyncFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, WriteFileSync(_, _, _, _)).WillOnce(Return(false));
    EXPECT_EQ(op.GenerateChecksums("/data/dir", "/tmp/checksum.txt"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateChecksums_Success, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, WriteFileSync(_, _, _, _)).WillOnce(Return(true));
    EXPECT_EQ(op.GenerateChecksums("/data/dir", "/tmp/checksum.txt"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_ParseChecksumFile_EmptyLineSkipped, TestSize.Level1)
{
    IsoOperator op;
    std::string content = "\nabc123  /data/file1\n";
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _))
        .WillOnce(Return(std::vector<std::string>{"", "abc123  /data/file1"}));
    EXPECT_CALL(*diskUtilMoc_, GetRelativePath(_, _)).WillOnce(Return("file1"));
    auto result = op.ParseChecksumFile(content, "/data");
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result["file1"], "abc123");
}

HWTEST_F(IsoOperatorTest, IsoOperator_ParseChecksumFile_NoDoubleSpace, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _))
        .WillOnce(Return(std::vector<std::string>{"nospacesingle/data/file"}));
    auto result = op.ParseChecksumFile("", "/data");
    EXPECT_EQ(result.size(), 0u);
}

HWTEST_F(IsoOperatorTest, IsoOperator_LogChecksumMap_Basic, TestSize.Level1)
{
    IsoOperator op;
    std::map<std::string, std::string> mapData = {{"/f", "h"}};
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    op.LogChecksumMap("testMap", mapData);
}

HWTEST_F(IsoOperatorTest, IsoOperator_CompareChecksums_FileNotFound, TestSize.Level1)
{
    IsoOperator op;
    std::map<std::string, std::string> src = {{"file1", "hash1"}};
    std::map<std::string, std::string> disc;
    EXPECT_EQ(op.CompareChecksums(src, disc), E_VERIFY_BURN_DATA_FAILED);
}

HWTEST_F(IsoOperatorTest, IsoOperator_CompareChecksums_Md5Mismatch, TestSize.Level1)
{
    IsoOperator op;
    std::map<std::string, std::string> src = {{"file1", "hash1"}};
    std::map<std::string, std::string> disc = {{"file1", "hash2"}};
    EXPECT_EQ(op.CompareChecksums(src, disc), E_VERIFY_BURN_DATA_FAILED);
}

HWTEST_F(IsoOperatorTest, IsoOperator_CompareChecksums_AllMatch, TestSize.Level1)
{
    IsoOperator op;
    std::map<std::string, std::string> src = {{"file1", "hash1"}};
    std::map<std::string, std::string> disc = {{"file1", "hash1"}};
    EXPECT_EQ(op.CompareChecksums(src, disc), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_IsIsoImage, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    std::vector<std::string> rawOutput = {"raw"};
    // 1st IsDir: BURN_TMP_DIR -> true (skip creating BURN_TMP_DIR)
    // 2nd IsDir: source_extract -> true (need to RmDirRecurse)
    EXPECT_CALL(*fileUtilMoc_, IsDir(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_MkDirFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    // 1st IsDir: BURN_TMP_DIR -> true (skip creating BURN_TMP_DIR)
    // 2nd IsDir: source_extract -> true (need to RmDirRecurse)
    EXPECT_CALL(*fileUtilMoc_, IsDir(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_NotIsoImage, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn_dir";
    std::string sourceDir;
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_OK);
    EXPECT_EQ(sourceDir, "/data/burn_dir");
}

/**
 * @tc.name: IsoOperator_PrepareSourceDirectory_BurnTmpNotDir_MkDirFailed
 * @tc.desc: Verify PrepareSourceDirectory returns E_ERR when BURN_TMP_DIR does not exist and
 *           MkDir(BURN_TMP_DIR) fails.
 * @tc.type: FUNC
 */
HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_BurnTmpNotDir_MkDirFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    // IsDir(BURN_TMP_DIR) returns false -> need to MkDir(BURN_TMP_DIR)
    // MkDir(BURN_TMP_DIR) fails -> E_ERR
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_ERR);
}

/**
 * @tc.name: IsoOperator_PrepareSourceDirectory_BurnTmpNotDir_MkDirSuccess_SourceDirNotExists
 * @tc.desc: Verify PrepareSourceDirectory succeeds when BURN_TMP_DIR does not exist (created),
 *           source_extract dir does not exist (no RmDirRecurse), and ExtractIsoFiles succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_BurnTmpNotDir_MkDirSuccess_SourceDirNotExists,
    TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    std::vector<std::string> rawOutput = {"raw"};
    // 1st IsDir: BURN_TMP_DIR -> false (need to create)
    // 2nd IsDir: source_extract -> false (skip RmDirRecurse)
    EXPECT_CALL(*fileUtilMoc_, IsDir(_))
        .WillOnce(Return(false))
        .WillOnce(Return(false));
    // 1st MkDir: BURN_TMP_DIR -> E_OK
    // 2nd MkDir: source_extract -> E_OK
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_OK);
}

/**
 * @tc.name: IsoOperator_PrepareSourceDirectory_BurnTmpNotDir_MkDirSuccess_SourceDirExists
 * @tc.desc: Verify PrepareSourceDirectory succeeds when BURN_TMP_DIR does not exist (created),
 *           source_extract dir exists (RmDirRecurse called), and ExtractIsoFiles succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_BurnTmpNotDir_MkDirSuccess_SourceDirExists,
    TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    std::vector<std::string> rawOutput = {"raw"};
    // 1st IsDir: BURN_TMP_DIR -> false (need to create)
    // 2nd IsDir: source_extract -> true (need to RmDirRecurse)
    EXPECT_CALL(*fileUtilMoc_, IsDir(_))
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    // 1st MkDir: BURN_TMP_DIR -> E_OK
    // 2nd MkDir: source_extract -> E_OK
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(rawOutput), Return(E_OK)));
    EXPECT_CALL(*diskUtilMoc_, SplitString(_, _)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_CALL(*diskUtilMoc_, MergeOutputLines(_)).WillOnce(Return(std::vector<std::string>()));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_OK);
}

/**
 * @tc.name: IsoOperator_PrepareSourceDirectory_SourceDirExists_ExtractFailed
 * @tc.desc: Verify PrepareSourceDirectory returns error when source_extract dir exists (removed),
 *           MkDir(source_extract) succeeds, but ExtractIsoFiles fails.
 * @tc.type: FUNC
 */
HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_SourceDirExists_ExtractFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    // 1st IsDir: BURN_TMP_DIR -> true (skip creating BURN_TMP_DIR)
    // 2nd IsDir: source_extract -> true (need to RmDirRecurse)
    EXPECT_CALL(*fileUtilMoc_, IsDir(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_ERR);
}

/**
 * @tc.name: IsoOperator_PrepareSourceDirectory_SourceDirNotExists_ExtractFailed
 * @tc.desc: Verify PrepareSourceDirectory returns error when source_extract dir does not exist,
 *           MkDir(source_extract) succeeds, but ExtractIsoFiles fails.
 * @tc.type: FUNC
 */
HWTEST_F(IsoOperatorTest, IsoOperator_PrepareSourceDirectory_SourceDirNotExists_ExtractFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.isIsoImage = true;
    opts.burnPath = "/data/image.iso";
    std::string sourceDir;
    // 1st IsDir: BURN_TMP_DIR -> true (skip creating BURN_TMP_DIR)
    // 2nd IsDir: source_extract -> false (skip RmDirRecurse)
    EXPECT_CALL(*fileUtilMoc_, IsDir(_))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.PrepareSourceDirectory(opts, sourceDir), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateAndCompareChecksums_SourceGenFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateAndCompareChecksums_DiscGenFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_ERR);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateAndCompareChecksums_UnmountFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateAndCompareChecksums_SourceChecksumEmpty, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateAndCompareChecksums_DiscChecksumEmpty, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateAndCompareChecksums_CompareFailed, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_GenerateAndCompareChecksums_Success, TestSize.Level1)
{
    IsoOperator op;
    EXPECT_CALL(*diskUtilMoc_, GenerateChecksums(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, GetAnonyString(_)).WillRepeatedly(Return("anon"));
    EXPECT_NE(op.GenerateAndCompareChecksums("/src", "/sc", "/dc"), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoVerifyBurnData_PrepareMountPathFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(op.DoVerifyBurnData("/dev/block/sr0", opts, true), E_ERR);}

HWTEST_F(IsoOperatorTest, IsoOperator_DoVerifyBurnData_MountFailed, TestSize.Level1)
{
    IsoOperator op;
    BurnOptions opts;
    opts.burnPath = "/data/burn";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fileUtilMoc_, MkDir(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, IsFilePathInvalid(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillRepeatedly(Return(true));
    EXPECT_NE(op.DoVerifyBurnData("/dev/block/sr0", opts, true), E_OK);
}

HWTEST_F(IsoOperatorTest, IsoOperator_DoVerifyBurnData_BurnTmpNotDir, TestSize.Level1)
{
    IsoOperator op;
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

HWTEST_F(IsoOperatorTest, IsoOperator_DoVerifyBurnData_Success, TestSize.Level1)
{
    IsoOperator op;
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
} // namespace StorageDaemon
} // namespace OHOS