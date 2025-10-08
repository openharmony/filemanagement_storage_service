/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <linux/kdev_t.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/disk_utils.h"
#include "volume/external_volume_info.h"

namespace OHOS {
namespace StorageDaemon {
int32_t g_readMetadata = 0;
int g_extStorageMountForkExec = 0;
int g_forkExec = 0;
int g_forkExecWithExit = 0;
int g_exitStatus = 0;
int32_t ReadMetadata(const std::string &devPath, std::string &uuid, std::string &type, std::string &label)
{
    return g_readMetadata;
}

int ForkExecWithExit(std::vector<std::string> &cmd, int *exitStatus)
{
    if (exitStatus != nullptr) {
        *exitStatus = g_exitStatus;
    }
    return g_forkExecWithExit;
}

int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output, int *exitStatus)
{
    if (exitStatus != nullptr) {
        *exitStatus = g_exitStatus;
    }
    return g_forkExec;
}

int ExtStorageMountForkExec(std::vector<std::string> &cmd, int *exitStatus)
{
    if (exitStatus != nullptr) {
        *exitStatus = g_exitStatus;
    }
    return g_extStorageMountForkExec;
}
} // STORAGE_DAEMON
} // OHOS

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class ExternalVolumeInfoOtherTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    ExternalVolumeInfo* externalVolumeInfo_;
};

void ExternalVolumeInfoOtherTest::SetUp()
{
    externalVolumeInfo_ = new ExternalVolumeInfo();
}

