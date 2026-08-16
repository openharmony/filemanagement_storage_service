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
#include <gtest/hwext/gtest-ext.h>
#include "utils/fsck_diagnose.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class FsckDiagnoseTest : public testing::Test {};

HWTEST_F(FsckDiagnoseTest, GetFsckDiagnoseCmd_Exfat_001, TestSize.Level0)
{
    const std::string cmd = GetFsckDiagnoseCmd("/dev/block/vol-1", "exfat");
    EXPECT_NE(cmd.find("fsck.exfat"), std::string::npos);
    EXPECT_NE(cmd.find("/dev/block/vol-1"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_UnsupportedFsType_001, TestSize.Level0)
{
    FsckResult result = FsckDiagnose("/dev/block/vol-1", "unknown");
    EXPECT_EQ(result.exitCode, -1);
    EXPECT_TRUE(result.cmd.empty());
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnoseWithTimeout_UnsupportedFsType_001, TestSize.Level0)
{
    FsckResult result = FsckDiagnoseWithTimeout("/dev/block/vol-1", "unknown", FSCK_DIAGNOSE_TIMEOUT_S);
    EXPECT_EQ(result.exitCode, -1);
    EXPECT_TRUE(result.cmd.empty());
}

HWTEST_F(FsckDiagnoseTest, GetFsckDiagnoseCmd_Vfat_001, TestSize.Level0)
{
    const std::string cmd = GetFsckDiagnoseCmd("/dev/block/vol-1", "vfat");
    EXPECT_NE(cmd.find("fsck_msdos"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_BuildCmdExfat_001, TestSize.Level0)
{
    FsckResult result = FsckDiagnose("/dev/block/vol-1", "exfat");
    EXPECT_FALSE(result.cmd.empty());
    EXPECT_NE(result.cmd.find("fsck.exfat"), std::string::npos);
}
} // namespace StorageDaemon
} // namespace OHOS
