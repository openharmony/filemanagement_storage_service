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
#include <csignal>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <gtest/hwext/gtest-ext.h>
#include "utils/fsck_diagnose.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

namespace {
constexpr const char *TOOL_DIR = "/data/storage_daemon_fsck_diag_ut";
constexpr int32_t SIGKILL_EXIT = 128 + SIGKILL;
constexpr int32_t SHORT_TIMEOUT_S = 1;
constexpr int32_t WAIT_EXIT_TIMEOUT_S = 5;
constexpr size_t MAX_OUTPUT_LEN = 1024;

void WriteFakeTool(const std::string &name, const std::string &body)
{
    std::string path = std::string(TOOL_DIR) + "/" + name;
    std::ofstream ofs(path);
    ofs << "#!/system/bin/sh\n" << body << "\n";
    ofs.close();
    (void)chmod(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

void RemoveFakeTool(const std::string &name)
{
    (void)unlink((std::string(TOOL_DIR) + "/" + name).c_str());
}
} // namespace

class FsckDiagnoseTest : public testing::Test {
public:
    void SetUp() override
    {
        (void)mkdir(TOOL_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        const char *path = getenv("PATH");
        if (path == nullptr) {
            oldPath_.clear();
        } else {
            oldPath_ = path;
        }
        (void)setenv("PATH", (std::string(TOOL_DIR) + ":" + oldPath_).c_str(), 1);
        WriteFakeTool("fsck.exfat", "printf 'clean\\n'; exit 0");
        WriteFakeTool("fsck.ntfs", "printf 'ntfs-out\\n'; printf 'ntfs-err\\n' >&2; exit 1");
        WriteFakeTool("fsck_msdos", "printf 'vfat-out\\n'; exit 0");
        WriteFakeTool("e2fsck", "printf 'ext4-out\\n'; exit 0");
        WriteFakeTool("fsck.f2fs", "printf 'f2fs-out\\n'; exit 0");
    }

    void TearDown() override
    {
        (void)setenv("PATH", oldPath_.c_str(), 1);
        RemoveFakeTool("fsck.exfat");
        RemoveFakeTool("fsck.ntfs");
        RemoveFakeTool("fsck_msdos");
        RemoveFakeTool("e2fsck");
        RemoveFakeTool("fsck.f2fs");
        (void)rmdir(TOOL_DIR);
    }

private:
    std::string oldPath_;
};

HWTEST_F(FsckDiagnoseTest, GetFsckDiagnoseCmd_Exfat_001, TestSize.Level0)
{
    const std::string cmd = GetFsckDiagnoseCmd("/dev/block/vol-1", "exfat");
    EXPECT_NE(cmd.find("fsck.exfat"), std::string::npos);
    EXPECT_NE(cmd.find("/dev/block/vol-1"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, GetFsckDiagnoseCmd_NtfsExt4Fat32_001, TestSize.Level0)
{
    EXPECT_NE(GetFsckDiagnoseCmd("/dev/block/vol-1", "ntfs").find("fsck.ntfs"), std::string::npos);
    EXPECT_NE(GetFsckDiagnoseCmd("/dev/block/vol-1", "ext4").find("e2fsck"), std::string::npos);
    EXPECT_NE(GetFsckDiagnoseCmd("/dev/block/vol-1", "fat32").find("fsck_msdos"), std::string::npos);
    EXPECT_TRUE(GetFsckDiagnoseCmd("/dev/block/vol-1", "").empty());
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

HWTEST_F(FsckDiagnoseTest, GetFsckDiagnoseCmd_F2fs_001, TestSize.Level0)
{
    const std::string cmd = GetFsckDiagnoseCmd("/dev/block/vol-1", "f2fs");
    EXPECT_NE(cmd.find("fsck.f2fs"), std::string::npos);
    EXPECT_NE(cmd.find("--dry-run"), std::string::npos);
    EXPECT_NE(cmd.find("-p1"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, GetFsckDiagnoseCmd_Hmfs_001, TestSize.Level0)
{
    const std::string cmd = GetFsckDiagnoseCmd("/dev/block/vol-1", "hmfs");
    EXPECT_NE(cmd.find("fsck.f2fs"), std::string::npos);
    EXPECT_NE(cmd.find("--dry-run"), std::string::npos);
    EXPECT_NE(cmd.find("-p1"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_BuildCmdExfat_001, TestSize.Level0)
{
    FsckResult result = FsckDiagnose("/dev/block/vol-1", "exfat");
    EXPECT_FALSE(result.cmd.empty());
    EXPECT_NE(result.cmd.find("fsck.exfat"), std::string::npos);
    EXPECT_EQ(result.ret, 0);
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_NE(result.output.find("clean"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_NtfsExitCodeAndStderr_001, TestSize.Level0)
{
    FsckResult result = FsckDiagnose("/dev/block/vol-1", "ntfs");
    EXPECT_EQ(result.ret, 0);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.output.find("ntfs-out"), std::string::npos);
    EXPECT_NE(result.output.find("ntfs-err"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_JoinOutputNoExtraBlank_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "printf 'line1\\n'; printf 'line2\\n' >&2; exit 0");
    FsckResult result = FsckDiagnose("/dev/block/vol-1", "exfat");
    EXPECT_EQ(result.output.find("line1\n\nline2"), std::string::npos);
    EXPECT_NE(result.output.find("line1"), std::string::npos);
    EXPECT_NE(result.output.find("line2"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_VfatExt4F2fs_001, TestSize.Level0)
{
    EXPECT_EQ(FsckDiagnose("/dev/block/vol-1", "vfat").exitCode, 0);
    EXPECT_EQ(FsckDiagnose("/dev/block/vol-1", "ext4").exitCode, 0);
    EXPECT_EQ(FsckDiagnose("/dev/block/vol-1", "f2fs").exitCode, 0);
    EXPECT_EQ(FsckDiagnose("/dev/block/vol-1", "hmfs").exitCode, 0);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_TruncateLongOutput_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "printf '" + std::string(2000, 'a') + "'; exit 0");
    FsckResult result = FsckDiagnose("/dev/block/vol-1", "exfat");
    EXPECT_EQ(result.output.size(), MAX_OUTPUT_LEN);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnose_MissingBinary_001, TestSize.Level0)
{
    RemoveFakeTool("fsck.exfat");
    (void)setenv("PATH", TOOL_DIR, 1);
    FsckResult result = FsckDiagnose("/dev/block/vol-1", "exfat");
    EXPECT_FALSE(result.cmd.empty());
    EXPECT_EQ(result.exitCode, 1);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnoseWithTimeout_Kill_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "printf 'partial\\n'; /system/bin/sleep 10; exit 0");
    FsckResult result = FsckDiagnoseWithTimeout("/dev/block/vol-1", "exfat", SHORT_TIMEOUT_S);
    EXPECT_NE(result.output.find("fsck diagnose timeout after 1s"), std::string::npos);
    EXPECT_EQ(result.exitCode, SIGKILL_EXIT);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnoseWithTimeout_NoNewlineThenKill_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "printf 'partial'; /system/bin/sleep 10; exit 0");
    FsckResult result = FsckDiagnoseWithTimeout("/dev/block/vol-1", "exfat", SHORT_TIMEOUT_S);
    EXPECT_NE(result.output.find("partial"), std::string::npos);
    EXPECT_NE(result.output.find("fsck diagnose timeout after 1s"), std::string::npos);
    EXPECT_EQ(result.exitCode, SIGKILL_EXIT);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnoseWithTimeout_ZeroSec_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "/system/bin/sleep 10; exit 0");
    FsckResult result = FsckDiagnoseWithTimeout("/dev/block/vol-1", "exfat", 0);
    EXPECT_FALSE(result.cmd.empty());
    EXPECT_NE(result.output.find("timeout"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnoseWithTimeout_NegativeSec_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "/system/bin/sleep 10; exit 0");
    FsckResult result = FsckDiagnoseWithTimeout("/dev/block/vol-1", "exfat", -1);
    EXPECT_FALSE(result.cmd.empty());
    EXPECT_NE(result.output.find("timeout"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnoseWithTimeout_ReadThenExit_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "printf 'first\\n'; /system/bin/sleep 1; printf 'second\\n'; exit 0");
    FsckResult result = FsckDiagnoseWithTimeout("/dev/block/vol-1", "exfat", WAIT_EXIT_TIMEOUT_S);
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_NE(result.output.find("first"), std::string::npos);
    EXPECT_NE(result.output.find("second"), std::string::npos);
    EXPECT_EQ(result.output.find("timeout"), std::string::npos);
}

HWTEST_F(FsckDiagnoseTest, FsckDiagnoseWithTimeout_TruncateThenMark_001, TestSize.Level0)
{
    WriteFakeTool("fsck.exfat", "printf '" + std::string(2000, 'a') + "'; /system/bin/sleep 10; exit 0");
    FsckResult result = FsckDiagnoseWithTimeout("/dev/block/vol-1", "exfat", SHORT_TIMEOUT_S);
    EXPECT_LE(result.output.size(), MAX_OUTPUT_LEN);
    EXPECT_EQ(result.exitCode, SIGKILL_EXIT);
}
} // namespace StorageDaemon
} // namespace OHOS
