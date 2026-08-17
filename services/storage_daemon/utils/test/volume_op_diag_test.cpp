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
#include "utils/volume_op_diag.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
namespace {
constexpr int32_t DFX_STAGE_GET_PARTITION_TABLE = 46;
constexpr int32_t VOL_OP_GET_PARTITION_TABLE = 5;

VolumeOpDiagContext MakeTestContext()
{
    VolumeOpDiagContext ctx;
    ctx.funcName = "StorageDaemonProvider::ReadPartitionTable";
    ctx.bizStage = DFX_STAGE_GET_PARTITION_TABLE;
    ctx.opType = VOL_OP_GET_PARTITION_TABLE;
    ctx.devPath = "/dev/block/disk-8-0";
    ctx.fsType = "exfat";
    return ctx;
}
} // namespace

class VolumeOpDiagTest : public testing::Test {
public:
    void SetUp() override
    {
        VolumeOpDiagEnd();
    }

    void TearDown() override
    {
        VolumeOpDiagEnd();
    }
};

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_ContextLifecycle_001, TestSize.Level0)
{
    EXPECT_FALSE(VolumeOpDiagWasReported());
    VolumeOpDiagBegin(MakeTestContext());
    EXPECT_FALSE(VolumeOpDiagWasReported());
    VolumeOpDiagEnd();
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_AttachContext_001, TestSize.Level0)
{
    VolumeOpDiagBegin(MakeTestContext());
    VolumeOpDiagContext captured = VolumeOpDiagCaptureContext();
    VolumeOpDiagEnd();
    VolumeOpDiagAttachContext(captured);
    std::vector<std::string> cmd = {"/system/bin/sgdisk", "--ohos-dump", "/dev/block/disk-8-0"};
    std::vector<std::string> output = {"error line"};
    VolumeOpDiagReportToolFailure(cmd, -1, 2, &output);
    EXPECT_FALSE(VolumeOpDiagWasReported());
    VolumeOpDiagFlushFailureReport(-1);
    EXPECT_TRUE(VolumeOpDiagWasReported());
    VolumeOpDiagEnd();
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_FlushWithoutContext_001, TestSize.Level0)
{
    VolumeOpDiagFlushFailureReport(-1);
    EXPECT_FALSE(VolumeOpDiagWasReported());
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_UpdateDevPath_001, TestSize.Level0)
{
    VolumeOpDiagBegin(MakeTestContext());
    VolumeOpDiagUpdateDevPath("/dev/block/verified-disk-8-0");
    VolumeOpDiagContext ctx = VolumeOpDiagCaptureContext();
    EXPECT_EQ(ctx.devPath, "/dev/block/verified-disk-8-0");
    VolumeOpDiagEnd();
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_ToolFailureThenFlush_001, TestSize.Level0)
{
    VolumeOpDiagBegin(MakeTestContext());
    VolumeOpDiagReportToolFailure({"/system/bin/mount.exfat"}, -1, 1, nullptr);
    EXPECT_FALSE(VolumeOpDiagWasReported());
    VolumeOpDiagFlushFailureReport(-1);
    EXPECT_TRUE(VolumeOpDiagWasReported());
    VolumeOpDiagEnd();
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_ScheduleAsyncFsckReport_001, TestSize.Level0)
{
    VolumeOpDiagContext ctx = MakeTestContext();
    ctx.funcName = "StorageDaemonProvider::Mount";
    ctx.active = true;
    VolumeOpDiagScheduleAsyncFsckReport(-1, ctx);
    SUCCEED();
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_MergeToolEntries_001, TestSize.Level0)
{
    VolumeOpDiagBegin(MakeTestContext());
    VolumeOpDiagToolEntry entry;
    entry.cmd = "mkfs.exfat /dev/block/disk-8-0";
    entry.ret = -1;
    entry.exitCode = 1;
    VolumeOpDiagMergeToolEntries({entry});
    VolumeOpDiagFlushFailureReport(-1);
    EXPECT_TRUE(VolumeOpDiagWasReported());
    VolumeOpDiagEnd();
}
} // namespace StorageDaemon
} // namespace OHOS