void ExternalVolumeInfoOtherTest::TearDown(void)
{
    if (externalVolumeInfo_ != nullptr) {
        delete externalVolumeInfo_;
        externalVolumeInfo_ = nullptr;
    }
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoTryToCheck_001
* @tc.desc: Verify the DoTryToCheck function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoTryToCheck_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoTryToCheck_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);

    g_readMetadata = 1;
    externalVolumeInfo_->fsLabel_ = "test";
    EXPECT_EQ(externalVolumeInfo_->DoTryToCheck(), E_DOCHECK_MOUNT);

    g_readMetadata = 0;
    externalVolumeInfo_->fsType_ = "f2fs";
    EXPECT_EQ(externalVolumeInfo_->DoTryToCheck(), E_VOL_FIX_NOT_SUPPORT);

    externalVolumeInfo_->fsType_ = "ntfs";
    g_forkExecWithExit = 0;
    g_exitStatus = 1;
    EXPECT_EQ(externalVolumeInfo_->DoTryToCheck(), E_OK);

    externalVolumeInfo_->fsType_ = "exfat";
    g_forkExecWithExit = 1;
    g_exitStatus = 0;
    EXPECT_EQ(externalVolumeInfo_->DoTryToCheck(), E_VOL_NEED_FIX);

    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoTryToCheck_001 end";
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoTryToFix_001
* @tc.desc: Verify the DoTryToFix function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoTryToFix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoTryToFix_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);

    g_readMetadata = 1;
    externalVolumeInfo_->fsLabel_ = "test";
    EXPECT_EQ(externalVolumeInfo_->DoTryToFix(), E_DOCHECK_MOUNT);

    g_readMetadata = 0;
    externalVolumeInfo_->fsType_ = "f2fs";
    EXPECT_EQ(externalVolumeInfo_->DoTryToFix(), E_VOL_FIX_NOT_SUPPORT);

    externalVolumeInfo_->fsType_ = "ntfs";
    g_extStorageMountForkExec = 0;
    g_forkExec = 0;
    g_exitStatus = 1;
    EXPECT_EQ(externalVolumeInfo_->DoTryToFix(), E_OK);

    externalVolumeInfo_->fsType_ = "exfat";
    g_extStorageMountForkExec = 1;
    g_forkExec = 1;
    g_exitStatus = 0;
    EXPECT_EQ(externalVolumeInfo_->DoTryToFix(), E_VOL_FIX_FAILED);

    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoTryToFix_001 end";
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoCheck4Exfat_001
* @tc.desc: Verify the DoCheck4Exfat function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoCheck4Exfat_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoCheck4Exfat_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);

    g_forkExecWithExit = 1;
    EXPECT_EQ(externalVolumeInfo_->DoCheck4Exfat(), E_VOL_NEED_FIX);

    g_forkExecWithExit = 0;
    EXPECT_EQ(externalVolumeInfo_->DoCheck4Exfat(), E_OK);

    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoCheck4Exfat_001 end";
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoCheck4Ntfs_001
* @tc.desc: Verify the DoCheck4Ntfs function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoCheck4Ntfs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoCheck4Ntfs_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);

    g_forkExecWithExit = 1;
    g_exitStatus = 0;
    EXPECT_EQ(externalVolumeInfo_->DoCheck4Ntfs(), E_VOL_NEED_FIX);

    g_forkExecWithExit = 0;
    g_exitStatus = 1;
    EXPECT_EQ(externalVolumeInfo_->DoCheck4Ntfs(), E_OK);

    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoCheck4Ntfs_001 end";
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoFix4Ntfs_001
* @tc.desc: Verify the DoFix4Ntfs function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoFix4Ntfs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoFix4Ntfs_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);

    g_extStorageMountForkExec = 1;
    g_forkExec = 1;
    EXPECT_EQ(externalVolumeInfo_->DoFix4Ntfs(), E_VOL_FIX_FAILED);

    g_extStorageMountForkExec = 0;
    g_forkExec = 0;
    EXPECT_EQ(externalVolumeInfo_->DoFix4Ntfs(), E_OK);
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoFix4Ntfs_001 end";
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoFix4Exfat_001
* @tc.desc: Verify the DoFix4Exfat function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoFix4Exfat_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoFix4Exfat_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);

    g_extStorageMountForkExec = 1;
    g_forkExec = 1;
    g_exitStatus = 0;
    EXPECT_EQ(externalVolumeInfo_->DoFix4Exfat(), E_VOL_FIX_FAILED);

    g_extStorageMountForkExec = 0;
    g_forkExec = 0;
    g_exitStatus = 1;
    EXPECT_EQ(externalVolumeInfo_->DoFix4Exfat(), E_OK);
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoFix4Exfat_001 end";
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoMount4Exfat_001
* @tc.desc: Verify the DoMount4Exfat function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoMount4Exfat_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoMount4Exfat_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);
    uint32_t mountFlags = 0;
    g_extStorageMountForkExec = 1;
    g_forkExec = 1;
    EXPECT_EQ(externalVolumeInfo_->DoMount4Exfat(mountFlags), E_EXFAT_MOUNT);

    g_extStorageMountForkExec = 0;
    g_forkExec = 0;
    EXPECT_EQ(externalVolumeInfo_->DoMount4Exfat(mountFlags), E_OK);
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoMount4Exfat_001 end";
}

/**
* @tc.name: ExternalVolumeInfoOtherTest_DoMount4Ntfs_001
* @tc.desc: Verify the DoMount4Ntfs function.
* @tc.type: FUNC
* @tc.require: AR000H09L6
*/
HWTEST_F(ExternalVolumeInfoOtherTest, ExternalVolumeInfoOtherTest_DoMount4Ntfs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoMount4Ntfs_001 start";
    ASSERT_TRUE(externalVolumeInfo_ != nullptr);
    uint32_t mountFlags = 0;
    g_extStorageMountForkExec = 1;
    g_forkExec = 1;
    EXPECT_EQ(externalVolumeInfo_->DoMount4Ntfs(mountFlags), E_NTFS_MOUNT);

    g_extStorageMountForkExec = 0;
    g_forkExec = 0;
    EXPECT_EQ(externalVolumeInfo_->DoMount4Ntfs(mountFlags), E_OK);
    GTEST_LOG_(INFO) << "ExternalVolumeInfoOtherTest_DoMount4Ntfs_001 end";
}
} // STORAGE_DAEMON
} // OHOS
