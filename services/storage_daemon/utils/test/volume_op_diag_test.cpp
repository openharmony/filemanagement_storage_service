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

#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <gtest/hwext/gtest-ext.h>
#include "utils/volume_op_diag.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
namespace {
constexpr int32_t DFX_STAGE_GET_PARTITION_TABLE = 46;
constexpr int32_t VOL_OP_GET_PARTITION_TABLE = 5;
constexpr size_t MAX_TOOL_OUTPUT_LINES = 16;
constexpr size_t MAX_TOOL_LINE_LEN = 256;

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

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_AppendFsckDiagnose_001, TestSize.Level0)
{
    const std::string toolDir = "/data/storage_daemon_fsck_diag_ut_vol";
    (void)mkdir(toolDir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    std::string toolPath = toolDir + "/fsck.exfat";
    std::ofstream ofs(toolPath);
    ofs << "#!/system/bin/sh\nprintf 'clean\\n'; exit 0\n";
    ofs.close();
    (void)chmod(toolPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    const char *oldPath = getenv("PATH");
    std::string saved = (oldPath == nullptr) ? "" : oldPath;
    (void)setenv("PATH", (toolDir + ":" + saved).c_str(), 1);

    VolumeOpDiagContext ctx = MakeTestContext();
    ctx.funcName = "StorageDaemonProvider::Mount";
    VolumeOpDiagBegin(ctx);
    VolumeOpDiagAppendFsckDiagnose(VolumeOpDiagCaptureContext());
    VolumeOpDiagContext after = VolumeOpDiagCaptureContext();
    EXPECT_FALSE(after.toolEntries.empty());
    EXPECT_NE(after.toolEntries[0].cmd.find("fsck.exfat"), std::string::npos);
    EXPECT_EQ(after.toolEntries[0].exitCode, 0);
    VolumeOpDiagFlushFailureReport(-1);
    EXPECT_TRUE(VolumeOpDiagWasReported());
    VolumeOpDiagEnd();

    (void)setenv("PATH", saved.c_str(), 1);
    (void)unlink(toolPath.c_str());
    (void)rmdir(toolDir.c_str());
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_AppendFsckDiagnose_Skip_001, TestSize.Level0)
{
    VolumeOpDiagContext ctx = MakeTestContext();
    VolumeOpDiagAppendFsckDiagnose(ctx);
    EXPECT_TRUE(VolumeOpDiagCaptureContext().toolEntries.empty());

    VolumeOpDiagBegin(ctx);
    ctx.devPath.clear();
    VolumeOpDiagAppendFsckDiagnose(ctx);
    EXPECT_TRUE(VolumeOpDiagCaptureContext().toolEntries.empty());

    ctx = MakeTestContext();
    ctx.fsType.clear();
    VolumeOpDiagAppendFsckDiagnose(ctx);
    EXPECT_TRUE(VolumeOpDiagCaptureContext().toolEntries.empty());

    ctx = MakeTestContext();
    ctx.fsType = "unknown";
    VolumeOpDiagAppendFsckDiagnose(ctx);
    EXPECT_TRUE(VolumeOpDiagCaptureContext().toolEntries.empty());
    VolumeOpDiagEnd();
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_TakeAndInactive_001, TestSize.Level0)
{
    VolumeOpDiagBegin(MakeTestContext());
    VolumeOpDiagReportToolFailure({"sgdisk"}, -1, 2, nullptr);
    auto entries = VolumeOpDiagTakeToolEntries();
    EXPECT_EQ(entries.size(), 1);
    EXPECT_TRUE(VolumeOpDiagCaptureContext().toolEntries.empty());
    VolumeOpDiagEnd();

    VolumeOpDiagToolEntry entry;
    entry.cmd = "fsck";
    VolumeOpDiagAppendToolEntry(entry);
    VolumeOpDiagReportToolFailure({"sgdisk"}, -1, 1, nullptr);
    VolumeOpDiagMergeToolEntries({entry});
    VolumeOpDiagUpdateDevPath("/dev/block/a");
    EXPECT_TRUE(VolumeOpDiagCaptureContext().devPath.empty());
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_JoinOutputLimit_001, TestSize.Level0)
{
    VolumeOpDiagBegin(MakeTestContext());
    std::vector<std::string> output;
    for (size_t i = 0; i < MAX_TOOL_OUTPUT_LINES + 2; ++i) {
        output.push_back("line");
    }
    output[0] = std::string(MAX_TOOL_LINE_LEN + 8, 'x');
    VolumeOpDiagReportToolFailure({"mkfs.exfat", "/dev/block/a"}, -1, 1, &output);
    auto entries = VolumeOpDiagCaptureContext().toolEntries;
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].output.find(std::string(MAX_TOOL_LINE_LEN + 1, 'x')), std::string::npos);
    VolumeOpDiagFlushFailureReport(-1);
    VolumeOpDiagFlushFailureReport(-2);
    EXPECT_TRUE(VolumeOpDiagWasReported());
    VolumeOpDiagEnd();
}

HWTEST_F(VolumeOpDiagTest, VolumeOpDiag_MergeSkipAndUpdateEmpty_001, TestSize.Level0)
{
    VolumeOpDiagBegin(MakeTestContext());
    VolumeOpDiagMergeToolEntries({});
    EXPECT_TRUE(VolumeOpDiagCaptureContext().toolEntries.empty());
    VolumeOpDiagUpdateDevPath("");
    EXPECT_EQ(VolumeOpDiagCaptureContext().devPath, "/dev/block/disk-8-0");
    VolumeOpDiagContext emptyCtx;
    VolumeOpDiagBegin(emptyCtx);
    VolumeOpDiagFlushFailureReport(-1);
    EXPECT_TRUE(VolumeOpDiagWasReported());
    VolumeOpDiagEnd();
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
